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
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Player Progress")
	TMap<FName, float> LevelTimes; // Key: 關卡名(如Cust_01), Value: 通關秒數(浮點數)
};