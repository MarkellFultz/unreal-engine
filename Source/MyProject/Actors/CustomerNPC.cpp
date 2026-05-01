#include "CustomerNPC.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameModeBase.h"
#include "../OrderManagerComponent.h"

ACustomerNPC::ACustomerNPC()
{
    // Ensure the NPC can be detected by LineTrace
    PrimaryActorTick.bCanEverTick = true;
}

/** * Implementation of the Interact function from IInteractInterface
 * Triggered when player focuses on NPC and presses 'E'
 */
void ACustomerNPC::Interact(AActor* Interactor)
{
    if (!Interactor) return;

    UE_LOG(LogTemp, Log, TEXT("Interacting with Customer: %s"), *CustomerId.ToString());

    TriggerOrderProcessing();
}

/** * Business logic for receiving an order (User Story 2.1)
 */
void ACustomerNPC::TriggerOrderProcessing()
{
    // Typically, the OrderManager would be on the GameMode or a dedicated Actor
    // For this example, we find the OrderManager to pass our CustomerId
    AGameModeBase* GameModeActor = UGameplayStatics::GetGameMode(GetWorld());
    if (GameModeActor)
    {
        UOrderManagerComponent* OrderManager = GameModeActor->FindComponentByClass<UOrderManagerComponent>();
        if (OrderManager)
        {
            // Assign this NPC's ID to the active order system
            OrderManager->ReceiveOrder(CustomerId);
        }
    }
}