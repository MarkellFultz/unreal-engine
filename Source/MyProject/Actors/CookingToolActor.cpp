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
	// 防呆：如果人死了或是一樣食材都沒選，直接退貨
	if (!Interactor || SelectedItems.Num() == 0) return;

	// ==========================================
	// 🛑【功能保留】：安全檢測機制（支援複數食材）
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

				// 這裡保持你的設定：將控制權還給玩家
				AMyCharacter* MyChar = Cast<AMyCharacter>(Interactor);
				if (MyChar)
				{
					// 💡 夥伴提示：這裡保持原樣，但在藍圖 UI 的按鈕被點擊後，
					// 建議也要做一個「關閉 UI」的動作，玩家畫面才不會卡住喔！
					MyChar->SetUIInputMode(false);
				}
				return; // 直接中斷整個函數，鍋子不開伙、包包不扣東西
			}
		}
	}
	else
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Red, TEXT("❌ 嚴重錯誤：廚具未綁定 IngredientDataTable，無法進行安全驗證！"));
		return;
	}

	// ==========================================
	// 📦【核心改動】：一次性扣除與倒入包包（完美對撞）
	// ==========================================
	UInventoryComponent* Inv = Interactor->FindComponentByClass<UInventoryComponent>();

	// 呼叫你的背包系統，一次扣除這陣列裡所有的食材（例如：[番茄糊, 豆腐, 青花菜]）
	if (Inv && Inv->ConsumeIngredients(SelectedItems))
	{
		// 1. 把購物車所有的食材 ID，追加（Append）到鍋子的儲存陣列中，等烹飪結束拿去跟配方表對撞！
		StoredIngredients.Append(SelectedItems);
		CurrentInteractor = Interactor;

		// 2. 【功能保留】：在鍋具上依序把複數食材的模型通通生出來！
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

						// 使用 SnapToTarget 強制將座標歸零到鍋子中心
						NewMeshComp->AttachToComponent(MeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
						NewMeshComp->SetStaticMesh(LoadedMesh);

						NewMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
						NewMeshComp->SetGenerateOverlapEvents(false);

						// 【功能保留】：食材隨機散落堆疊算法
						float RandomX = FMath::RandRange(-15.0f, 15.0f);
						float RandomY = FMath::RandRange(-15.0f, 15.0f);
						// 基於 Index 進行高度疊加（Index 0 在最下面，Index 2 疊在上面）
						float ZOffset = 25.0f + (Index * 5.0f);

						NewMeshComp->SetRelativeLocation(FVector(RandomX, RandomY, ZOffset));
						NewMeshComp->SetRelativeRotation(FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f));

						// 存進陣列，FinishCooking 的時候會迴圈刪除它們
						DisplayedIngredientMeshes.Add(NewMeshComp);
					}
				}
			}
		}

		// 3. 【功能保留】：鎖定玩家，融合同步運鏡
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

		// 4. 【功能保留】：啟動隨機 QTE 烹飪機制！
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