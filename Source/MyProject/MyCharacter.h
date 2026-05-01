#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ProceduralMeshComponent.h"
#include "KismetProceduralMeshLibrary.h"
#include "MyCharacter.generated.h"

class UCameraComponent;
class UAnimMontage;
class UDataTable;
class UUserWidget;
class UInventoryComponent;

UCLASS()
class MYPROJECT_API AMyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AMyCharacter();

protected:
	virtual void BeginPlay() override;

	// ---  ---
	void MoveForward(float Value);
	void MoveRight(float Value);
	void OnInteract();
	void OnQTEPressed();
	void SliceObject(UProceduralMeshComponent* TargetMesh, FVector PlanePosition, FVector PlaneNormal);
	void OnQTEPressedAxis(float Value);
	
	/** 取得目前準心對準物件的互動提示文字，供 Blueprint Binding 使用 (User Story 3.1-1) */
	class AActor* GetCurrentInteractable() const;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Jump() override;

	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	UInventoryComponent* InventoryComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slicing")
	float TraceDistance = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slicing")
	UAnimMontage* CutMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	UDataTable* IngredientDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> InventoryWidgetClass;

	UPROPERTY()
	UUserWidget* InventoryWidgetInstance;

	/**
	 * 每幀由 HUD Text Binding 呼叫，回傳目前準心物件的提示文字。
	 * BlueprintPure = 不需執行線，Blueprint 可直接拉線到 Return Node。
	 */
	UFUNCTION(BlueprintPure, Category = "UI")
	FText GetCurrentInteractPromptText() const;

	UPROPERTY(BlueprintReadWrite, Category = "Tutorial")
	bool bHasCompletedMoveTutorial = false;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void ToggleInventory();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetUIInputMode(bool bIsUIMode);
};