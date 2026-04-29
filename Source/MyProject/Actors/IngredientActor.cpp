#include "IngredientActor.h"
#include "../InventoryComponent.h" 
#include "../TutorialManager.h"
#include "Kismet/GameplayStatics.h"
AIngredientActor::AIngredientActor()
{
	PrimaryActorTick.bCanEverTick = false;
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;
	IngredientID = TEXT("DefaultIngredient");
}

void AIngredientActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (IngredientDataTable && IngredientID != NAME_None)
	{
		FIngredientData* Data = IngredientDataTable->FindRow<FIngredientData>(IngredientID, TEXT("IngredientLookup"));

		if (Data && !Data->IngredientMesh.IsNull())
		{
			MeshComp->SetStaticMesh(Data->IngredientMesh.LoadSynchronous());
		}
	}
}
void AIngredientActor::Interact(AActor* Interactor)
{
	if (Interactor)
	{
		UInventoryComponent* Inv = Interactor->FindComponentByClass<UInventoryComponent>();
		if (Inv)
		{
			Inv->AddIngredient(IngredientID);

			ATutorialManager* TutManager = Cast<ATutorialManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ATutorialManager::StaticClass()));
			if (TutManager)
			{
				TutManager->CompleteStep(ETutorialStep::GrabIngredient);
			}

			Destroy();
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("2. no UInventoryComponent!"));
		}
	}
}