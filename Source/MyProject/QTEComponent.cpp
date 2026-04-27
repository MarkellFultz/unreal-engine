#include "QTEComponent.h"

UQTEComponent::UQTEComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	bIsQTERunning = false;
}

void UQTEComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsQTERunning)
	{
		CurrentPointerAngle += PointerSpeed * DeltaTime;
		if (CurrentPointerAngle >= 360.0f)
		{
			CurrentPointerAngle -= 360.0f;
		}
	}
}

void UQTEComponent::StartQTE(float InSuccessStart, float InSuccessEnd, float InSpeed)
{
	SuccessZoneStart = InSuccessStart;
	SuccessZoneEnd = InSuccessEnd;
	PointerSpeed = InSpeed;
	CurrentPointerAngle = 0.0f;
	bIsQTERunning = true;
}

void UQTEComponent::StopQTE()
{
	bIsQTERunning = false;
}

bool UQTEComponent::AttemptHit()
{
	if (!bIsQTERunning) return false;

	// 判定點擊瞬間指針是否位於「黃金區間」
	bool bSuccess = (CurrentPointerAngle >= SuccessZoneStart && CurrentPointerAngle <= SuccessZoneEnd);

	OnQTECompleted.Broadcast(bSuccess);

	if (bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("QTE Success! Time Reward Added."));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("QTE Failed!"));
	}

	StopQTE();
	return bSuccess;
}