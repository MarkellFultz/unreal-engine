#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractInterface.h"
#include "DoorActor.generated.h"

UCLASS()
class MYPROJECT_API ADoorActor : public AActor, public IInteractInterface
{
	GENERATED_BODY()

public:
	ADoorActor();
	virtual void Interact(AActor* Interactor) override;
	virtual FText GetInteractPrompt() const override;

	// 留一個空位給藍圖，當通過驗證時，讓藍圖去執行「切換關卡」或「播放開門動畫」
	UFUNCTION(BlueprintImplementableEvent, Category = "Door")
	void OnProceedToCookingStage();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* DoorMesh;
};