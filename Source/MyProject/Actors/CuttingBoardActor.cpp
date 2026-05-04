#include "CuttingBoardActor.h"
#include "Kismet/GameplayStatics.h"
#include "KismetProceduralMeshLibrary.h"
#include "Engine/Engine.h"
#include "../InventoryComponent.h"
#include "../GameDataStructs.h"
#include "../MyCharacter.h"

ACuttingBoardActor::ACuttingBoardActor()
{
    PrimaryActorTick.bCanEverTick = true;

    // 1. 建立砧板基礎模型
    BoardMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoardMesh"));
    RootComponent = BoardMesh;

    // 2. 建立載入模型的 Dummy (遊戲中隱藏)
    DummyStaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DummyStaticMeshComp"));
    DummyStaticMeshComp->SetupAttachment(RootComponent);
    DummyStaticMeshComp->SetVisibility(false);

    // ==========================================
    // 【終極修復】：徹底關閉假模型的碰撞！
    // 讓它變成真正的幽靈，物理引擎就不會因為它產生爆炸排斥力了。
    // ==========================================
    DummyStaticMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    DummyStaticMeshComp->SetGenerateOverlapEvents(false); // 順便關閉重疊偵測，節省效能
    // ==========================================

    // 3. 建立瞄準線平面
    AimingPlane = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AimingPlane"));
    AimingPlane->SetupAttachment(RootComponent);
    AimingPlane->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 瞄準線不需要碰撞
    AimingPlane->SetVisibility(false);

    TopDownCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
    TopDownCamera->SetupAttachment(RootComponent);
}
void ACuttingBoardActor::BeginPlay()
{
    Super::BeginPlay();
}

void ACuttingBoardActor::Interact(AActor* Interactor)
{
    CurrentInteractor = Interactor;

    // 螢幕除錯訊息：只要你有按到砧板，畫面上一定會印出這行黃字！
    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, TEXT("✅ 成功觸發砧板互動 (Interact)!"));

    switch (CurrentState)
    {
    case ECuttingState::Idle:
        OnOpenInventoryUI(Interactor);
        break;
    case ECuttingState::Aiming:
    case ECuttingState::ReadyToCollect:
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Cyan, TEXT("✅ 進入拾取流程..."));
        CollectIngredients();
        break;
    }
}

void ACuttingBoardActor::CollectIngredients()
{
    // 如果沒有互動者，印出紅字錯誤
    if (!CurrentInteractor)
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("❌ 錯誤：找不到互動的玩家！"));
        return;
    }

    UInventoryComponent* Inv = CurrentInteractor->FindComponentByClass<UInventoryComponent>();

    // 如果有找到背包組件
    if (Inv)
    {
        FString SlicedItemName = CurrentIngredientID.ToString() + TEXT("_Sliced");
        Inv->AddIngredient(FName(*SlicedItemName));

        // 清除砧板上的所有碎塊
        for (UProceduralMeshComponent* Mesh : ProcMeshes)
        {
            if (Mesh) Mesh->DestroyComponent();
        }
        ProcMeshes.Empty();

        // 重置砧板狀態
        AimingPlane->SetVisibility(false);
        CurrentIngredientID = NAME_None;
        CurrentState = ECuttingState::Idle;
        CurrentInteractor = nullptr;

        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("✅ 成功拾取！食材已放入背包，砧板重置。"));
    }
    else
    {
        // 如果找不到背包，印出紅字錯誤
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("❌ 錯誤：玩家身上沒有 InventoryComponent 背包組件！"));
    }
}

FText ACuttingBoardActor::GetInteractPrompt() const
{
    if (CurrentState == ECuttingState::Idle) return FText::FromString(TEXT("[E] 放置食材"));
    if (CurrentState == ECuttingState::Aiming) return FText::FromString(TEXT("[空白鍵] 切片 / [E] 拾取"));
    return FText::FromString(TEXT("[E] 拾取切塊"));
}

