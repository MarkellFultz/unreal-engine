#include "IngredientActor.h"
#include "../InventoryComponent.h"

AIngredientActor::AIngredientActor()
{
	PrimaryActorTick.bCanEverTick = false;
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;
	IngredientID = TEXT("DefaultIngredient");
}

void AIngredientActor::Interact(AActor* Interactor)
{
	if (Interactor)
	{
		UInventoryComponent* Inv = Interactor->FindComponentByClass<UInventoryComponent>();
		if (Inv)
		{
			Inv->AddIngredient(IngredientID);
			Destroy();
		}
	}
}