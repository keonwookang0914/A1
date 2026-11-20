// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "Score/A1ScoreManager.h"

#include "A1LogChannels.h"
#include "A1ScoreBlueprintFunctionLibrary.h"
#include "A1ScoreSaveGame.h"
#include "Kismet/GameplayStatics.h"

UA1ScoreManager* UA1ScoreManager::Instance = nullptr;
const FString UA1ScoreManager::SaveSlotName = TEXT("A1ScoreData");

UA1ScoreManager* UA1ScoreManager::Get()
{
	if (!Instance)
	{
		Instance = NewObject<UA1ScoreManager>();
		Instance->AddToRoot();
		Instance->LoadScores();
	}

	return Instance;
}

void UA1ScoreManager::StartNewGame()
{
	CurrentGameScore = FA1ScoreData();
	CurrentGameScore.GameNumber = GetNextGameNumber();

	UE_LOG(LogA1ScoreSystem, Log, TEXT("[ScoreManager] Started Game %d"), CurrentGameScore.GameNumber);
}

void UA1ScoreManager::EndGame(EGameEndReason EndReason)
{
	CurrentGameScore.GameEndReason = EndReason;
	
	CurrentGameScore.CalculateScore(); // 최종 점수 계산

	// 저장된 점수에 추가
	if (!SavedScores)
	{
		SavedScores = Cast<UA1ScoreSaveGame>(UGameplayStatics::CreateSaveGameObject(UA1ScoreSaveGame::StaticClass()));
	}
	SavedScores->AddScore(CurrentGameScore);
	SaveScores();

	// 이벤트 브로드캐스트
	OnGameEnded.Broadcast(CurrentGameScore);

	SetDoTutorial(false);

	UE_LOG(LogA1ScoreSystem, Log, TEXT("[ScoreManager] Game %d ended. Final score: %d"), CurrentGameScore.GameNumber, CurrentGameScore.TotalScore);
	UE_LOG(LogA1ScoreSystem, Log, TEXT("[ScoreManager] Score breakdown: %s"), *CurrentGameScore.GetScoreBreakdown());
}

void UA1ScoreManager::AddDaySurvived(int32 Count)
{
	CurrentGameScore.DaysSurvived += Count;
	OnScoreChanged.Broadcast(CurrentGameScore);

	UE_LOG(LogA1ScoreSystem, Log, TEXT("[ScoreManager] Days survived: %d"), CurrentGameScore.DaysSurvived);
}

void UA1ScoreManager::AddInventoryItems(int32 Count)
{
	CurrentGameScore.InventoryItems += Count;
	OnScoreChanged.Broadcast(CurrentGameScore);

	UE_LOG(LogA1ScoreSystem, Log, TEXT("[ScoreManager] Inventory items: %d"), CurrentGameScore.InventoryItems);
}

void UA1ScoreManager::SetInventoryItems(int32 Count)
{
	CurrentGameScore.InventoryItems = FMath::Max(0, Count);
	OnScoreChanged.Broadcast(CurrentGameScore);

	UE_LOG(LogA1ScoreSystem, Log, TEXT("[ScoreManager] Inventory items: %d"), CurrentGameScore.InventoryItems);
}

int32 UA1ScoreManager::GetStorageItems() const
{
	return CurrentGameScore.StorageItems;
}

void UA1ScoreManager::SetStorageItems(int32 Count)
{
	CurrentGameScore.StorageItems = FMath::Max(0, Count);
	OnScoreChanged.Broadcast(CurrentGameScore);

	UE_LOG(LogA1ScoreSystem, Log, TEXT("[ScoreManager] Storage items: %d"), CurrentGameScore.StorageItems);
}

void UA1ScoreManager::AddConsumedItems(int32 Count)
{
	CurrentGameScore.ConsumedItems += FMath::Max(0, Count);
	OnScoreChanged.Broadcast(CurrentGameScore);

	UE_LOG(LogA1ScoreSystem, Log, TEXT("[ScoreManager] Consumed items: %d"), CurrentGameScore.ConsumedItems);
}

float UA1ScoreManager::GetTotalRepair() const
{
	return CurrentGameScore.TotalRepair;
}

void UA1ScoreManager::SetTotalRepair(int32 Count)
{
	CurrentGameScore.TotalRepair = FMath::Clamp(Count, 0, 50);
	OnScoreChanged.Broadcast(CurrentGameScore);

	UE_LOG(LogA1ScoreSystem, Log, TEXT("[ScoreManager] Total Repair : %d"), CurrentGameScore.TotalRepair);
}

