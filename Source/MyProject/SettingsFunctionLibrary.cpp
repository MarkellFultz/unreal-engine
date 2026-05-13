#include "SettingsFunctionLibrary.h"
#include "GameFramework/GameUserSettings.h"
#include "GameFramework/InputSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/PlayerInput.h"
#include "GameFramework/Pawn.h" 
#include "GameFramework/PlayerController.h"
#include "Sound/SoundMix.h"
#include "Sound/SoundClass.h"

// 1. 視窗設定實作
void USettingsFunctionLibrary::SetAndApplyWindowSettings(int32 Width, int32 Height, bool bIsFullscreen)
{
    UGameUserSettings* UserSettings = GEngine->GetGameUserSettings();
    if (UserSettings)
    {
        UserSettings->SetScreenResolution(FIntPoint(Width, Height));
        UserSettings->SetFullscreenMode(bIsFullscreen ? EWindowMode::Fullscreen : EWindowMode::Windowed);
        UserSettings->ApplySettings(true);
    }
}

void USettingsFunctionLibrary::SetWindowModeByIndex(int32 ModeIndex)
{
    UGameUserSettings* UserSettings = GEngine->GetGameUserSettings();
    if (UserSettings)
    {
        EWindowMode::Type NewMode;
        switch (ModeIndex)
        {
        case 0: NewMode = EWindowMode::Windowed; break;
        case 1: NewMode = EWindowMode::WindowedFullscreen; break;
        case 2: NewMode = EWindowMode::Fullscreen; break;
        default: NewMode = EWindowMode::Windowed; break;
        }

        UserSettings->SetFullscreenMode(NewMode);
        UserSettings->ApplySettings(false);
        UserSettings->SaveSettings();
    }
}

int32 USettingsFunctionLibrary::GetCurrentWindowModeIndex()
{
    UGameUserSettings* UserSettings = GEngine->GetGameUserSettings();
    if (UserSettings)
    {
        EWindowMode::Type Mode = UserSettings->GetFullscreenMode();
        if (Mode == EWindowMode::Windowed) return 0;
        if (Mode == EWindowMode::WindowedFullscreen) return 1;
        if (Mode == EWindowMode::Fullscreen) return 2;
    }
    return 0;
}

// 2. 音量設定實作
void USettingsFunctionLibrary::SetGameVolume(USoundMix* Mix, USoundClass* SoundClass, float Volume)
{
    if (Mix && SoundClass)
    {
        UGameplayStatics::SetSoundMixClassOverride(nullptr, Mix, SoundClass, Volume, 1.0f, 0.0f, true);
        UGameplayStatics::PushSoundMixModifier(nullptr, Mix);
    }
}

// 3. 按鍵綁定實作 (Action Mapping)
void USettingsFunctionLibrary::RebindActionMapping(const UObject* WorldContextObject, FName ActionName, FKey NewKey)
{
    UInputSettings* InputSettings = UInputSettings::GetInputSettings();
    if (!InputSettings) return;

    TArray<FInputActionKeyMapping> OutMappings;
    InputSettings->GetActionMappingByName(ActionName, OutMappings);
    for (const FInputActionKeyMapping& Mapping : OutMappings)
    {
        InputSettings->RemoveActionMapping(Mapping, false);
    }

    FInputActionKeyMapping NewMapping;
    NewMapping.ActionName = ActionName;
    NewMapping.Key = NewKey;
    InputSettings->AddActionMapping(NewMapping, true);

    InputSettings->SaveKeyMappings();
    InputSettings->TryUpdateDefaultConfigFile();

    if (WorldContextObject)
    {
        APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0);
        if (PC && PC->PlayerInput)
        {
            PC->PlayerInput->ForceRebuildingKeyMaps(true);
        }
    }
}

