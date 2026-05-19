#include "CookingToolActor.h"
#include "Components/StaticMeshComponent.h" 
#include "Engine/World.h"                   
#include "TimerManager.h"                   
#include "Kismet/KismetMathLibrary.h"       
#include "Kismet/GameplayStatics.h"         
#include "../InventoryComponent.h"
#include "../QTEComponent.h" 
#include "../MyCharacter.h"

ACookingToolActor::ACookingToolActor()
{
	PrimaryActorTick.bCanEverTick = false;
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	// 初始化 QTE 組件
	QTEComp = CreateDefaultSubobject<UQTEComponent>(TEXT("QTEComp"));

	// 建立俯視相機
	TopDownCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCamera->SetupAttachment(RootComponent);

	// ====== ✨【調整參數】：預設高度直接從 150 挪高到 250，視野更廣 ======
	TopDownCamera->SetRelativeLocation(FVector(0.0f, 0.0f, 250.0f));
	TopDownCamera->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));
}

void ACookingToolActor::Interact(AActor* Interactor)
{
	OnOpenCookingUI(Interactor);
}

FText ACookingToolActor::GetInteractPrompt() const
{
	return FText::FromString(TEXT("[E] 放入食材"));
}

void ACookingToolActor::AcceptIngredients(AActor* Interactor, TArray<FName> SelectedItems)
{
	if (!Interactor || SelectedItems.Num() == 0) return;

	// ==========================================
	// 🛑【新功能】：安全檢測機制
	// 只有 IsSliceable == false 的食材才能進行烹飪互動！
	// ==========================================
	if (IngredientDataTable)
	{
		for (FName ItemID : SelectedItems)
		{
			FIngredientData* Data = IngredientDataTable->FindRow<FIngredientData>(ItemID, TEXT("CookingToolCheck"));
			if (Data && Data->IsSliceable)
			{
				// 只要發現有任何一個需要切的食材，立刻無情拒絕！
				FString ErrorMsg = FString::Printf(TEXT("❌ 拒絕烹飪：[%s] 還需要切片！請先去砧板處理。"), *ItemID.ToString());
				if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Red, ErrorMsg);

				AMyCharacter* MyChar = Cast<AMyCharacter>(Interactor);
				if (MyChar) MyChar->SetUIInputMode(false); // 關閉 UI 模式恢復玩家控制
				return; // 直接中斷整個函數，不扣除也不開伙
			}
		}
	}
	else
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Red, TEXT("❌ 嚴重錯誤：廚具未綁定 IngredientDataTable，無法進行安全驗證！"));
		return;
	}
	// ==========================================

	UInventoryComponent* Inv = Interactor->FindComponentByClass<UInventoryComponent>();
	if (Inv && Inv->ConsumeIngredients(SelectedItems))
	{
		StoredIngredients.Append(SelectedItems);
		CurrentInteractor = Interactor;

		// 在鍋具上動態生成並固定食材模型
		if (IngredientDataTable)
		{
			for (int32 Index = 0; Index < SelectedItems.Num(); ++Index)
			{
				FName ItemID = SelectedItems[Index];
				FIngredientData* Data = IngredientDataTable->FindRow<FIngredientData>(ItemID, TEXT("CookingTool"));

				if (Data && !Data->IngredientMesh.IsNull())
				{
					UStaticMesh* LoadedMesh = Data->IngredientMesh.LoadSynchronous();
					if (LoadedMesh)
					{
						UStaticMeshComponent* NewMeshComp = NewObject<UStaticMeshComponent>(this, NAME_None);
						NewMeshComp->RegisterComponent();

						// ✨【附加規則修正】：使用 SnapToTarget 強制將座標歸零到鍋子中心
						NewMeshComp->AttachToComponent(MeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
						NewMeshComp->SetStaticMesh(LoadedMesh);

						NewMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
						NewMeshComp->SetGenerateOverlapEvents(false);

						// 食材隨機散落堆疊
						float RandomX = FMath::RandRange(-15.0f, 15.0f);
						float RandomY = FMath::RandRange(-15.0f, 15.0f);
						// ✨【基礎高度修正】：調高到 25.0f 確保模型不會陷進鍋底
						float ZOffset = 25.0f + (Index * 5.0f);

						NewMeshComp->SetRelativeLocation(FVector(RandomX, RandomY, ZOffset));
						NewMeshComp->SetRelativeRotation(FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f));

						DisplayedIngredientMeshes.Add(NewMeshComp);
					}
				}
			}
		}

		AMyCharacter* MyChar = Cast<AMyCharacter>(Interactor);
		if (MyChar)
		{
			MyChar->SetUIInputMode(false);

			APlayerController* PC = Cast<APlayerController>(MyChar->GetController());
			if (PC)
			{
				PC->SetIgnoreMoveInput(true);
				PC->SetIgnoreLookInput(true);

				// 平滑融合切換至廚具上方的相機 (0.5秒)
				PC->SetViewTargetWithBlend(this, 0.5f, EViewTargetBlendFunction::VTBlend_Cubic);
			}
		}

		QTECount = 0;
		SuccessfulQTECount = 0;
		TriggerRandomQTE();
	}
}

