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
bool UInventoryComponent::RemoveIngredient(FName ItemID)
{
    // 1. 檢查背包裡有沒有這個東西
    if (InventoryItems.Contains(ItemID))
    {
        // 2. 如果有，就扣除「一個」，然後印出成功訊息，回傳 true
        InventoryItems.RemoveSingle(ItemID);

        // 使用英文印出成功訊息，避免編碼問題
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString::Printf(TEXT("Success! Removed: %s"), *ItemID.ToString()));
        return true;
    }

    // 3. 如果沒有，先印出我們「試圖尋找卻找不到」的 ID
    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Red, FString::Printf(TEXT("Failed! Cannot find: [%s]"), *ItemID.ToString()));

    // 4. 【透視鏡功能】：把背包裡目前所有的東西都印出來讓我們檢查！
    FString InventoryList = TEXT("Current Inventory Contains: ");
    if (InventoryItems.Num() == 0)
    {
        InventoryList += TEXT("EMPTY (Nothing in bag!)");
    }
    else
    {
        for (const FName& Item : InventoryItems)
        {
            InventoryList += TEXT("[") + Item.ToString() + TEXT("] ");
        }
    }

    // 印出黃色的背包清單
    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Yellow, InventoryList);

    return false;
}