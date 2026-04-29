#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MyPlayerController.generated.h"

UCLASS()
class MYPROJECT_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	// --- 12.1 按鍵設定 ---
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void RebindActionKey(FName ActionName, FKey NewKey);

	// --- 12.2 視窗設定 ---
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void SetWindowSettings(int32 Width, int32 Height, int32 WindowMode);

	// --- 12.3 音量設定 ---
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void SetMasterVolume(float Volume);

	// --- 12.4 靈敏度設定 ---
	UFUNCTION(BlueprintCallable, Category = "Settings")
	void SetSensitivity(float Sensitivity);

	// --- 12.5 & 12.6 儲存與讀取 ---
	UFUNCTION(BlueprintCallable, Category = "SaveGame")
	void SaveGameData();

	UFUNCTION(BlueprintCallable, Category = "SaveGame")
	void LoadGameData();

	// --- 12.7 關閉遊戲 ---
	UFUNCTION(BlueprintCallable, Category = "Game")
	void QuitGame();
};