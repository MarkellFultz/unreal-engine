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
	// 🌟 追蹤點 1：確認 C++ 真的有被藍圖呼叫到！
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Yellow, TEXT("🟡 C++：成功觸發 AcceptIngredients！"));

	if (!Interactor)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Red, TEXT("🔴 C++ 退件：找不到玩家 (Interactor 為空)！"));
		return false;
	}

	if (SelectedItems.Num() == 0)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Red, TEXT("🔴 C++ 退件：你沒有選擇任何食材！"));
		return false;
	}

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
				FString ErrorMsg = FString::Printf(TEXT("❌ 拒絕烹飪：[%s] 還需要切片！請先去砧板處理。"), *ItemID.ToString());
				if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Red, ErrorMsg);

				AMyCharacter* MyChar = Cast<AMyCharacter>(Interactor);
				if (MyChar)
				{
					MyChar->SetUIInputMode(true);
					APlayerController* PC = Cast<APlayerController>(MyChar->GetController());
					if (PC)
					{
						PC->ResetIgnoreMoveInput();
						PC->ResetIgnoreLookInput();

						if (!this->ActorHasTag(FName("NoCamera")))
						{
							PC->SetViewTargetWithBlend(MyChar, 0.5f, EViewTargetBlendFunction::VTBlend_Cubic);
						}
					}
				}
				return false;
			}
		}
	}

	// ==========================================
	// 📦 扣除食材與開始料理
	// ==========================================
	UInventoryComponent* Inv = Interactor->FindComponentByClass<UInventoryComponent>();
	if (!Inv)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Red, TEXT("🔴 C++ 退件：玩家身上找不到背包組件！"));
		return false;
	}

	if (Inv->ConsumeIngredients(SelectedItems))
	{
		// 🌟 追蹤點 2：食材扣除成功
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Orange, TEXT("🟠 C++：食材扣除成功！準備開火..."));

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

				// 🌟 追蹤點 3：鏡頭判定
				if (!this->ActorHasTag(FName("NoCamera")))
				{
					if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Cyan, TEXT("🔵 C++：沒有標籤，我要轉鏡頭了！"));
					PC->SetViewTargetWithBlend(this, 0.5f, EViewTargetBlendFunction::VTBlend_Cubic);
				}
				else
				{
					if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Green, TEXT("🟢 C++：發現 NoCamera 標籤，不轉鏡頭！"));
				}
			}
		}

		QTECount = 0;
		SuccessfulQTECount = 0;
		TriggerRandomQTE();

		return true;
	}
	else
	{
		// 🌟 追蹤點 4：扣除失敗
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Red, TEXT("🔴 C++ 退件：扣除食材失敗！(可能背包裡根本沒這個食材)"));
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
	FName FinalOutputItem = TEXT("Ing_RuinedFood");
	bool bRecipeMatched = false;

	if (CookingRecipeDataTable && StoredIngredients.Num() > 0)
	{
		TArray<FCookingRecipeData*> AllRecipes;
		CookingRecipeDataTable->GetAllRows<FCookingRecipeData>(TEXT("CookingMatchContext"), AllRecipes);

		TArray<FName> SortedCurrentInput = StoredIngredients;
		SortedCurrentInput.Sort([](const FName& A, const FName& B) { return A.ToString() < B.ToString(); });

		for (FCookingRecipeData* Recipe : AllRecipes)
		{
			if (Recipe)
			{
				TArray<FName> SortedRecipeInput = Recipe->InputIngredients;
				SortedRecipeInput.Sort([](const FName& A, const FName& B) { return A.ToString() < B.ToString(); });

				if (SortedCurrentInput == SortedRecipeInput)
				{
					FinalOutputItem = Recipe->OutputItem;
					bRecipeMatched = true;
					break;
				}
			}
		}
	}

	if (CurrentInteractor)
	{
		AMyCharacter* MyChar = Cast<AMyCharacter>(CurrentInteractor);
		if (MyChar)
		{
			MyChar->SetUIInputMode(true);

			APlayerController* PC = Cast<APlayerController>(MyChar->GetController());
			if (PC)
			{
				PC->ResetIgnoreMoveInput();
				PC->ResetIgnoreLookInput();
				if (!this->ActorHasTag(FName("NoCamera")))
				{
					PC->SetViewTargetWithBlend(MyChar, 0.5f, EViewTargetBlendFunction::VTBlend_Cubic);
				}
			}

			UInventoryComponent* Inv = MyChar->FindComponentByClass<UInventoryComponent>();
			if (Inv)
			{
				Inv->AddIngredient(FinalOutputItem);

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

	OnCookingFinished(SuccessfulQTECount);

	for (UStaticMeshComponent* Comp : DisplayedIngredientMeshes)
	{
		if (Comp) Comp->DestroyComponent();
	}
	DisplayedIngredientMeshes.Empty();
	StoredIngredients.Empty();
}