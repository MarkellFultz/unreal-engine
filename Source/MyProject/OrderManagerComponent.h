#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameDataStructs.h" 
#include "OrderManagerComponent.generated.h"

/**
 * Handles User Story 2.1 - 2.5: Order Reception and Management
 * Cites data from CustomerProfileData table [cite: 1272, 1282]
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MYPROJECT_API UOrderManagerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UOrderManagerComponent();

    /** Data Table Reference for Customer Profiles [cite: 1281] */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nutrition|Data")
    class UDataTable* CustomerTable;

    /** Current active customer data for the UI binding [cite: 1282] */
    UPROPERTY(BlueprintReadOnly, Category = "Nutrition|Order")
    FCustomerProfileData CurrentActiveOrder;

    /** * Function to trigger a new order (User Story 2.1) [cite: 261, 328]
     * @param CustomerId - The Row Name from the Customer DataTable
     */
    UFUNCTION(BlueprintCallable, Category = "Nutrition|Order")
    bool ReceiveOrder(FName CustomerId);

    /** * Formats health restrictions into a readable string (User Story 2.2, 2.5) [cite: 345, 394]
     */
    UFUNCTION(BlueprintPure, Category = "Nutrition|Order")
    FText GetCustomerHealthRequirements() const;
};