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

    // Initial inventory setup for testing
    if (InventoryComp)
    {
        InventoryComp->AddIngredient(FName("Ing_Chicken"));
    }
}

void AMyCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 互動提示文字改由 HUD Blueprint Binding 呼叫 GetCurrentInteractPromptText() 取得
}

void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Movement Bindings
    PlayerInputComponent->BindAxis("MoveForward", this, &AMyCharacter::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &AMyCharacter::MoveRight);
    PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
    PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

    // Action Bindings
    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMyCharacter::Jump);
    PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &AMyCharacter::OnInteract);
    PlayerInputComponent->BindAction("Inventory", IE_Pressed, this, &AMyCharacter::ToggleInventory);
    PlayerInputComponent->BindAction("QTEKey", IE_Pressed, this, &AMyCharacter::OnQTEPressed);
}

FText AMyCharacter::GetCurrentInteractPromptText() const
{
    // 每次 HUD Binding 執行時做一次射線，直接回傳提示文字
    AActor* HitActor = GetCurrentInteractable();
    IInteractInterface* Interactable = Cast<IInteractInterface>(HitActor);

    if (Interactable)
    {
        return Interactable->GetInteractPrompt();
    }
    return FText::GetEmpty();
}

AActor* AMyCharacter::GetCurrentInteractable() const
{
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

void AMyCharacter::OnInteract()
{
    AActor* Target = GetCurrentInteractable();
    if (!Target) return;

    // Prioritize Slicing logic (Original implementation)
    UProceduralMeshComponent* HitMesh = Cast<UProceduralMeshComponent>(Target->GetRootComponent());
    if (HitMesh && HitMesh->ComponentTags.Contains("Sliceable"))
    {
        SliceObject(HitMesh, Target->GetActorLocation(), GetActorForwardVector());
        return;
    }

    // Then handle Interface-based interaction (NPCs, Tools, etc.)
    IInteractInterface* Interactable = Cast<IInteractInterface>(Target);
    if (Interactable)
    {
        Interactable->Interact(this);
    }
}

void AMyCharacter::OnQTEPressed()
{
    AActor* InteractionTarget = GetCurrentInteractable();
    if (InteractionTarget)
    {
        UQTEComponent* QTEComp = InteractionTarget->FindComponentByClass<UQTEComponent>();
        if (QTEComp && QTEComp->IsQTEActive())
        {
            QTEComp->AttemptHit();
        }
    }
}

void AMyCharacter::ToggleInventory()
{
    APlayerController* PC = Cast<APlayerController>(GetController());

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

            // Tutorial progress tracking
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

// Basic movement and slicing implementations
void AMyCharacter::MoveForward(float Value) { AddMovementInput(GetActorForwardVector(), Value); }
void AMyCharacter::MoveRight(float Value) { AddMovementInput(GetActorRightVector(), Value); }
void AMyCharacter::Jump() { Super::Jump(); }

void AMyCharacter::SliceObject(UProceduralMeshComponent* TargetMesh, FVector PlanePosition, FVector PlaneNormal)
{
    if (!TargetMesh) return;
    UProceduralMeshComponent* OutHalfMesh;
    UKismetProceduralMeshLibrary::SliceProceduralMesh(TargetMesh, PlanePosition, PlaneNormal, true, OutHalfMesh, EProcMeshSliceCapOption::CreateNewSectionForCap, nullptr);
    if (OutHalfMesh) OutHalfMesh->SetSimulatePhysics(true);
}