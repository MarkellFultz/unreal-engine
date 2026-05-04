#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../InteractInterface.h"
#include "../GameDataStructs.h"
#include "Camera/CameraComponent.h"
#include "ProceduralMeshComponent.h"
#include "CuttingBoardActor.generated.h"

UENUM(BlueprintType)
enum class ECuttingState : uint8
{
    Idle            UMETA(DisplayName = "閒置"),
    Aiming          UMETA(DisplayName = "瞄準切割中"),
    ReadyToCollect  UMETA(DisplayName = "待拾取")
};

UCLASS()
class MYPROJECT_API ACuttingBoardActor : public AActor, public IInteractInterface
{
    GENERATED_BODY()
    
public:	
    ACuttingBoardActor();
    virtual void Tick(float DeltaTime) override;

    // --- 介面實作 ---
    virtual void Interact(AActor* Interactor) override;
    virtual FText GetInteractPrompt() const override;

    // --- 核心功能 ---
    // 1. UI 呼叫此函數放置食材
    UFUNCTION(BlueprintCallable, Category = "Cutting")
    void PlaceIngredient(FName IngredientID, AActor* Interactor);

    // 2. 玩家按下空白鍵時呼叫此函數進行切片
    UFUNCTION(BlueprintCallable, Category = "Cutting")
    void ExecuteCut();

    // --- 藍圖事件 ---
    UFUNCTION(BlueprintImplementableEvent, Category = "Cutting|Events")
    void OnOpenInventoryUI(AActor* Interactor);
    UFUNCTION(BlueprintCallable, Category = "Cutting")
    void FinishCutting();

protected:
    virtual void BeginPlay() override;

    // 狀態與資料
    ECuttingState CurrentState = ECuttingState::Idle;
    FName CurrentIngredientID;
    AActor* CurrentInteractor;
    int32 CutCount = 0; // 記錄切了幾刀

    // --- 元件 ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UStaticMeshComponent* BoardMesh;

    // 隱藏的 StaticMesh，用來載入資料表中的模型，以轉換成 ProcMesh
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* DummyStaticMeshComp;

    // 動態生成的切片組件清單
    UPROPERTY()
    TArray<UProceduralMeshComponent*> ProcMeshes;

    // 瞄準線平面
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UStaticMeshComponent* AimingPlane;

    UPROPERTY(EditAnywhere, Category = "Data")
    UDataTable* IngredientDataTable;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cutting|Settings")
    float SurfaceHeightOffset = 5.0f;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cutting|Camera")
    UCameraComponent* TopDownCamera;
private:
    void CollectIngredients();
    void UpdateAimingLine();
};