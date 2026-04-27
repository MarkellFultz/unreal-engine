#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../InteractInterface.h"

#include "CookingToolActor.generated.h" // 👈 這行永遠要是最後一個 include

class UQTEComponent;

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

	// 鍋具專屬的 QTE 組件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QTE")
	UQTEComponent* QTEComp;

	// 用來通知藍圖生成 QTE UI 的事件
	UFUNCTION(BlueprintImplementableEvent, Category = "QTE")
	void OnQTEStarted();

	// 👇 讓藍圖可以呼叫隨機觸發 QTE 的函式
	UFUNCTION(BlueprintCallable, Category = "QTE")
	void TriggerRandomQTE();

protected:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cooking")
	TArray<FName> StoredIngredients;

	// 👇 隨機 QTE 所需的變數與函式 (確保只有這裡有一份)
	FTimerHandle QTE_TimerHandle;

	UPROPERTY()
	int32 QTECount = 0;

	UPROPERTY()
	AActor* CurrentInteractor;

	// 烹飪完成的函式
	void FinishCooking();
};