#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractableInterface.generated.h"

UINTERFACE(MinimalAPI)
class UInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

class MYPROJECT_API IInteractableInterface
{
	GENERATED_BODY()

public:
	/** Triggered when the player presses the interaction key (e.g., 'E') */
	virtual void Interact(AActor* Interactor) = 0;

	/** Optional: Show UI prompt when looking at the object */
	virtual FText GetInteractPrompt() const = 0;
};
