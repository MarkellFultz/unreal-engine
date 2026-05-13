#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SettingsFunctionLibrary.generated.h"

/**
 * 遊戲設定功能庫
 * 提供視窗、音量、按鍵綁定等全域設定功能
 */
UCLASS()
class MYPROJECT_API USettingsFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // ==========================================
    // 1. 視窗設定 (Window Settings)
    // ==========================================

    /** 設置並套用解析度與全螢幕狀態 */
    UFUNCTION(BlueprintCallable, Category = "Game Settings")
    static void SetAndApplyWindowSettings(int32 Width, int32 Height, bool bIsFullscreen);

    /** 透過索引設置視窗模式 (0: 視窗, 1: 無邊框全螢幕, 2: 全螢幕) */
    UFUNCTION(BlueprintCallable, Category = "Game Settings")
    static void SetWindowModeByIndex(int32 ModeIndex);

    /** 取得目前視窗模式的索引 */
    UFUNCTION(BlueprintPure, Category = "Game Settings")
    static int32 GetCurrentWindowModeIndex();

    // ==========================================
    // 2. 音量設定 (Volume Settings)
    // ==========================================

    /** 設置特定 SoundClass 的音量 */
    UFUNCTION(BlueprintCallable, Category = "Game Settings")
    static void SetGameVolume(class USoundMix* Mix, class USoundClass* SoundClass, float Volume);

    // ==========================================
    // 3. 按鍵綁定 (Input Rebinding)
    // ==========================================

    /** 重新綁定單次按下的動作 (Action Mapping) */
    UFUNCTION(BlueprintCallable, Category = "Game Settings|Input", meta = (WorldContext = "WorldContextObject"))
    static void RebindActionMapping(const UObject* WorldContextObject, FName ActionName, FKey NewKey);

    /** 重新綁定持續性的軸向動作 (Axis Mapping) - 顯現引腳版本 */
    UFUNCTION(BlueprintCallable, Category = "Game Settings|Input")
    static void RebindAxisMapping(UObject* WorldContextObject, FName AxisName, FKey NewKey, float Scale = 1.0f);

    /** 讀取目前 Action 指定的按鍵 */
    UFUNCTION(BlueprintPure, Category = "Game Settings|Input")
    static FKey GetCurrentActionKey(FName ActionName);

    // ==========================================
    // 4. 系統功能 (System)
    // ==========================================

    /** 安全退出遊戲 */
    UFUNCTION(BlueprintCallable, Category = "Game Settings", meta = (WorldContext = "WorldContextObject"))
    static void QuitTheGame(const UObject* WorldContextObject);
    UFUNCTION(BlueprintCallable, Category = "Game Settings|Input")
    static void ForceApplyInputSettings(const UObject* WorldContextObject);
    // ==========================================
    // 5. 遊戲邏輯與 UI 輔助
    // ==========================================

    /** * 計算營養素進度條比例與顯示文字
     * @param Current 當前備料區的數值 (分子)
     * @param Target 顧客的需求數值 (分母)
     * @param OutProgressBarRatio 輸出給 ProgressBar 的數值 (自動限制在 0.0 ~ 1.0)
     * @param OutPercentageText 輸出給 TextBlock 的文字 (例如 "75%")
     */
    UFUNCTION(BlueprintPure, Category = "Game Logic|UI")
    static void GetNutritionProgressData(float Current, float Target, float& OutProgressBarRatio, FString& OutPercentageText);
};