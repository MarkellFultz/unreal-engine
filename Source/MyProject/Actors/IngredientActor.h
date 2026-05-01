#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h" 
#include "../InteractInterface.h" 
#include "../GameDataStructs.h"
#include "IngredientActor.generated.h"



UCLASS()
class MYPROJECT_API AIngredientActor : public AActor, public IInteractInterface
{
	GENERATED_BODY()

public:
	AIngredientActor();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Interact(AActor* Interactor) override;
	virtual FText GetInteractPrompt() const override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ingredient")
	UDataTable* IngredientDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ingredient")
	FName IngredientID;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComp;
};