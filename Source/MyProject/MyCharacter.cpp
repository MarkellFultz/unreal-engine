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
#include "KismetProceduralMeshLibrary.h" // Added for UKismetProceduralMeshLibrary
#include "Blueprint/UserWidget.h"
#include "TutorialManager.h"
AMyCharacter::AMyCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    InventoryComp = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComp"));
}

void AMyCharacter::BeginPlay()
{
    Super::BeginPlay();


}

void AMyCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Bind interaction
    PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &AMyCharacter::OnInteract);
    PlayerInputComponent->BindAction("Inventory", IE_Pressed, this, &AMyCharacter::ToggleInventory);
    // Bind movement axes (Make sure these names match your Project Settings -> Input)
    PlayerInputComponent->BindAxis("MoveForward", this, &AMyCharacter::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &AMyCharacter::MoveRight);
}

void AMyCharacter::MoveForward(float Value)
{

    if (Value != 0.0f)
    {
        /*if (!bHasCompletedMoveTutorial)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Orange, TEXT("1. 0.0..."));

            ATutorialManager* TutManager = Cast<ATutorialManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATutorialManager::StaticClass()));

            if (TutManager)
            {
                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, TEXT("2. find CompleteStep(Move)"));

                TutManager->CompleteStep(ETutorialStep::Move);
                bHasCompletedMoveTutorial = true; 
            }
            else
            {
                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("2. err Tutorial Manager¡I"));
            }
        }*/
        // Apply movement
        AddMovementInput(GetActorForwardVector(), Value);

        // Check and trigger the movement tutorial step
        if (!bHasCompletedMoveTutorial)
        {
            bHasCompletedMoveTutorial = true; // Lock the flag immediately

            ATutorialManager* TutManager = Cast<ATutorialManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATutorialManager::StaticClass()));
            if (TutManager)
            {
                TutManager->CompleteStep(ETutorialStep::Move);
            }
        }
    }
}

void AMyCharacter::MoveRight(float Value)
{
    if (Value != 0.0f)
    {
        // Apply movement
        AddMovementInput(GetActorRightVector(), Value);

        // Check and trigger the movement tutorial step (Handles cases where player presses A/D first)
        if (!bHasCompletedMoveTutorial)
        {
            bHasCompletedMoveTutorial = true; // Lock the flag immediately

            ATutorialManager* TutManager = Cast<ATutorialManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATutorialManager::StaticClass()));
            if (TutManager)
            {
                TutManager->CompleteStep(ETutorialStep::Move);
            }
        }
    }
}

void AMyCharacter::OnInteract()
{
    if (CutMontage)
    {
        PlayAnimMontage(CutMontage);
    }

    APlayerCameraManager* CamManager = GetWorld()->GetFirstPlayerController()->PlayerCameraManager;
    FVector Start = CamManager->GetCameraLocation();
    FRotator Rotation = CamManager->GetCameraRotation();
    FVector End = Start + (Rotation.Vector() * TraceDistance);

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    /*DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 3.0f, 0, 0.5f);*/

    if (GetWorld() && GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params))
    {
        AActor* HitActor = HitResult.GetActor();
        if (HitActor)
        {
            IInteractInterface* Interactable = Cast<IInteractInterface>(HitActor);
            if (Interactable)
            {
                Interactable->Interact(this);
                return;
            }

            if (HitActor->ActorHasTag(FName("Sliceable")))
            {
                UProceduralMeshComponent* ProcMesh = Cast<UProceduralMeshComponent>(HitActor->GetComponentByClass(UProceduralMeshComponent::StaticClass()));
                if (ProcMesh)
                {
                    SliceObject(ProcMesh, HitResult.ImpactPoint, GetActorRightVector());
                }
            }
        }
    }
}

void AMyCharacter::SliceObject(UProceduralMeshComponent* TargetMesh, FVector PlanePosition, FVector PlaneNormal)
{
    if (!TargetMesh)
    {
        return;
    }

    UProceduralMeshComponent* OtherHalfMesh = nullptr;
    UMaterialInterface* InsideMaterial = TargetMesh->GetMaterial(1);
    if (!InsideMaterial)
    {
        InsideMaterial = TargetMesh->GetMaterial(0);
    }

    UKismetProceduralMeshLibrary::SliceProceduralMesh(
        TargetMesh, PlanePosition, PlaneNormal, true, OtherHalfMesh,
        EProcMeshSliceCapOption::CreateNewSectionForCap, InsideMaterial
    );

    TargetMesh->SetSimulatePhysics(true);
    if (OtherHalfMesh)
    {
        OtherHalfMesh->SetSimulatePhysics(true);
        OtherHalfMesh->SetCollisionProfileName(TEXT("BlockAll"));
        OtherHalfMesh->AddImpulse(PlaneNormal * 150.f, NAME_None, true);
        TargetMesh->AddImpulse(PlaneNormal * -150.f, NAME_None, true);

        ATutorialManager* TutManager = Cast<ATutorialManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATutorialManager::StaticClass()));
        if (TutManager)
        {
            TutManager->CompleteStep(ETutorialStep::Chop);
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

        if (PC)
        {
            PC->SetShowMouseCursor(false);
            PC->SetInputMode(FInputModeGameOnly());
        }
    }
    else if (InventoryWidgetClass)
    {
        InventoryWidgetInstance = CreateWidget<UUserWidget>(GetWorld(), InventoryWidgetClass);
        if (InventoryWidgetInstance)
        {
            InventoryWidgetInstance->AddToViewport();

            ATutorialManager* TutManager = Cast<ATutorialManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATutorialManager::StaticClass()));
            if (TutManager)
            {
                TutManager->CompleteStep(ETutorialStep::Backpack);
            }

            if (PC)
            {
                PC->SetShowMouseCursor(true);
                FInputModeGameAndUI InputMode;
                InputMode.SetWidgetToFocus(InventoryWidgetInstance->TakeWidget());
                PC->SetInputMode(InputMode);
            }
        }
    }
}