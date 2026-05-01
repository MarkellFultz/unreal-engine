#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../InteractInterface.h"
#include "../GameDataStructs.h"
#include "CustomerNPC.generated.h"

UCLASS(Blueprintable)
class MYPROJECT_API ACustomerNPC : public ACharacter, public IInteractInterface
{
    GENERATED_BODY()

public:
    ACustomerNPC();

    // The data table row ID for this specific NPC
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nutrition|NPC")
    FName CustomerId;
    virtual FText GetInteractPrompt() const override
    {
        return FText::FromString(TEXT("按 E 接受訂單"));
    }
    // --- IInteractInterface Implementation ---
    // This matches the function in your uploaded InteractInterface.h
    virtual void Interact(AActor* Interactor) override;

    // Optional: If you updated the interface to include a prompt
    // virtual FText GetInteractPrompt() const override;

protected:
    // Function to handle the actual order logic (User Story 2.1)
    void TriggerOrderProcessing();
};
