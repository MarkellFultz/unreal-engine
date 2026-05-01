#include "PanFryingActor.h"
#include "Components/StaticMeshComponent.h"

// =============================================
// 建構子
// =============================================
APanFryingActor::APanFryingActor()
{
    PrimaryActorTick.bCanEverTick = true;

    PanMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PanMesh"));
    RootComponent = PanMesh;

    QTEComp = CreateDefaultSubobject<UQTEComponent>(TEXT("QTEComp"));
}

void APanFryingActor::BeginPlay()
{
    Super::BeginPlay();

    // 綁定 QTE 完成事件 → 觸發翻面判定
    if (QTEComp)
    {
        QTEComp->OnQTECompleted.AddDynamic(this, &APanFryingActor::OnFlipQTECompleted);
    }
}

// =============================================
// Tick：烹飪主迴圈
// =============================================
void APanFryingActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 沒開火或沒有食材則不計時
    if (!bHeatOn || !bHasIngredient) return;

    TickCooking(DeltaTime);
}

void APanFryingActor::TickCooking(float DeltaTime)
{
    // 每面的目標時間 = 總烹飪時間的一半
    const float HalfTime = CurrentIngredientData.TotalCookTime * 0.5f;
    if (HalfTime <= 0.0f) return;

    switch (CurrentState)
    {
    // ────────────────────────────────────────
    // 烹飪第一面
    // ────────────────────────────────────────
    case EPanFryingState::Cooking_Side1:
    {
        ElapsedSide1 += DeltaTime;

        // 通知 Blueprint 更新顏色（0.0 = 生，1.0 = 該面熟透）
        float Progress = FMath::Clamp(ElapsedSide1 / HalfTime, 0.0f, 1.0f);
        OnCookingProgressUpdated(Progress, true);

        // 達到翻面最佳時機（ColorChangeRatio 代表「幾成熟時翻面最好」）
        if (!bFlipPromptFired &&
            ElapsedSide1 >= HalfTime * CurrentIngredientData.ColorChangeRatio)
        {
            bFlipPromptFired = true;
            SetState(EPanFryingState::WaitingFlip);
            OnFlipPromptReady(); // 通知 Blueprint 顯示翻面提示
        }

        CheckBurn();
        break;
    }

    // ────────────────────────────────────────
    // 等待翻面：繼續累積時間，超時仍會燒焦
    // ────────────────────────────────────────
    case EPanFryingState::WaitingFlip:
    {
        ElapsedSide1 += DeltaTime;
        CheckBurn();
        break;
    }

    // ────────────────────────────────────────
    // 烹飪第二面
    // ────────────────────────────────────────
    case EPanFryingState::Cooking_Side2:
    {
        ElapsedSide2 += DeltaTime;

        float Progress = FMath::Clamp(ElapsedSide2 / HalfTime, 0.0f, 1.0f);
        OnCookingProgressUpdated(Progress, false);

        // 第二面時間到 → 完成
        if (ElapsedSide2 >= HalfTime)
        {
            FinalCookScore = CalculateFinalScore();
            SetState(EPanFryingState::Done);
            OnCookingCompleted(FinalCookScore);
        }

        CheckBurn();
        break;
    }

    default:
        break;
    }
}

// =============================================
// IInteractInterface：根據狀態給玩家不同行為
// =============================================
void APanFryingActor::Interact(AActor* Interactor)
{
    CurrentInteractor = Interactor;

    switch (CurrentState)
    {
    // 尚未開火 → 開火
    case EPanFryingState::Idle:
        ToggleHeat();
        break;

    // 已開火，無食材 → 請 Blueprint 開啟選食材 UI
    case EPanFryingState::Heating:
        OnRequestIngredientSelection(Interactor);
        break;

    // 等待翻面 → 觸發翻面 QTE（黃金區間 45°~105°，共 60 度）
    case EPanFryingState::WaitingFlip:
        if (QTEComp && !QTEComp->IsQTEActive())
        {
            QTEComp->StartQTE(45.0f, 105.0f, 150.0f);
        }
        break;

    // 完成或燒焦 → 出鍋（或清除）
    case EPanFryingState::Done:
    case EPanFryingState::Burned:
        RemoveIngredient();
        break;

    default:
        break;
    }
}

FText APanFryingActor::GetInteractPrompt() const
{
    switch (CurrentState)
    {
    case EPanFryingState::Idle:           return FText::FromString(TEXT("[E] 開啟火源"));
    case EPanFryingState::Heating:        return FText::FromString(TEXT("[E] 放入食材"));
    case EPanFryingState::Cooking_Side1:  return FText::GetEmpty(); // 烹飪中，不干擾
    case EPanFryingState::WaitingFlip:    return FText::FromString(TEXT("[E] 翻面"));
    case EPanFryingState::Cooking_Side2:  return FText::GetEmpty();
    case EPanFryingState::Done:           return FText::FromString(TEXT("[E] 出鍋"));
    case EPanFryingState::Burned:         return FText::FromString(TEXT("[E] 清除燒焦食材"));
    default:                              return FText::GetEmpty();
    }
}

// =============================================
// 玩家操作：開/關火 (US 7.2 / 7.7)
// =============================================
void APanFryingActor::ToggleHeat()
{
    bHeatOn = !bHeatOn;
    OnHeatToggled(bHeatOn);

    if (bHeatOn && CurrentState == EPanFryingState::Idle)
        SetState(EPanFryingState::Heating);
    else if (!bHeatOn && CurrentState == EPanFryingState::Heating)
        SetState(EPanFryingState::Idle);
}

