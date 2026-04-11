#include "InventoryComponent.h"

AInventoryComponent::AInventoryComponent()
{
    AInventoryComponent.bCanEverTick = false;
}

void AInventoryComponent::AddIngredient(FName IngredientID)
{
    InventoryItems.Add(IngredientID);
    UE_LOG(LogTemp, Warning, TEXT("Added %s to Inventory!"), *IngredientID.ToString());
}

bool AInventoryComponent::RemoveIngredients(TArray<FName> IngredientsToRemove)
{
    // 為了安全起見，先檢查是否有足夠的食材
    for (FName Item : IngredientsToRemove)
    {
        if (!InventoryItems.Contains(Item))
        {
            return false; // 缺少食材，操作失敗
        }
    }

    // 確定都有之後再進行移除
    for (FName Item : IngredientsToRemove)
    {
        InventoryItems.RemoveSingle(Item); // 只移除一個該名稱的食材
    }
    return true;
}

TArray<FName> AInventoryComponent::GetInventoryItems() const
{
    return InventoryItems;
}