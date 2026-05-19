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

bool ACookingToolActor::AcceptIngredients(AActor* Interactor, TArray<FName> SelectedItems)
{
	if (!Interactor || SelectedItems.Num() == 0) return false;

	// ==========================================
	// 🛑 安全檢測機制
	// ==========================================
	if (IngredientDataTable)
	{
		for (FName ItemID : SelectedItems)
		{
			FIngredientData* Data = IngredientDataTable->FindRow<FIngredientData>(ItemID, TEXT("CookingToolCheck"));
			if (Data && Data->IsSliceable)
			{
				// 1. 印出紅字警告
				FString ErrorMsg = FString::Printf(TEXT("❌ 拒絕烹飪：[%s] 還需要切片！請先去砧板處理。"), *ItemID.ToString());
				if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Red, ErrorMsg);

				// 2. 徹底解放玩家：把 UI 關掉，並解除所有的鎖定！
				AMyCharacter* MyChar = Cast<AMyCharacter>(Interactor);
				if (MyChar)
				{
					MyChar->SetUIInputMode(false); // 切回遊戲模式 (Game Only)

					APlayerController* PC = Cast<APlayerController>(MyChar->GetController());
					if (PC)
					{
						PC->ResetIgnoreMoveInput(); // 恢復鍵盤走路
						PC->ResetIgnoreLookInput(); // 恢復滑鼠轉頭

						// 鏡頭強制平滑飛回玩家身上
						PC->SetViewTargetWithBlend(MyChar, 0.5f, EViewTargetBlendFunction::VTBlend_Cubic);
					}
				}

				return false; // 回傳失敗給藍圖
			}
		}
	}

	// ==========================================
	// 📦 扣除食材與開始料理 (這裡完全保留你的功能)
	// ==========================================
	UInventoryComponent* Inv = Interactor->FindComponentByClass<UInventoryComponent>();
	if (Inv && Inv->ConsumeIngredients(SelectedItems))
	{
		StoredIngredients.Append(SelectedItems);
		CurrentInteractor = Interactor;

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
						NewMeshComp->AttachToComponent(MeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
						NewMeshComp->SetStaticMesh(LoadedMesh);
						NewMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
						NewMeshComp->SetGenerateOverlapEvents(false);

						float RandomX = FMath::RandRange(-15.0f, 15.0f);
						float RandomY = FMath::RandRange(-15.0f, 15.0f);
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
				PC->SetViewTargetWithBlend(this, 0.5f, EViewTargetBlendFunction::VTBlend_Cubic);
			}
		}

		QTECount = 0;
		SuccessfulQTECount = 0;
		TriggerRandomQTE();

		// 料理成功啟動！回傳 true，讓藍圖放心關閉 UI！
		return true;
	}

	return false;
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
	// 預設產出為「黑暗料理 / 失敗作」
	FName FinalOutputItem = TEXT("Ing_RuinedFood");
	bool bRecipeMatched = false;

	// ========================================================
	// 🌲 新增：核心配方比對邏輯（不看順序，只看材料有沒有齊）
	// ========================================================
	if (CookingRecipeDataTable && StoredIngredients.Num() > 0)
	{
		// 1. 撈出配方表裡所有的橫列 (Rows)
		TArray<FCookingRecipeData*> AllRecipes;
		CookingRecipeDataTable->GetAllRows<FCookingRecipeData>(TEXT("CookingMatchContext"), AllRecipes);

		// 2. 將玩家目前丟進鍋子裡的食材進行「字母排序」
		TArray<FName> SortedCurrentInput = StoredIngredients;
		SortedCurrentInput.Sort([](const FName& A, const FName& B) { return A.ToString() < B.ToString(); });

		// 3. 逐一比對每一張配方
		for (FCookingRecipeData* Recipe : AllRecipes)
		{
			if (Recipe)
			{
				// 將配方要求的食材也進行「字母排序」
				TArray<FName> SortedRecipeInput = Recipe->InputIngredients;
				SortedRecipeInput.Sort([](const FName& A, const FName& B) { return A.ToString() < B.ToString(); });

				// 4. 精準對撞！如果排序後的陣列完全一模一樣，代表材料全中！
				if (SortedCurrentInput == SortedRecipeInput)
				{
					FinalOutputItem = Recipe->OutputItem; // 撈出對應的成功產物
					bRecipeMatched = true;
					break; // 找到了，直接跳出迴圈
				}
			}
		}
	}
	// ========================================================

	// 5. 結算：把做好的東西（成功料理或黑暗料理）塞進玩家背包
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

			// 塞入背包
			UInventoryComponent* Inv = MyChar->FindComponentByClass<UInventoryComponent>();
			if (Inv)
			{
				Inv->AddIngredient(FinalOutputItem);

				// 螢幕跳出華麗的提示
				if (GEngine)
				{
					FColor MsgColor = bRecipeMatched ? FColor::Green : FColor::Red;
					FString Msg = bRecipeMatched ?
						FString::Printf(TEXT("🍳 烹飪成功！獲得了：[%s]"), *FinalOutputItem.ToString()) :
						FString::Printf(TEXT("💥 配方出錯！做出了黑暗料理... 獲得：[%s]"), *FinalOutputItem.ToString());

					GEngine->AddOnScreenDebugMessage(-1, 6.f, MsgColor, Msg);
				}
			}
		}
	}

	// 觸發藍圖的結算畫面 UI 事件
	OnCookingFinished(SuccessfulQTECount);

	// 清除鍋子模型與暫存資料
	for (UStaticMeshComponent* Comp : DisplayedIngredientMeshes)
	{
		if (Comp) Comp->DestroyComponent();
	}
	DisplayedIngredientMeshes.Empty();
	StoredIngredients.Empty();
}