// =============================================
// 玩家操作：放入食材 (US 7.4)
// =============================================
void APanFryingActor::PlaceIngredient(FName IngredientID, AActor* Interactor)
{
    if (CurrentState != EPanFryingState::Heating)
    {
        UE_LOG(LogTemp, Warning, TEXT("PanFryingActor::PlaceIngredient — 需先開火才能放入食材"));
        return;
    }

    if (!IngredientDataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("PanFryingActor::PlaceIngredient — IngredientDataTable 未設定！"));
        return;
    }

    FIngredientData* Data = IngredientDataTable->FindRow<FIngredientData>(
        IngredientID, TEXT("PanFrying_PlaceIngredient"));

    if (!Data)
    {
        UE_LOG(LogTemp, Warning, TEXT("PanFryingActor::PlaceIngredient — 找不到食材 ID: %s"),
            *IngredientID.ToString());
        return;
    }

    // 初始化食材資料
    CurrentIngredientID   = IngredientID;
    CurrentIngredientData = *Data;
    bHasIngredient        = true;
    CurrentInteractor     = Interactor;

    // 重置所有計時與評分
    ElapsedSide1    = 0.0f;
    ElapsedSide2    = 0.0f;
    FlipScore       = 0.0f;
    FinalCookScore  = 0.0f;
    bFlipPromptFired = false;

    SetState(EPanFryingState::Cooking_Side1);
    OnIngredientPlaced(IngredientID);
}

// =============================================
// 玩家操作：出鍋 (US 7.8)
// =============================================
void APanFryingActor::RemoveIngredient()
{
    if (CurrentState != EPanFryingState::Done &&
        CurrentState != EPanFryingState::Burned)
    {
        return;
    }

    bHasIngredient    = false;
    CurrentIngredientID = NAME_None;

    // 出鍋後回到「加熱待機」（火還開著）或「待機」
    SetState(bHeatOn ? EPanFryingState::Heating : EPanFryingState::Idle);
}

// =============================================
// QTE 回調：翻面判定 (US 7.5-2 / 7.6-2 / 7.6-3)
// =============================================
void APanFryingActor::OnFlipQTECompleted(bool bSuccess)
{
    if (CurrentState != EPanFryingState::WaitingFlip) return;

    // 計算從翻面提示出現到玩家實際翻面的延遲（越快越好）
    const float IdealFlipTime = CurrentIngredientData.TotalCookTime * 0.5f
                              * CurrentIngredientData.ColorChangeRatio;
    const float OverTime = FMath::Max(0.0f, ElapsedSide1 - IdealFlipTime);

    EFlipQuality Quality;

    if (!bSuccess)
    {
        // QTE 失敗：翻面失敗，不給加成
        FlipScore = 0.0f;
        Quality   = EFlipQuality::Missed;
    }
    else if (OverTime <= 1.5f)
    {
        // 提示出現後 1.5 秒內翻面：完美
        FlipScore = 1.0f;
        Quality   = EFlipQuality::Perfect;
    }
    else
    {
        // 稍晚翻面：良好
        FlipScore = 0.5f;
        Quality   = EFlipQuality::Good;
    }

    SetState(EPanFryingState::Cooking_Side2);
    OnIngredientFlipped(Quality); // 通知 Blueprint 播放翻面動畫
}

// =============================================
// 內部輔助
// =============================================
void APanFryingActor::SetState(EPanFryingState NewState)
{
    CurrentState = NewState;
}

float APanFryingActor::GetCookingProgress() const
{
    if (!bHasIngredient) return 0.0f;

    const float HalfTime = CurrentIngredientData.TotalCookTime * 0.5f;
    if (HalfTime <= 0.0f) return 0.0f;

    switch (CurrentState)
    {
    case EPanFryingState::Cooking_Side1:
    case EPanFryingState::WaitingFlip:
        return FMath::Clamp(ElapsedSide1 / HalfTime, 0.0f, 1.0f);

    case EPanFryingState::Cooking_Side2:
        return FMath::Clamp(ElapsedSide2 / HalfTime, 0.0f, 1.0f);

    case EPanFryingState::Done:
    case EPanFryingState::Burned:
        return 1.0f;

    default:
        return 0.0f;
    }
}

float APanFryingActor::CalculateFinalScore() const
{
    // ── 時間分：總烹飪時間與目標時間的誤差 ──
    const float TotalTime  = ElapsedSide1 + ElapsedSide2;
    const float TargetTime = CurrentIngredientData.TotalCookTime;
    const float TimeDiff   = FMath::Abs(TotalTime - TargetTime);

    float TimeScore;
    if      (TimeDiff <= 1.0f) TimeScore = 1.0f; // 誤差 1 秒內：滿分
    else if (TimeDiff <= 3.0f) TimeScore = 0.7f; // 1~3 秒：良好
    else                       TimeScore = 0.4f; // 超過 3 秒：普通

    // 翻面占 50%，時間占 50%
    return (FlipScore * 0.5f) + (TimeScore * 0.5f);
}

void APanFryingActor::CheckBurn()
{
    if (!bHasIngredient) return;
    if (CurrentState == EPanFryingState::Burned) return;

    // BurnTimeRatio 是相對於 TotalCookTime 的比例（例如 1.2 = 超時 20%）
    const float BurnThreshold = CurrentIngredientData.TotalCookTime
                              * CurrentIngredientData.BurnTimeRatio;
    const float TotalElapsed  = ElapsedSide1 + ElapsedSide2;

    if (TotalElapsed >= BurnThreshold)
    {
        SetState(EPanFryingState::Burned);
        OnIngredientBurned(); // 通知 Blueprint 播放燒焦特效
    }
}
