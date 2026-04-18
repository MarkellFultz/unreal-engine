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
	}
}