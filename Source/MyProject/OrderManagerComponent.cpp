#include "OrderManagerComponent.h"

UOrderManagerComponent::UOrderManagerComponent()
{
    // Disable ticking for optimization as per system design [cite: 252]
    PrimaryComponentTick.bCanEverTick = false;
}

/** * Implementation of User Story 2.1: Receiving customer orders [cite: 328, 336]
 */
bool UOrderManagerComponent::ReceiveOrder(FName CustomerId)
{
    if (!CustomerTable)
    {
        UE_LOG(LogTemp, Error, TEXT("OrderManager: CustomerTable is NOT assigned!"));
        return false;
    }

    // Context string for data retrieval [cite: 1270]
    static const FString ContextString(TEXT("Customer Order Context"));
    FCustomerProfileData* FoundCustomer = CustomerTable->FindRow<FCustomerProfileData>(CustomerId, ContextString);

    if (FoundCustomer)
    {
        // Store current order to provide data for the UI bubble (User Story 2.3, 2.4) [cite: 367, 381]
        CurrentActiveOrder = *FoundCustomer;

        // Log the received order status [cite: 336]
        UE_LOG(LogTemp, Log, TEXT("Order Received: %s"), *CurrentActiveOrder.CustomerName.ToString());
        return true;
    }

    UE_LOG(LogTemp, Warning, TEXT("OrderManager: Customer ID [%s] not found."), *CustomerId.ToString());
    return false;
}

/** * Combines HealthTags and Allergens for the HUD display (User Story 2.2 & 2.5) [cite: 348, 394]
 */
FText UOrderManagerComponent::GetCustomerHealthRequirements() const
{
    // Formats requirements to support health education feedback [cite: 1106, 1144]
    FString RequirementString = FString::Printf(TEXT("Health: %s\nAllergens: %s"),
        *CurrentActiveOrder.HealthTags,
        *CurrentActiveOrder.AllergenList);

    return FText::FromString(RequirementString);
}