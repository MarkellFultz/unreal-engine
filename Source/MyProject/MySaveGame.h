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
	UPROPERTY(VisibleAnywhere, Category = "Progress")
	TArray<FName> SavedInventory;

	UPROPERTY(VisibleAnywhere, Category = "Progress")
	int32 CurrentTutorialStep;

	// 系統設定變數 (12.3, 12.4)
	UPROPERTY(VisibleAnywhere, Category = "Settings")
	float MasterVolume = 1.0f;

	UPROPERTY(VisibleAnywhere, Category = "Settings")
	float MouseSensitivity = 1.0f;
};