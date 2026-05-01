#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../InteractInterface.h"
#include "../GameDataStructs.h"
#include "../QTEComponent.h"
#include "PanFryingActor.generated.h"

// =============================================
// 煎的烹飪狀態機 (US 7.2 ~ 7.8)
// =============================================
UENUM(BlueprintType)
enum class EPanFryingState : uint8
{
    Idle             UMETA(DisplayName = "待機（火未開）"),
    Heating          UMETA(DisplayName = "加熱中（等待放入食材）"),
    Cooking_Side1    UMETA(DisplayName = "烹飪第一面"),
    WaitingFlip      UMETA(DisplayName = "等待翻面 QTE"),
    Cooking_Side2    UMETA(DisplayName = "烹飪第二面"),
    Done             UMETA(DisplayName = "烹飪完成，等待出鍋"),
    Burned           UMETA(DisplayName = "燒焦")
};

// =============================================
// 翻面品質（影響最終評分）
// =============================================
UENUM(BlueprintType)
enum class EFlipQuality : uint8
{
    Perfect  UMETA(DisplayName = "完美翻面"),
    Good     UMETA(DisplayName = "良好翻面"),
    Missed   UMETA(DisplayName = "翻面失敗")
};

UCLASS()
class MYPROJECT_API APanFryingActor : public AActor, public IInteractInterface
{
    GENERATED_BODY()

public:
    APanFryingActor();

    virtual void Tick(float DeltaTime) override;

    // ── IInteractInterface ──
    virtual void Interact(AActor* Interactor) override;
    virtual FText GetInteractPrompt() const override;

    // =============================================
    // 玩家操作入口（供 Character 或 Blueprint 呼叫）
    // =============================================

    /** 開/關火源 (US 7.2 / 7.7) */
    UFUNCTION(BlueprintCallable, Category = "PanFrying")
    void ToggleHeat();

    /**
     * 放入食材，開始烹飪 (US 7.4)
     * Blueprint 選食材 UI 確認後呼叫此函式
     * @param IngredientID  對應 IngredientData 資料表的 Row Name
     * @param Interactor    玩家 Actor
     */
    UFUNCTION(BlueprintCallable, Category = "PanFrying")
    void PlaceIngredient(FName IngredientID, AActor* Interactor);

    /** 出鍋，結束烹飪流程 (US 7.8) */
    UFUNCTION(BlueprintCallable, Category = "PanFrying")
    void RemoveIngredient();

    // =============================================
    // 狀態查詢（給 HUD / Blueprint 讀取）
    // =============================================

    UFUNCTION(BlueprintPure, Category = "PanFrying")
    EPanFryingState GetCookingState() const { return CurrentState; }

    /**
     * 當前烹飪面的進度 0.0 ~ 1.0，給 HUD 進度條使用 (US 7.6-1)
     * 第一面和第二面各自獨立從 0 開始
     */
    UFUNCTION(BlueprintPure, Category = "PanFrying")
    float GetCookingProgress() const;

    /** 0.0 ~ 1.0，烹飪完成後可傳給結算系統 (US 10.1) */
    UFUNCTION(BlueprintPure, Category = "PanFrying")
    float GetFinalScore() const { return FinalCookScore; }

    UFUNCTION(BlueprintPure, Category = "PanFrying")
    bool IsHeatOn() const { return bHeatOn; }

    UFUNCTION(BlueprintPure, Category = "PanFrying")
    FName GetCurrentIngredientID() const { return CurrentIngredientID; }

    // =============================================
    // Blueprint 視覺/音效事件
    // （C++ 觸發時機，Blueprint 實作特效）
    // =============================================

    /** 火源開關時呼叫 — BP 播放火焰特效與音效 (US 7.2) */
    UFUNCTION(BlueprintImplementableEvent, Category = "PanFrying|Events")
    void OnHeatToggled(bool bIsOn);

    /** 食材放入鍋中 — BP 播放放入動畫與油爆聲 (US 7.4) */
    UFUNCTION(BlueprintImplementableEvent, Category = "PanFrying|Events")
    void OnIngredientPlaced(FName IngredientId);

    /**
     * 每 Tick 呼叫，用於 BP 驅動食材顏色漸變 (US 7.6-1)
     * @param Progress    0.0(生) ~ 1.0(該面熟透)
     * @param bIsFirstSide  true = 第一面, false = 第二面
     */
    UFUNCTION(BlueprintImplementableEvent, Category = "PanFrying|Events")
    void OnCookingProgressUpdated(float Progress, bool bIsFirstSide);

    /** 達到翻面最佳時機 — BP 顯示提示動畫 (US 7.5-2 前置) */
    UFUNCTION(BlueprintImplementableEvent, Category = "PanFrying|Events")
    void OnFlipPromptReady();

    /**
     * QTE 判定完成後的翻面結果 — BP 播放翻面動畫 (US 7.5-1 / 7.6-2 / 7.6-3)
     * @param FlipQuality  Perfect / Good / Missed
     */
    UFUNCTION(BlueprintImplementableEvent, Category = "PanFrying|Events")
    void OnIngredientFlipped(EFlipQuality FlipQuality);

    /**
     * 烹飪完成，可以出鍋 — BP 播放完成提示 (US 7.8 前置 / US 10.1 輸入)
     * @param Score  0.0 ~ 1.0 的最終評分
     */
    UFUNCTION(BlueprintImplementableEvent, Category = "PanFrying|Events")
    void OnCookingCompleted(float Score);

    /** 食材燒焦 — BP 播放焦黑特效與警報音 (US 7.6-3) */
    UFUNCTION(BlueprintImplementableEvent, Category = "PanFrying|Events")
    void OnIngredientBurned();

    /**
     * 請求 Blueprint 開啟選食材 UI (US 7.3 / 7.4)
     * Blueprint 在玩家確認食材後呼叫 PlaceIngredient()
     */
    UFUNCTION(BlueprintImplementableEvent, Category = "PanFrying|Events")
    void OnRequestIngredientSelection(AActor* Interactor);

protected:
    virtual void BeginPlay() override;

    /** QTE 完成後的翻面邏輯回調（綁定 QTEComponent::OnQTECompleted） */
    UFUNCTION()
    void OnFlipQTECompleted(bool bSuccess);

private:
    // ── 狀態 ──
    EPanFryingState CurrentState = EPanFryingState::Idle;
    bool bHeatOn          = false;
    bool bFlipPromptFired = false;

    // ── 目前食材 ──
    FName           CurrentIngredientID;
    FIngredientData CurrentIngredientData;
    bool            bHasIngredient = false;

    // ── 計時（秒）──
    float ElapsedSide1 = 0.0f;
    float ElapsedSide2 = 0.0f;

    // ── 評分 ──
    float FlipScore     = 0.0f; // 1.0=完美, 0.5=良好, 0.0=失敗
    float FinalCookScore = 0.0f;

    // ── 資料表（在 Editor Details 面板指定）──
    UPROPERTY(EditAnywhere, Category = "Data",
        meta = (ToolTip = "指向包含 FIngredientData 的 DataTable"))
    UDataTable* IngredientDataTable;

    // ── 元件 ──
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
        meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* PanMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QTE",
        meta = (AllowPrivateAccess = "true"))
    UQTEComponent* QTEComp;

    // ── 互動者暫存 ──
    UPROPERTY()
    AActor* CurrentInteractor = nullptr;

    // ── 內部輔助 ──
    void SetState(EPanFryingState NewState);
    void TickCooking(float DeltaTime);
    void CheckBurn();
    float CalculateFinalScore() const;
};
