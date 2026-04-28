#include "InventoryComponent.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::AddIngredient(FName IngredientID)
{
	InventoryItems.Add(IngredientID);
	// 修正點：將 \N 改為 \n (換行) 或直接移除，並使用簡單字串測試
	UE_LOG(LogTemp, Warning, TEXT("Item Added: %s"), *IngredientID.ToString());
}

// 配合上一步的修改，改名為 ConsumeIngredients 徹底避開名稱衝突
bool UInventoryComponent::ConsumeIngredients(TArray<FName> IngredientsToRemove)
{
	// 1. 先安全檢查：確認所有要扣除的食材背包裡都有
	for (FName Item : IngredientsToRemove)
	{
		if (!InventoryItems.Contains(Item))
		{
			UE_LOG(LogTemp, Error, TEXT("Consume Failed: %s not found in inventory"), *Item.ToString());
			return false;
		}
	}

	// 2. 確定全部都有之後，才真正執行扣除
	for (FName Item : IngredientsToRemove)
	{
		InventoryItems.RemoveSingle(Item);
	}

	UE_LOG(LogTemp, Warning, TEXT("Ingredients consumed successfully."));
	return true;
}

TArray<FName> UInventoryComponent::GetInventoryItems() const
{
	return InventoryItems;
}