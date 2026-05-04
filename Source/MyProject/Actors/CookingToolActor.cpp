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
			CurrentInteractor = Interactor;

			AMyCharacter* MyChar = Cast<AMyCharacter>(Interactor);
			if (MyChar)
			{
				// 1. 關閉 UI 模式 (這會隱藏游標，但同時也會短暫解鎖玩家的移動限制)
				MyChar->SetUIInputMode(false);

				// 2. 獲取控制器，進行 QTE 專屬的鎖定
				APlayerController* PC = Cast<APlayerController>(MyChar->GetController());
				if (PC)
				{
					// 鎖死移動與視角轉動！
					PC->SetIgnoreMoveInput(true);
					PC->SetIgnoreLookInput(true);

					// [新增體驗升級]：計算玩家與鍋子之間的向量，強制將鏡頭對準鍋子
					FVector DirectionToPot = GetActorLocation() - MyChar->GetActorLocation();
					DirectionToPot.Z = 0.0f; // 保持視角水平，避免玩家突然低頭或抬頭
					FRotator TargetRotation = DirectionToPot.Rotation();

					// 同時轉動玩家控制器與角色實體
					PC->SetControlRotation(TargetRotation);
					MyChar->SetActorRotation(TargetRotation);
				}
			}

			QTECount = 0;
			SuccessfulQTECount = 0;
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
	// 確保有互動者可以設定狀態
	if (CurrentInteractor)
	{
		AMyCharacter* MyChar = Cast<AMyCharacter>(CurrentInteractor);
		if (MyChar)
		{
			// [重點修改] 料理結束後，進入 UI 模式！
			// 這會自動顯示滑鼠游標，並且因為我們之前寫好的邏輯，玩家依然會被鎖定無法移動。
			MyChar->SetUIInputMode(true);
		}
	}

	// 執行藍圖的料理結束事件 (這會觸發你的藍圖去生成結算畫面 UI)
	OnCookingFinished(SuccessfulQTECount);

	UE_LOG(LogTemp, Warning, TEXT("料理結束，已顯示滑鼠，等待玩家點擊結算畫面"));
}