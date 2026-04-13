#include "CookingToolActor.h"
#include "../InventoryComponent.h"

ACookingToolActor::ACookingToolActor()
{
	PrimaryActorTick.bCanEverTick = false;
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;
}

void ACookingToolActor::Interact(AActor* Interactor)
{
	OnOpenCookingUI(Interactor);
}

void ACookingToolActor::AcceptIngredients(AActor* Interactor, TArray<FName> SelectedItems)
{
	if (Interactor && SelectedItems.Num() > 0)
	{
		UInventoryComponent* Inv = Interactor->FindComponentByClass<UInventoryComponent>();
		if (Inv && Inv->RemoveIngredients(SelectedItems))
		{
			StoredIngredients.Append(SelectedItems);
		}
	}
}