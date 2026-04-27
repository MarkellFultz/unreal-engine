#include "MyPlayerController.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/GameplayStatics.h"
#include "MySaveGame.h"
#include "MyCharacter.h"
#include "Sound/SoundMix.h"

// 12.1 按鍵設定 (基於 Enhanced Input 的邏輯)
void AMyPlayerController::RebindActionKey(FName ActionName, FKey NewKey)
{
    // 這裡通常需要獲取你的 Input Mapping Context 並動態修改
    // 實作較複雜，建議配合 UI 的 Key Selector 使用
}

// 12.2 視窗設定
void AMyPlayerController::SetWindowSettings(int32 Width, int32 Height, int32 WindowMode)
{
    UGameUserSettings* Settings = GEngine->GetGameUserSettings();
    if (Settings)
    {
        Settings->SetScreenResolution(FIntPoint(Width, Height));
        Settings->SetFullscreenMode((EWindowMode::Type)WindowMode);
        Settings->ApplySettings(false);
    }
}

// 12.3 音量設定
void AMyPlayerController::SetMasterVolume(float Volume)
{
    // 這邊會去控制 Sound Class Mix
    UGameplayStatics::SetSoundMixClassOverride(GetWorld(), nullptr, nullptr, Volume, 1.0f, 0.0f, true);
}

// 12.4 靈敏度設定
void AMyPlayerController::SetSensitivity(float Sensitivity)
{
    // 直接修改 Controller 的靈敏度倍率
    InputYawScale_DEPRECATED = 0.5f * Sensitivity;
    InputPitchScale_DEPRECATED = 0.5f * Sensitivity;
}

// 12.5 儲存遊戲
void AMyPlayerController::SaveGameData()
{
    UMySaveGame* SaveInstance = Cast<UMySaveGame>(UGameplayStatics::CreateSaveGameObject(UMySaveGame::StaticClass()));
    AMyCharacter* MyChar = Cast<AMyCharacter>(GetPawn());

    if (SaveInstance && MyChar)
    {
        // 抓取背包資料 (假設你有 InventoryComp)
        // SaveInstance->SavedInventory = MyChar->InventoryComp->GetItems();
        
        UGameplayStatics::SaveGameToSlot(SaveInstance, TEXT("Slot1"), 0);
    }
}

// 12.6 讀取遊戲
void AMyPlayerController::LoadGameData()
{
    UMySaveGame* LoadInstance = Cast<UMySaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("Slot1"), 0));
    if (LoadInstance)
    {
        // 應用讀取到的資料
        SetMasterVolume(LoadInstance->MasterVolume);
        SetSensitivity(LoadInstance->MouseSensitivity);
    }
}

// 12.7 關閉遊戲
void AMyPlayerController::QuitGame()
{
    UKismetSystemLibrary::QuitGame(GetWorld(), this, EQuitPreference::Quit, false);
}