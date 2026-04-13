#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../InteractInterface.h"
#include "CookingToolActor.generated.h"

UCLASS()
class MYPROJECT_API ACookingToolActor : public AActor, public IInteractInterface
{
	GENERATED_BODY()
public:
	ACookingToolActor();
	virtual void Interact(AActor* Interactor) override;

	UFUNCTION(BlueprintCallable, Category = "Cooking")
	void AcceptIngredients(AActor* Interactor, TArray<FName> SelectedItems);

	UFUNCTION(BlueprintImplementableEvent, Category = "Cooking")
	void OnOpenCookingUI(AActor* Interactor);

protected:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cooking")
	TArray<FName> StoredIngredients;
};