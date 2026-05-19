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
	//  MySaveGame.h  UPROPERTY 跑计跋干硂︽
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Player Progress")
	TMap<FName, float> LevelTimes; // Key: 闽(Cust_01), Value: 硄闽计(疊翴计)
};