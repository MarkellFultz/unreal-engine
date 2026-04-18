#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TutorialManager.generated.h"

// 定義教學的所有步驟 (對應你的 User Story 需求)
UENUM(BlueprintType)
enum class ETutorialStep : uint8
{
	Welcome         UMETA(DisplayName = "Welcome"),
	Move            UMETA(DisplayName = "Move"),
	GrabIngredient  UMETA(DisplayName = "Grab Ingredient"),
	Chop            UMETA(DisplayName = "Chop"),
	Cook            UMETA(DisplayName = "Cook"),
	Serve           UMETA(DisplayName = "Serve"),
	Completed       UMETA(DisplayName = "Completed")
};

// 委派：當教學步驟改變時，通知藍圖 UI
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTutorialStepChanged, ETutorialStep, NewStep);

UCLASS()
class MYPROJECT_API ATutorialManager : public AActor
{
	GENERATED_BODY()
	
public:	
	ATutorialManager();

protected:
	virtual void BeginPlay() override;

public:	
	UPROPERTY(BlueprintAssignable, Category = "Tutorial")
	FOnTutorialStepChanged OnStepChanged;

	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void CompleteStep(ETutorialStep StepCompleted);

	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void StartSpecificTutorial(ETutorialStep StepToStart);

	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void CheckAndStartTutorial();

	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void FinishTutorial();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tutorial")
	ETutorialStep CurrentStep;

	FString SaveSlotName = TEXT("PlayerSaveSlot");
};