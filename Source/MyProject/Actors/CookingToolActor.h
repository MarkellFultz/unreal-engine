#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../InteractInterface.h"
#include "../GameDataStructs.h"
#include "Camera/CameraComponent.h"

#include "CookingToolActor.generated.h"

class UQTEComponent;

UCLASS()
class MYPROJECT_API ACookingToolActor : public AActor, public IInteractInterface
{
	GENERATED_BODY()
public:
	ACookingToolActor();
	virtual void Interact(AActor* Interactor) override;
	virtual FText GetInteractPrompt() const override;

	// 接收食材的函數 (內部已新增 IsSliceable 驗證邏輯)
	// 把原本的 void 改成 bool
	UFUNCTION(BlueprintCallable, Category = "Cooking")
	bool AcceptIngredients(AActor* Interactor, TArray<FName> SelectedItems);

	// 打開料理 UI
	UFUNCTION(BlueprintImplementableEvent, Category = "Cooking")
	void OnOpenCookingUI(AActor* Interactor);

	// 鍋具專屬的 QTE 組件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QTE")
	UQTEComponent* QTEComp;

	// 用來通知藍圖生成 QTE UI 的事件
	UFUNCTION(BlueprintImplementableEvent, Category = "QTE")
	void OnQTEStarted();

	// 讓藍圖可以呼叫隨機觸發 QTE 的函式
	UFUNCTION(BlueprintCallable, Category = "QTE")
	void TriggerRandomQTE();

protected:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cooking")
	TArray<FName> StoredIngredients;

	// 視覺化食材相關變數
	UPROPERTY(EditAnywhere, Category = "Data")
	UDataTable* IngredientDataTable;

	// ====== 🌲 新增：鍋具配方組合表（用來判定合成什麼菜） ======
	UPROPERTY(EditAnywhere, Category = "Data")
	UDataTable* CookingRecipeDataTable;

	// 用來記錄我們動態生成的食材模型
	UPROPERTY()
	TArray<UStaticMeshComponent*> DisplayedIngredientMeshes;

	// 廚具專屬的俯視相機組件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cooking|Camera")
	UCameraComponent* TopDownCamera;

	// 隨機 QTE 所需的變數
	FTimerHandle QTE_TimerHandle;

	UPROPERTY()
	int32 QTECount = 0;

	UPROPERTY(BlueprintReadWrite, Category = "QTE")
	int32 SuccessfulQTECount = 0;

	// 烹飪結束時，通知藍圖彈出結算畫面的事件
	UFUNCTION(BlueprintImplementableEvent, Category = "Cooking")
	void OnCookingFinished(int32 FinalSuccessCount);

	UPROPERTY()
	AActor* CurrentInteractor;

	// 烹飪完成的內部邏輯處理 (內部已新增產出 _Cooked 食材邏輯)
	void FinishCooking();
	// 🌟🌟🌟 新增：專門給盤子使用的料理完成事件 🌟🌟🌟
	UFUNCTION(BlueprintImplementableEvent, Category = "Cooking")
	void OnPlateDishCompleted(FName DishName);

};