// 按鍵綁定實作 (Axis Mapping)
void USettingsFunctionLibrary::RebindAxisMapping(UObject* WorldContextObject, FName AxisName, FKey NewKey, float Scale)
{
    UInputSettings* InputSettings = UInputSettings::GetInputSettings();
    if (!InputSettings) return;

    // 1. 執行存檔邏輯
    const TArray<FInputAxisKeyMapping>& AxisMappings = InputSettings->GetAxisMappings();
    TArray<FInputAxisKeyMapping> MappingsToRemove;
    for (const FInputAxisKeyMapping& Mapping : AxisMappings)
    {
        if (Mapping.AxisName == AxisName) MappingsToRemove.Add(Mapping);
    }
    for (const FInputAxisKeyMapping& Mapping : MappingsToRemove)
    {
        InputSettings->RemoveAxisMapping(Mapping, false);
    }

    FInputAxisKeyMapping NewMapping;
    NewMapping.AxisName = AxisName;
    NewMapping.Key = NewKey;
    NewMapping.Scale = Scale;
    InputSettings->AddAxisMapping(NewMapping, true);

    // 2. 核心：儲存到硬碟檔案[cite: 1]
    InputSettings->SaveKeyMappings();
    InputSettings->TryUpdateDefaultConfigFile();

    // 3. 安全刷新：若目前有 Pawn 則刷新，若無則單純完成存檔
    if (WorldContextObject)
    {
        APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0);
        if (PC && PC->PlayerInput)
        {
            PC->PlayerInput->ForceRebuildingKeyMaps(true);
            APawn* P = PC->GetPawn();
            if (P && !P->IsPendingKillPending())
            {
                P->PawnClientRestart(); //
            }
        }
    }
}

FKey USettingsFunctionLibrary::GetCurrentActionKey(FName ActionName)
{
    UInputSettings* InputSettings = UInputSettings::GetInputSettings();
    if (InputSettings)
    {
        TArray<FInputActionKeyMapping> Mappings;
        InputSettings->GetActionMappingByName(ActionName, Mappings);
        if (Mappings.Num() > 0) return Mappings[0].Key;
    }
    return EKeys::Invalid;
}

// 4. 退出遊戲實作
void USettingsFunctionLibrary::QuitTheGame(const UObject* WorldContextObject)
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0);
    if (PC)
    {
        UKismetSystemLibrary::QuitGame(WorldContextObject, PC, EQuitPreference::Quit, true);
    }
}
void USettingsFunctionLibrary::ForceApplyInputSettings(const UObject* WorldContextObject)
{
    UInputSettings* InputSettings = UInputSettings::GetInputSettings();
    if (!InputSettings) return;

    // 1. 強制重載硬碟上的設定 (.ini)
    InputSettings->LoadConfig();

    if (WorldContextObject)
    {
        APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0);
        if (PC && PC->PlayerInput)
        {
            // 2. 徹底清空並重建按鍵映射快取[cite: 1]
            PC->PlayerInput->ForceRebuildingKeyMaps(true);

            // 3. 通知角色重新啟動輸入組件
            APawn* P = PC->GetPawn();
            if (P && !P->IsPendingKillPending())
            {
                P->PawnClientRestart();
            }
        }
    }
}
void USettingsFunctionLibrary::GetNutritionProgressData(float Current, float Target, float& OutProgressBarRatio, FString& OutPercentageText)
{
    // 防呆機制：如果顧客沒有該項需求 (分母為0)，或是數值異常
    if (Target <= 0.0f)
    {
        OutProgressBarRatio = 0.0f;
        OutPercentageText = TEXT("0%");
        return;
    }

    // 計算實際比例
    float Ratio = Current / Target;

    // 1. 進度條專用：ProgressBar 只能接受 0.0 到 1.0 的數值，用 Clamp 限制住
    OutProgressBarRatio = FMath::Clamp(Ratio, 0.0f, 1.0f);

    // 2. 文字專用：將比例轉換為整數百分比 (例如 0.756 -> 76%)
    // 即使超過 100%，文字依然可以顯示真實數值 (如 120%) 讓玩家知道超標了
    int32 PercentInt = FMath::RoundToInt(Ratio * 100.0f);
    OutPercentageText = FString::Printf(TEXT("%d%%"), PercentInt);
}