void ACuttingBoardActor::PlaceIngredient(FName IngredientID, AActor* Interactor)
{
    // ==========================================
    // 偵錯第一關：確認 UI 有沒有成功呼叫這個函數？傳來的 ID 是什麼？
    // ==========================================
    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Yellow, FString::Printf(TEXT("👉 收到放入請求！準備放入: %s"), *IngredientID.ToString()));

    // 偵錯第二關：檢查資料表有沒有設定？
    if (!IngredientDataTable)
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Red, TEXT("❌ 嚴重錯誤：資料表遺失！請打開 BP_CuttingBoard 重新設定 IngredientDataTable！"));
        return;
    }

    if (CurrentState != ECuttingState::Idle)
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Red, TEXT("❌ 錯誤：砧板目前不是 Idle 狀態，無法放入！"));
        return;
    }

    FIngredientData* Data = IngredientDataTable->FindRow<FIngredientData>(IngredientID, TEXT("CuttingBoard"));

    // 偵錯第三關：資料表裡找得到這個 ID 嗎？
    if (!Data)
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Red, FString::Printf(TEXT("❌ 錯誤：在 CSV 資料表中找不到這個 ID: %s"), *IngredientID.ToString()));
        return;
    }

    if (Data && !Data->IngredientMesh.IsNull())
    {
        AMyCharacter* MyChar = Cast<AMyCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));

        if (!Data->IsSliceable)
        {
            if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Orange, TEXT("⚠️ 提示：這個食材不需要切！"));
            if (MyChar) MyChar->SetUIInputMode(false);
            return;
        }

        if (MyChar)
        {
            UInventoryComponent* Inv = MyChar->FindComponentByClass<UInventoryComponent>();
            if (Inv && !Inv->RemoveIngredient(IngredientID))
            {
                // 如果是這裡失敗，我們之前已經有寫紅字警告了
                FString ErrorMsg = FString::Printf(TEXT("❌ 扣除失敗：背包找不到 [%s]！"), *IngredientID.ToString());
                if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Red, ErrorMsg);
                return;
            }
        }

        // ==========================================
        // 如果能走到這裡，代表前面的檢查全部過關！
        // ==========================================
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("✅ 檢查通過！開始生成模型與切換視角..."));

        CurrentIngredientID = IngredientID;
        CutCount = 0;

        UStaticMesh* LoadedMesh = Data->IngredientMesh.LoadSynchronous();
        DummyStaticMeshComp->SetStaticMesh(LoadedMesh);
        DummyStaticMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

        UProceduralMeshComponent* NewProcMesh = NewObject<UProceduralMeshComponent>(this);
        NewProcMesh->RegisterComponent();
        NewProcMesh->AttachToComponent(RootComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

        NewProcMesh->bUseAsyncCooking = false;

        UKismetProceduralMeshLibrary::CopyProceduralMeshFromStaticMeshComponent(DummyStaticMeshComp, 0, NewProcMesh, true);

        NewProcMesh->bUseComplexAsSimpleCollision = false;
        NewProcMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));
        NewProcMesh->ComponentTags.Add(TEXT("Sliceable"));

        NewProcMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
        NewProcMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
        NewProcMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

        FBoxSphereBounds Bounds = LoadedMesh->GetBounds();
        float BottomZ = Bounds.Origin.Z - Bounds.BoxExtent.Z;
        float TargetZ = SurfaceHeightOffset - BottomZ + 0.5f;

        NewProcMesh->SetRelativeLocation(FVector(0.f, 0.f, TargetZ));
        ProcMeshes.Add(NewProcMesh);

        float IngredientTopZ = TargetZ + (Bounds.BoxExtent.Z * 2.0f);
        FVector LaserLoc = AimingPlane->GetRelativeLocation();
        LaserLoc.Z = IngredientTopZ + 2.0f;
        AimingPlane->SetRelativeLocation(LaserLoc);

        AimingPlane->SetVisibility(true);
        CurrentState = ECuttingState::Aiming;

        // 4. 切換視角與鎖定玩家
        if (MyChar)
        {
            MyChar->SetUIInputMode(false);
            MyChar->bUseControllerRotationYaw = false;
        }

        APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
        if (PC)
        {
            PC->SetIgnoreMoveInput(true);
            PC->SetViewTargetWithBlend(this, 0.5f, EViewTargetBlendFunction::VTBlend_Cubic);
        }
    }
}
void ACuttingBoardActor::ExecuteCut()
{
    if (CurrentState != ECuttingState::Aiming || ProcMeshes.Num() == 0) return;

    FVector CutPosition = AimingPlane->GetComponentLocation();
    FVector CutNormal = AimingPlane->GetRightVector();

    TArray<UProceduralMeshComponent*> MeshesToCut = ProcMeshes;
    bool bHasCut = false;

    // 遍歷所有需要切割的網格
    for (UProceduralMeshComponent* MeshToCut : MeshesToCut)
    {
        if (!MeshToCut) continue;

        UMaterialInterface* CapMaterial = MeshToCut->GetMaterial(0);
        UProceduralMeshComponent* OutHalfMesh;
        UKismetProceduralMeshLibrary::SliceProceduralMesh(MeshToCut, CutPosition, CutNormal, true, OutHalfMesh, EProcMeshSliceCapOption::CreateNewSectionForCap, CapMaterial);

        if (OutHalfMesh)
        {
            OutHalfMesh->bUseAsyncCooking = false;

            MeshToCut->bUseComplexAsSimpleCollision = false;
            OutHalfMesh->bUseComplexAsSimpleCollision = false;

            MeshToCut->SetCollisionProfileName(TEXT("BlockAllDynamic"));
            OutHalfMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));

            MeshToCut->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            OutHalfMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

            OutHalfMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
            OutHalfMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
            MeshToCut->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
            MeshToCut->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

            OutHalfMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
            MeshToCut->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

            MeshToCut->SetUseCCD(true);
            OutHalfMesh->SetUseCCD(true);

            MeshToCut->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
            OutHalfMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

            // ==========================================
            // 【修復 1】：把 Z 軸抬升距離從 3.0f 加大到 8.0f，確保完全脫離砧板表面
            // ==========================================
            FVector HorizontalOffset = CutNormal * 3.0f;
            FVector LiftUpOffset = FVector(0.0f, 0.0f, 8.0f);

            MeshToCut->AddWorldOffset(-HorizontalOffset + LiftUpOffset, false, nullptr, ETeleportType::TeleportPhysics);
            OutHalfMesh->AddWorldOffset(HorizontalOffset + LiftUpOffset, false, nullptr, ETeleportType::TeleportPhysics);

            // ==========================================
            // 【修復 2】：順序對調！先開啟物理，再把動能歸零！
            // ==========================================
            // 1. 正式開啟物理模擬 (如果引擎覺得有重疊，此時會產生巨大初速)
            MeshToCut->SetSimulatePhysics(true);
            OutHalfMesh->SetSimulatePhysics(true);

            // 2. 立刻強制將動能歸零！沒收物理引擎剛剛產生的暴走速度
            MeshToCut->SetPhysicsLinearVelocity(FVector::ZeroVector);
            OutHalfMesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
            MeshToCut->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
            OutHalfMesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);

            // 3. 現在才套用我們自己設定的溫柔推力
            FVector PushForce = CutNormal * 7.0f;
            MeshToCut->AddImpulse(-PushForce, NAME_None, true);
            OutHalfMesh->AddImpulse(PushForce, NAME_None, true);

            OutHalfMesh->ComponentTags.Add(TEXT("Sliceable"));
            ProcMeshes.Add(OutHalfMesh);
            bHasCut = true;
        }
    }

    if (bHasCut)
    {
        CutCount++;

        if (CutCount >= 3)
        {
            AimingPlane->SetVisibility(false);
            CurrentState = ECuttingState::ReadyToCollect;
            FinishCutting();
        }
    }
}

void ACuttingBoardActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (CurrentState == ECuttingState::Aiming && CurrentInteractor)
    {
        AMyCharacter* MyChar = Cast<AMyCharacter>(CurrentInteractor);
        if (MyChar)
        {
            APlayerController* PC = Cast<APlayerController>(MyChar->GetController());
            if (PC)
            {
                AimingPlane->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));

                float MouseDeltaX, MouseDeltaY;
                PC->GetInputMouseDelta(MouseDeltaX, MouseDeltaY);

                if (FMath::Abs(MouseDeltaX) > 0.0f)
                {
                    FVector CurrentLoc = AimingPlane->GetRelativeLocation();
                    float MoveSpeed = 2.0f;
                    CurrentLoc.X -= MouseDeltaX * MoveSpeed;

                    float MaxDistance = 30.0f;
                    CurrentLoc.X = FMath::Clamp(CurrentLoc.X, -MaxDistance, MaxDistance);

                    AimingPlane->SetRelativeLocation(CurrentLoc);
                }
            }
        }
    }
}

void ACuttingBoardActor::FinishCutting()
{
    if (CurrentInteractor)
    {
        AMyCharacter* MyChar = Cast<AMyCharacter>(CurrentInteractor);
        if (MyChar)
        {
            MyChar->bUseControllerRotationYaw = true;

            APlayerController* PC = Cast<APlayerController>(MyChar->GetController());
            if (PC)
            {
                PC->ResetIgnoreMoveInput();
                PC->ResetIgnoreLookInput();
                PC->SetViewTargetWithBlend(MyChar, 0.5f, EViewTargetBlendFunction::VTBlend_Cubic);
            }
        }
    }
}