float UA1ScoreManager::GetCompleteRepair() const
{
	return CurrentGameScore.CompleteRepair;
}

void UA1ScoreManager::SetCompleteRepair(int32 Count)
{
	CurrentGameScore.CompleteRepair = FMath::Clamp(Count, 0, 50);
	OnScoreChanged.Broadcast(CurrentGameScore);

	UE_LOG(LogA1ScoreSystem, Log, TEXT("[ScoreManager] Complete Repair : %d"), CurrentGameScore.CompleteRepair);
}

void UA1ScoreManager::SetRepairedBeforeEscape(bool bRepaired)
{
	CurrentGameScore.bRepairedBeforeEscape = bRepaired;
	OnScoreChanged.Broadcast(CurrentGameScore);

	UE_LOG(LogA1ScoreSystem, Log, TEXT("[ScoreManager] Repaired before escape: %s"),
		bRepaired ? TEXT("Yes") : TEXT("No"));
}

void UA1ScoreManager::SetRemainingFuel(int32 Fuel)
{
	CurrentGameScore.RemainingFuel = FMath::Max(0, Fuel);
	OnScoreChanged.Broadcast(CurrentGameScore);

	UE_LOG(LogA1ScoreSystem, Log, TEXT("[ScoreManager] Remaining fuel: %d"), CurrentGameScore.RemainingFuel);
}

bool UA1ScoreManager::SaveScores()
{
	if (!SavedScores)
	{
		return false;
	}

	bool bSaveSuccess = UGameplayStatics::SaveGameToSlot(SavedScores, SaveSlotName, 0);
	UE_LOG(LogA1ScoreSystem, Log, TEXT("[ScoreManager] Save result: %s"), bSaveSuccess ? TEXT("Success") : TEXT("Failed"));

	return bSaveSuccess;
}

bool UA1ScoreManager::LoadScores()
{
	if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
	{
		SavedScores = Cast<UA1ScoreSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));

		if (SavedScores)
		{
			UE_LOG(LogA1ScoreSystem, Log, TEXT("[ScoreManager] Loaded %d scores"), SavedScores->GetAllScores().Num());
			return true;
		}
	}

	// 저장 파일이 없거나 로드 실패시 새로 생성
	SavedScores = Cast<UA1ScoreSaveGame>(UGameplayStatics::CreateSaveGameObject(UA1ScoreSaveGame::StaticClass()));
	UE_LOG(LogA1ScoreSystem, Log, TEXT("[ScoreManager] Created new save data"));

	return SavedScores != nullptr;
}

TArray<FA1ScoreData> UA1ScoreManager::GetAllScores() const
{
	if (SavedScores)
	{
		return SavedScores->GetAllScores();
	}
	return TArray<FA1ScoreData>();
}

int32 UA1ScoreManager::GetHighestScore() const
{
	if (SavedScores)
	{
		return SavedScores->GetHighestScore();
	}
	return 0;
}

int32 UA1ScoreManager::GetNextGameNumber() const
{
	if (SavedScores && SavedScores->GetAllScores().Num() > 0)
	{
		// 마지막 게임 번호 + 1
		int32 MaxGameNumber = 0;
		for (const FA1ScoreData& Score : SavedScores->GetAllScores())
		{
			MaxGameNumber = FMath::Max(MaxGameNumber, Score.GameNumber);
		}
		return MaxGameNumber + 1;
	}
	return 1; // 첫 게임
}

void UA1ScoreManager::ClearAllData()
{
	if (SavedScores)
	{
		SavedScores->ClearAllScore();
		SaveScores();
		UE_LOG(LogA1ScoreSystem, Log, TEXT("[ScoreManager] All score data cleared"));
	}
}

int32 UA1ScoreManager::CalculateCurrentScore() const
{
	FA1ScoreData TempScore = CurrentGameScore;
	TempScore.CalculateScore();
	return TempScore.TotalScore;
}

FString UA1ScoreManager::GetCurrentScoreBreakdown() const
{
	FA1ScoreData TempScore = CurrentGameScore;
	TempScore.CalculateScore();
	return TempScore.GetScoreBreakdown();
}

void UA1ScoreManager::SetDoTutorial(bool InDoTutorial)
{
	 bDoTutorial = InDoTutorial;
	 UE_LOG(LogA1ScoreSystem, Log, TEXT("Set Do Tutoiral: %s"), bDoTutorial ? TEXT("True") : TEXT("False"));
}
