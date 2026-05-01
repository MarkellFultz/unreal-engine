#include "MyCharacter.h"
#include "Components/InputComponent.h"
#include "Engine/World.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "InteractInterface.h"
#include "InventoryComponent.h"
#include "Actors/IngredientActor.h"
#include "Actors/CookingToolActor.h"
#include "TutorialManager.h"
#include "Kismet/GameplayStatics.h"
#include "KismetProceduralMeshLibrary.h"
#include "Blueprint/UserWidget.h"
#include "QTEComponent.h"

AMyCharacter::AMyCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    InventoryComp = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComp"));
}

void AMyCharacter::BeginPlay()
{
    Super::BeginPlay();
    if (InventoryComp)
    {
        InventoryComp->AddIngredient(FName("Ing_Chicken"));
    }
}

void AMyCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // 軸向綁定 (舊版 Axis Mapping)
    PlayerInputComponent->BindAxis("MoveForward", this, &AMyCharacter::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &AMyCharacter::MoveRight);
    PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
    PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

    // 你的目標：將空白鍵設定的 QTEAction (Axis) 綁定到邏輯
    // 注意：若 QTEAction 與 Jump 同時綁定 Space，Jump 會優先觸發，因此我們在 Jump 內做攔截
    PlayerInputComponent->BindAxis("QTEAction", this, &AMyCharacter::OnQTEPressedAxis);

    // 動作綁定 (舊版 Action Mapping)
    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMyCharacter::Jump);
    PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &AMyCharacter::OnInteract);
    PlayerInputComponent->BindAction("Inventory", IE_Pressed, this, &AMyCharacter::ToggleInventory);
}

// --- 修改後的 Jump 函數：攔截跳躍並執行判定 ---
void AMyCharacter::Jump()
{
    APlayerController* PC = Cast<APlayerController>(GetController());

    // 前提：QTE 時視角與移動已被鎖定 (IsMoveInputIgnored 為 true)
    if (PC && PC->IsMoveInputIgnored())
    {
        // 既然視角沒偏離，直接呼叫判定邏輯
        OnQTEPressed();
        return; // 攔截跳躍，角色不會跳起來
    }

    Super::Jump(); // 正常狀態下執行跳躍
}

void AMyCharacter::OnInteract()
{
    AActor* Target = GetCurrentInteractable();
    if (!Target) return;

    TArray<UProceduralMeshComponent*> ProcMeshes;
    Target->GetComponents<UProceduralMeshComponent>(ProcMeshes);

    for (UProceduralMeshComponent* ProcMesh : ProcMeshes)
    {
        if (ProcMesh && ProcMesh->ComponentTags.Contains("Sliceable"))
        {
            SliceObject(ProcMesh, Target->GetActorLocation(), GetActorForwardVector());
            return;
        }
    }

    IInteractInterface* Interactable = Cast<IInteractInterface>(Target);
    if (Interactable)
    {
        Interactable->Interact(this);
    }
}

// 支援原本的 Action 呼叫方式
void AMyCharacter::OnQTEPressed()
{
    AActor* InteractionTarget = GetCurrentInteractable();
    if (InteractionTarget)
    {
        UQTEComponent* QTEComp = InteractionTarget->FindComponentByClass<UQTEComponent>();
        if (QTEComp && QTEComp->IsQTEActive())
        {
            QTEComp->AttemptHit(); // 執行判定
        }
    }
}

// 支援你提到的 Axis Mapping QTEAction
void AMyCharacter::OnQTEPressedAxis(float Value)
{
    // Axis 會每影格執行，只有在按下的瞬間 (Value > 0.9) 且移動鎖定時才可能執行
    // 但因為 Jump 已經處理了 IE_Pressed 邏輯，此處可保持空白或用於其他連續性邏輯
}

FText AMyCharacter::GetCurrentInteractPromptText() const
{
    AActor* HitActor = GetCurrentInteractable();
    IInteractInterface* Interactable = Cast<IInteractInterface>(HitActor);
    return Interactable ? Interactable->GetInteractPrompt() : FText::GetEmpty();
}

AActor* AMyCharacter::GetCurrentInteractable() const
{
    if (!GetWorld() || !GetWorld()->GetFirstPlayerController()) return nullptr;
    APlayerCameraManager* CamManager = GetWorld()->GetFirstPlayerController()->PlayerCameraManager;
    if (!CamManager) return nullptr;

    FVector Start = CamManager->GetCameraLocation();
    FVector End = Start + (CamManager->GetCameraRotation().Vector() * TraceDistance);

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params))
    {
        return HitResult.GetActor();
    }
    return nullptr;
}

void AMyCharacter::ToggleInventory()
{
    if (InventoryWidgetInstance && InventoryWidgetInstance->IsInViewport())
    {
        InventoryWidgetInstance->RemoveFromParent();
        InventoryWidgetInstance = nullptr;
        SetUIInputMode(false);
    }
    else if (InventoryWidgetClass)
    {
        InventoryWidgetInstance = CreateWidget<UUserWidget>(GetWorld(), InventoryWidgetClass);
        if (InventoryWidgetInstance)
        {
            InventoryWidgetInstance->AddToViewport();
            SetUIInputMode(true);
            ATutorialManager* TutManager = Cast<ATutorialManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATutorialManager::StaticClass()));
            if (TutManager) TutManager->CompleteStep(ETutorialStep::Backpack);
        }
    }
}

void AMyCharacter::SetUIInputMode(bool bIsUIMode)
{
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC) return;
    if (bIsUIMode)
    {
        FInputModeGameAndUI InputMode;
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = true;
    }
    else
    {
        PC->SetInputMode(FInputModeGameOnly());
        PC->bShowMouseCursor = false;
    }
}

void AMyCharacter::MoveForward(float Value) { AddMovementInput(GetActorForwardVector(), Value); }
void AMyCharacter::MoveRight(float Value) { AddMovementInput(GetActorRightVector(), Value); }

void AMyCharacter::SliceObject(UProceduralMeshComponent* TargetMesh, FVector PlanePosition, FVector PlaneNormal)
{
    if (!TargetMesh) return;
    UProceduralMeshComponent* OutHalfMesh;
    UKismetProceduralMeshLibrary::SliceProceduralMesh(TargetMesh, PlanePosition, PlaneNormal, true, OutHalfMesh, EProcMeshSliceCapOption::CreateNewSectionForCap, nullptr);
    if (OutHalfMesh) OutHalfMesh->SetSimulatePhysics(true);
}