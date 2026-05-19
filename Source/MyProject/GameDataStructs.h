#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"
#include "Engine/StaticMesh.h"
#include "GameDataStructs.generated.h"

// ==========================================
// ==========================================
USTRUCT(BlueprintType)
struct FChapterData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chapter")
    FText ChapterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chapter", meta = (MultiLine = true))
    FString Description; // 
};

// ==========================================
// 2.  (Stage Data)
// ==========================================
USTRUCT(BlueprintType)
struct FStageData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage")
    FText StageName; // 

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage")
    FName ChapterId; // 

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage")
    TArray<FName> CustomerPool; // 

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage")
    FName RequiredPreviousStageId; // 
};

// ==========================================
// 3.  (Customer Profile Data)
// ==========================================
USTRUCT(BlueprintType)
struct FCustomerProfileData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customer")
    FText CustomerName; // 

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customer", meta = (MultiLine = true))
    FString Dialogue_Order; // 

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customer|NutritionTarget")
    float TargetCalories; // 

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customer|NutritionTarget")
    float TargetProtein; //  (g)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customer|NutritionTarget")
    float TargetCarbs; //  (g)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customer|NutritionTarget")
    float TargetFat; //  (g)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customer|Restrictions")
    FString HealthTags; //  (例: Diabetes)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customer|Restrictions")
    FString AllergenList; // 

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customer|Restrictions")
    FString DislikeList; // 

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customer|Solution")
    TArray<FName> ExpectedRecipeIds; //  ID
};

// ==========================================
// 4.  (Recipe Data)
// ==========================================
USTRUCT(BlueprintType)
struct FRecipeData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
    FText RecipeName; // 

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
    TArray<FName> IngredientIds; //  ID

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe")
    TArray<FString> RecipeSteps; //  (UI )

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe|Assets")
    TSoftObjectPtr<UTexture2D> RecipeImage; //  ( Soft Pointer )

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe|Evaluation")
    TArray<FString> EvaluationCriteria; // 

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recipe|Evaluation")
    float TargetCookTime; // 
};

// ==========================================
// 5.  (Ingredient Data)
// ==========================================
USTRUCT(BlueprintType)
struct FIngredientData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ingredient")
    FText DisplayName; // 

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ingredient|Nutrition")
    float Calories; // 

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ingredient|Nutrition")
    float Protein; //  (g)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ingredient|Nutrition")
    float Carbs; //  (g)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ingredient|Nutrition")
    float Fat; //  (g)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ingredient|Tags")
    FString AllergenTags; // 

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ingredient|Assets")
    TSoftObjectPtr<UTexture2D> Icon; // UI  (Soft Pointer)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ingredient|Assets")
    TSoftObjectPtr<UStaticMesh> IngredientMesh; // 3D  (Soft Pointer)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ingredient|Gameplay")
    bool IsSliceable; // 

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ingredient|Gameplay")
    float TotalCookTime; // 

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ingredient|Gameplay")
    float ColorChangeRatio; //  (0~1)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ingredient|Gameplay")
    float BurnTimeRatio; //  (.2)
};
USTRUCT(BlueprintType)
struct FCookingRecipeData : public FTableRowBase
{
    GENERATED_BODY()

    // 輸入孔：這道菜需要哪些食材陣列（例如：[番茄糊, 切片豆腐, 切片青花菜]）
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe")
    TArray<FName> InputIngredients;

    // 輸出孔：合成成功的產物 ID（例如：Dish_R005 或半成品 Ing_Tomato_Paste）
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Recipe")
    FName OutputItem;
};