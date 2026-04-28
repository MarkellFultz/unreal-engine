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

	void OnInteract();
	void MoveForward(float Value);
	void MoveRight(float Value);

	void SliceObject(UProceduralMeshComponent* TargetMesh, FVector PlanePosition, FVector PlaneNormal);

	// 👇 新增 QTE 與互動抓取函式宣告
	void OnQTEPressed();
	class AActor* GetCurrentInteractable();

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

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void ToggleInventory();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetUIInputMode(bool bIsUIMode);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class UUserWidget> InventoryWidgetClass;

	UPROPERTY()
	class UUserWidget* InventoryWidgetInstance;

private:
	bool bHasCompletedMoveTutorial = false;
	bool bHasCompletedInventoryTutorial = false;
};