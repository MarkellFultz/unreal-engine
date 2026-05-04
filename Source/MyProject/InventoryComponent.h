// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MYPROJECT_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

	// 加入食材到背包
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void AddIngredient(FName IngredientID);

	// 消耗(移除)指定的食材陣列，若成功刪除則回傳 true
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool ConsumeIngredients(TArray<FName> IngredientsToRemove);

	// 取得背包內所有食材
	UFUNCTION(BlueprintPure, Category = "Inventory")
	TArray<FName> GetInventoryItems() const;
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveIngredient(FName ItemID);
protected:
	// 儲存食材 ID 的陣列
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TArray<FName> InventoryItems;
};