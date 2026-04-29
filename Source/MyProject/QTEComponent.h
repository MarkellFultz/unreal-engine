#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "QTEComponent.generated.h"

// 定義 QTE 結果的委派，方便通知 UI 或其他系統
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQTECompleted, bool, bWasSuccessful);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MYPROJECT_API UQTEComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UQTEComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// 開始 QTE
	UFUNCTION(BlueprintCallable, Category = "QTE")
	void StartQTE(float InSuccessStart, float InSuccessEnd, float InSpeed);

	// 停止 QTE
	UFUNCTION(BlueprintCallable, Category = "QTE")
	void StopQTE();

	UFUNCTION(BlueprintCallable, Category = "QTE")
	bool AttemptHit();

	// 檢查 QTE 是否正在運行
	UFUNCTION(BlueprintPure, Category = "QTE")
	bool IsQTEActive() const { return bIsQTERunning; }

	// 提供給 UI 讀取的角度
	UFUNCTION(BlueprintPure, Category = "QTE")
	float GetPointerAngle() const { return CurrentPointerAngle; }

	UPROPERTY(BlueprintAssignable, Category = "QTE")
	FOnQTECompleted OnQTECompleted;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QTE")
	float SuccessZoneStart = 45.0f; // 黃金區間起點 (角度)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QTE")
	float SuccessZoneEnd = 90.0f;   // 黃金區間終點 (角度)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QTE")
	float PointerSpeed = 150.0f;    // 指針移動速度

	UPROPERTY(BlueprintReadOnly, Category = "QTE")
	float CurrentPointerAngle = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "QTE")
	bool bIsQTERunning = false;     // 改名避免與父類別 bIsActive 衝突

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QTE")
	float TimeReward = 5.0f;        // 成功後的獎勵秒數
};