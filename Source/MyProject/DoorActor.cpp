#include "DoorActor.h"
#include "MyCharacter.h" // 記得引入你的玩家類別
#include "Kismet/GameplayStatics.h"

ADoorActor::ADoorActor()
{
	PrimaryActorTick.bCanEverTick = false;
	DoorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));
	RootComponent = DoorMesh;
}

void ADoorActor::Interact(AActor* Interactor)
{
	AMyCharacter* MyChar = Cast<AMyCharacter>(Interactor);
	if (MyChar)
	{
		if (MyChar->GetCanEnterCookingStage())
		{
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("✅ 驗證通過，進入料理階段！"));
			// 呼叫藍圖去換關卡 (因為把地圖名字寫死在 C++ 是不好的設計，所以換關卡交給藍圖)
			OnProceedToCookingStage();
		}
		else
		{
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("❌ 營養素尚未達標，不能開門！"));
		}
	}
}

FText ADoorActor::GetInteractPrompt() const
{
	// 自動根據玩家的狀態，切換提示文字
	AMyCharacter* MyChar = Cast<AMyCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	if (MyChar)
	{
		if (MyChar->GetCanEnterCookingStage())
		{
			return FText::FromString(TEXT("[E] 進入料理階段"));
		}
		else
		{
			return FText::FromString(TEXT("食材未達標，無法進入"));
		}
	}
	return FText::FromString(TEXT("大門"));
}