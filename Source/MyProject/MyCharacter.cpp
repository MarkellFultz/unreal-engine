#include "MyCharacter.h"
#include "Components/InputComponent.h"
#include "Engine/World.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"

AMyCharacter::AMyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
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

	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &AMyCharacter::OnInteract);
}

void AMyCharacter::OnInteract()
{
	if (CutMontage) { PlayAnimMontage(CutMontage); }

	APlayerCameraManager* CamManager = GetWorld()->GetFirstPlayerController()->PlayerCameraManager;
	FVector Start = CamManager->GetCameraLocation();
	FRotator Rotation = CamManager->GetCameraRotation();

	FVector End = Start + (Rotation.Vector() * TraceDistance);

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	//DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 3.0f, 0, 0.5f);

	if (GetWorld() && GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params))
	{
		AActor* HitActor = HitResult.GetActor();

		if (HitActor)
		{
			if (GEngine)
			{
				//FString Msg = FString::Printf(TEXT("hit: %s"), *HitActor->GetName());
				//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, Msg);
			}

			if (HitActor->ActorHasTag(FName("Sliceable")))
			{
				//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("pass"));

				UProceduralMeshComponent* ProcMesh = Cast<UProceduralMeshComponent>(HitActor->GetComponentByClass(UProceduralMeshComponent::StaticClass()));

				if (ProcMesh)
				{
					SliceObject(ProcMesh, HitResult.ImpactPoint, GetActorRightVector());
					//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Cyan, TEXT("complete"));
				}
				else
				{
					//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("failed no ProceduralMesh "));
				}
			}
			else
			{
				//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("failed no Sliceable tag"));
			}
		}
	}
	else
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("too short"));
	}
}

void AMyCharacter::SliceObject(UProceduralMeshComponent* TargetMesh, FVector PlanePosition, FVector PlaneNormal)
{
	if (!TargetMesh) return;

	UProceduralMeshComponent* OtherHalfMesh = nullptr;

	UMaterialInterface* InsideMaterial = TargetMesh->GetMaterial(1);

	if (!InsideMaterial)
	{
		InsideMaterial = TargetMesh->GetMaterial(0);
	}

	UKismetProceduralMeshLibrary::SliceProceduralMesh(
		TargetMesh,
		PlanePosition,
		PlaneNormal,
		true,
		OtherHalfMesh,
		EProcMeshSliceCapOption::CreateNewSectionForCap,
		InsideMaterial
	);

	TargetMesh->SetSimulatePhysics(true);

	if (OtherHalfMesh)
	{
		OtherHalfMesh->SetSimulatePhysics(true);
		OtherHalfMesh->SetCollisionProfileName(TEXT("BlockAll"));

		OtherHalfMesh->AddImpulse(PlaneNormal * 150.f, NAME_None, true);
		TargetMesh->AddImpulse(PlaneNormal * -150.f, NAME_None, true);
	}
}