#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "MySaveGame.generated.h"

UCLASS()
class MYPROJECT_API UMySaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UMySaveGame();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tutorial")
	bool bHasCompletedTutorial;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Player Settings")
	float MouseSensitivity;
};