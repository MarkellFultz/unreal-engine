#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../InteractInterface.h"
#include "IngredientActor.generated.h"

UCLASS()
class MYPROJECT_API AIngredientActor : public AActor, public IInteractInterface
{
	GENERATED_BODY()
public:
	AIngredientActor();
	virtual void Interact(AActor* Interactor) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ingredient")
	FName IngredientID;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComp;
};