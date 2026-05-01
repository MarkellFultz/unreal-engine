#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractInterface.generated.h"

UINTERFACE(MinimalAPI)
class UInteractInterface : public UInterface
{
	GENERATED_BODY()
};

class MYPROJECT_API IInteractInterface
{
	GENERATED_BODY()

public:
	// (User Story 2.1)
	virtual void Interact(AActor* Interactor) = 0;

	//  HUD (User Story 3.1-1)
	virtual FText GetInteractPrompt() const = 0;
};