void ACookingToolActor::TriggerRandomQTE()
{
	if (QTECount >= 3)
	{
		FinishCooking();
		return;
	}

	float RandomDelay = UKismetMathLibrary::RandomFloatInRange(1.0f, 3.0f);

	GetWorld()->GetTimerManager().SetTimer(
		QTE_TimerHandle,
		[this]()
		{
			if (QTEComp)
			{
				float RandomStart = UKismetMathLibrary::RandomFloatInRange(0.0f, 300.0f);
				float RandomEnd = RandomStart + 60.0f;

				QTEComp->StartQTE(RandomStart, RandomEnd, 150.0f);

				OnQTEStarted();
				QTECount++;
			}
		},
		RandomDelay,
		false
	);
}

void ACookingToolActor::FinishCooking()
{
	if (CurrentInteractor)
	{
		AMyCharacter* MyChar = Cast<AMyCharacter>(CurrentInteractor);
		if (MyChar)
		{
			MyChar->SetUIInputMode(true);

			// 視角復原
			APlayerController* PC = Cast<APlayerController>(MyChar->GetController());
			if (PC)
			{
				PC->ResetIgnoreMoveInput();
				PC->ResetIgnoreLookInput();
				PC->SetViewTargetWithBlend(MyChar, 0.5f, EViewTargetBlendFunction::VTBlend_Cubic);
			}
		}

		// ==========================================
		// 🎉【新功能】：產出煮過後的資料 (_Cooked) 送回玩家背包！
		// ==========================================
		UInventoryComponent* Inv = CurrentInteractor->FindComponentByClass<UInventoryComponent>();
		if (Inv)
		{
			for (FName ItemID : StoredIngredients)
			{
				// 把原本的 ID 加上 _Cooked 後綴
				FString CookedItemName = ItemID.ToString() + TEXT("_Cooked");
				Inv->AddIngredient(FName(*CookedItemName));

				if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("🍳 烹飪完成：[%s] 已轉為 [%s] 存入背包！"), *ItemID.ToString(), *CookedItemName));
			}
		}
		// ==========================================
	}

	OnCookingFinished(SuccessfulQTECount);
	UE_LOG(LogTemp, Warning, TEXT("✅ 料理結束，視角已回歸玩家，等待點擊結算畫面"));

	// 清除鍋子裡的生食材模型
	for (UStaticMeshComponent* Comp : DisplayedIngredientMeshes)
	{
		if (Comp)
		{
			Comp->DestroyComponent();
		}
	}
	DisplayedIngredientMeshes.Empty();
	StoredIngredients.Empty();
}