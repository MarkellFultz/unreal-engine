#include "TutorialManager.h"
#include "MySaveGame.h"
#include "Kismet/GameplayStatics.h"

ATutorialManager::ATutorialManager()
{
	PrimaryActorTick.bCanEverTick = false;
	CurrentStep = ETutorialStep::Welcome;
}

void ATutorialManager::BeginPlay()
{
	Super::BeginPlay();
	CheckAndStartTutorial();
}

void ATutorialManager::CheckAndStartTutorial()
{
	UMySaveGame* SaveGame = Cast<UMySaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));
	if (SaveGame && SaveGame->bHasCompletedTutorial)
	{
		CurrentStep = ETutorialStep::Completed;
	}
	else
	{
		// 
		StartSpecificTutorial(ETutorialStep::Welcome);
	}
}

void ATutorialManager::StartSpecificTutorial(ETutorialStep StepToStart)
{
	CurrentStep = StepToStart;
	OnStepChanged.Broadcast(CurrentStep);
}

void ATutorialManager::CompleteStep(ETutorialStep StepCompleted)
{
	if (CurrentStep == StepCompleted && CurrentStep != ETutorialStep::Completed)
	{
		uint8 NextStepNum = static_cast<uint8>(CurrentStep) + 1;
		CurrentStep = static_cast<ETutorialStep>(NextStepNum);
		OnStepChanged.Broadcast(CurrentStep);

		if (CurrentStep == ETutorialStep::Completed)
		{
			FinishTutorial();
		}
	}
}

void ATutorialManager::FinishTutorial()
{
	UMySaveGame* SaveGame = Cast<UMySaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));
	if (!SaveGame)
	{
		SaveGame = Cast<UMySaveGame>(UGameplayStatics::CreateSaveGameObject(UMySaveGame::StaticClass()));
	}
	
	if (SaveGame)
	{
		SaveGame->bHasCompletedTutorial = true;
		UGameplayStatics::SaveGameToSlot(SaveGame, SaveSlotName, 0);
	}
}