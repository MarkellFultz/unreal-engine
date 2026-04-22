// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyCharacter.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"
/**
 * 
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MYPROJECT_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()
public:
    UInventoryComponent();

    // food in back
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void AddIngredient(FName IngredientID);

    // remove food (on tool
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RemoveIngredients(TArray<FName> IngredientsToRemove);

    // get food 
    UFUNCTION(BlueprintPure, Category = "Inventory")
    TArray<FName> GetInventoryItems() const;

protected:
    // restore food ID array
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    TArray<FName> InventoryItems;
};


