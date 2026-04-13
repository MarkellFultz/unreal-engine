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

bool UInventoryComponent::RemoveIngredients(TArray<FName> IngredientsToRemove)
{
    for (FName Item : IngredientsToRemove)
    {
        if (!InventoryItems.Contains(Item))
        {
            UE_LOG(LogTemp, Error, TEXT("Remove Failed: %s not found"), *Item.ToString());
            return false;
        }
    }

    for (FName Item : IngredientsToRemove)
    {
        InventoryItems.RemoveSingle(Item);
    }
    return true;
}

TArray<FName> UInventoryComponent::GetInventoryItems() const
{
    return InventoryItems;
}