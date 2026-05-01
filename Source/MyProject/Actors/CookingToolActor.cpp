#include "CookingToolActor.h"
#include "Components/StaticMeshComponent.h" // 解決 StaticMeshComponent 紅線
#include "Engine/World.h"                   // 解決 GetWorld() 紅線
#include "TimerManager.h"                   // 解決 Timer 相關紅線
#include "Kismet/KismetMathLibrary.h"       // 解決隨機數紅線
#include "Kismet/GameplayStatics.h"         // 解決教學管理員紅線
#include "../InventoryComponent.h"
#include "../QTEComponent.h" 
#include "../MyCharacter.h"
#include "../TutorialManager.h"

// 下面接著你的 ACookingToolActor::ACookingToolActor() ...

ACookingToolActor::ACookingToolActor()
{
	PrimaryActorTick.bCanEverTick = false;
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	// 初始化 QTE 組件
	QTEComp = CreateDefaultSubobject<UQTEComponent>(TEXT("QTEComp"));
}

void ACookingToolActor::Interact(AActor* Interactor)
{
	OnOpenCookingUI(Interactor);
}

FText ACookingToolActor::GetInteractPrompt() const
{
	return FText::FromString(TEXT("[E] 放入食材"));
}

// ⚠️ 確保整個檔案只有這一個 AcceptIngredients
void ACookingToolActor::AcceptIngredients(AActor* Interactor, TArray<FName> SelectedItems)
{
	if (Interactor && SelectedItems.Num() > 0)
	{
		UInventoryComponent* Inv = Interactor->FindComponentByClass<UInventoryComponent>();

		if (Inv && Inv->ConsumeIngredients(SelectedItems))
		{
			StoredIngredients.Append(SelectedItems);

			// 記住玩家是誰
			CurrentInteractor = Interactor;

			// 鎖定玩家移動
			AMyCharacter* MyChar = Cast<AMyCharacter>(Interactor);
			if (MyChar)
			{
				MyChar->SetUIInputMode(false); // 收回滑鼠

				APlayerController* PC = Cast<APlayerController>(MyChar->GetController());
				if (PC)
				{
					PC->SetIgnoreMoveInput(true); // 鎖定移動
					PC->SetIgnoreLookInput(true); // 👈 新增這行：鎖定視角旋轉，防止射線歪掉！
				}
			}

			// 重置計數器並開始第一次隨機觸發
			QTECount = 0;
			SuccessfulQTECount = 0; // 👈 新增這行：每次煮飯前把成功次數歸零
			TriggerRandomQTE();
		}
	}
}

void ACookingToolActor::TriggerRandomQTE()
{
	if (QTECount >= 3)
	{
		// 已經觸發過 3 次了，結束烹飪
		FinishCooking();
		return;
	}

	// 產生 1.0 ~ 3.0 秒之間的隨機延遲
	float RandomDelay = UKismetMathLibrary::RandomFloatInRange(1.0f, 3.0f);

	// 設定計時器，時間到就執行 Lambda 函式來啟動 QTE
	GetWorld()->GetTimerManager().SetTimer(
		QTE_TimerHandle,
		[this]()
		{
			if (QTEComp)
			{
				// 👇 1. 產生 0 到 300 度之間的隨機起點
				float RandomStart = UKismetMathLibrary::RandomFloatInRange(0.0f, 300.0f);

				// 👇 2. 設定黃金區間的範圍為 60 度 (你可以自己調整難度)
				float RandomEnd = RandomStart + 60.0f;

				// 👇 3. 把隨機數傳進去！
				QTEComp->StartQTE(RandomStart, RandomEnd, 150.0f);

				OnQTEStarted();
				QTECount++; // 次數 +1
			}
		},
		RandomDelay,
		false
	);
}

void ACookingToolActor::FinishCooking()
{
	// 呼叫藍圖事件，傳出成功次數，讓鍋子去顯示結算畫面！
	OnCookingFinished(SuccessfulQTECount);
	if (CurrentInteractor)
	{
		AMyCharacter* MyChar = Cast<AMyCharacter>(CurrentInteractor);
		if (MyChar)
		{
			APlayerController* PC = Cast<APlayerController>(MyChar->GetController());
			if (PC)
			{
				// 解鎖移動與視角
				PC->SetIgnoreMoveInput(false);
				PC->SetIgnoreLookInput(false);
			}

			// 確保回到遊戲模式 (關閉鼠標)
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("烹飪完成！"));
}