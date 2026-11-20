// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "Score/A1ScoreBlueprintFunctionLibrary.h"

#include "A1ScoreManager.h"

void UA1ScoreBlueprintFunctionLibrary::StartNewGame()
{
    UA1ScoreManager::Get()->StartNewGame();
}

void UA1ScoreBlueprintFunctionLibrary::EndGame(EGameEndReason EndReason)
{
    UA1ScoreManager::Get()->EndGame(EndReason);
}

void UA1ScoreBlueprintFunctionLibrary::AddDaySurvived(int32 Count)
{
    UA1ScoreManager::Get()->AddDaySurvived(Count);
}

void UA1ScoreBlueprintFunctionLibrary::AddInventoryItems(int32 Count)
{
    UA1ScoreManager::Get()->AddInventoryItems(Count);
}

void UA1ScoreBlueprintFunctionLibrary::SetInventoryItems(int32 Count)
{
    UA1ScoreManager::Get()->SetInventoryItems(Count);
}

int32 UA1ScoreBlueprintFunctionLibrary::GetStorageItems()
{
    return UA1ScoreManager::Get()->GetStorageItems();
}

void UA1ScoreBlueprintFunctionLibrary::SetStorageItems(int32 Count)
{
    UA1ScoreManager::Get()->SetStorageItems(Count);
}

void UA1ScoreBlueprintFunctionLibrary::AddConsumedItems(int32 Count)
{
    UA1ScoreManager::Get()->AddConsumedItems(Count);
}

float UA1ScoreBlueprintFunctionLibrary::GetTotalRepair()
{
    return UA1ScoreManager::Get()->GetTotalRepair();
}

void UA1ScoreBlueprintFunctionLibrary::SetTotalRepair(int32 count)
{
    UA1ScoreManager::Get()->SetTotalRepair(count);
}

float UA1ScoreBlueprintFunctionLibrary::GetCompleteRepair()
{
    return UA1ScoreManager::Get()->GetCompleteRepair();
}

void UA1ScoreBlueprintFunctionLibrary::SetCompleteRepair(int32 count)
{
    UA1ScoreManager::Get()->SetCompleteRepair(count);
}

void UA1ScoreBlueprintFunctionLibrary::SetRepairedBeforeEscape(bool bRepaired)
{
    UA1ScoreManager::Get()->SetRepairedBeforeEscape(bRepaired);
}

void UA1ScoreBlueprintFunctionLibrary::SetRemainingFuel(int32 Fuel)
{
    UA1ScoreManager::Get()->SetRemainingFuel(Fuel);
}

void UA1ScoreBlueprintFunctionLibrary::UpdateItemCounts(int32 InventoryCount, int32 StorageCount)
{
    UA1ScoreManager* Manager = UA1ScoreManager::Get();
    Manager->SetInventoryItems(InventoryCount);
    Manager->SetStorageItems(StorageCount);
}

void UA1ScoreBlueprintFunctionLibrary::ConsumeItem(int32 Count)
{
    UA1ScoreManager::Get()->AddConsumedItems(Count);
}

FA1ScoreData UA1ScoreBlueprintFunctionLibrary::GetCurrentScore()
{
    return UA1ScoreManager::Get()->GetCurrentScore();
}

TArray<FA1ScoreData> UA1ScoreBlueprintFunctionLibrary::GetAllScores()
{
    return UA1ScoreManager::Get()->GetAllScores();
}

int32 UA1ScoreBlueprintFunctionLibrary::GetHighestScore()
{
    return UA1ScoreManager::Get()->GetHighestScore();
}

int32 UA1ScoreBlueprintFunctionLibrary::GetNextGameNumber()
{
    return UA1ScoreManager::Get()->GetNextGameNumber();
}

int32 UA1ScoreBlueprintFunctionLibrary::CalculateCurrentScore()
{
    return UA1ScoreManager::Get()->CalculateCurrentScore();
}

FString UA1ScoreBlueprintFunctionLibrary::GetCurrentScoreBreakdown()
{
    return UA1ScoreManager::Get()->GetCurrentScoreBreakdown();
}

void UA1ScoreBlueprintFunctionLibrary::ClearAllScores()
{
    UA1ScoreManager::Get()->ClearAllData();
}

UA1ScoreManager* UA1ScoreBlueprintFunctionLibrary::GetScoreManager()
{
    return UA1ScoreManager::Get();
}

FString UA1ScoreBlueprintFunctionLibrary::GetScoreFormula()
{
    return TEXT("점수 = (생존일수×100) + (보관아이템×100, MAX:5200) + (소모아이템×120) + 탈출보너스(5000) + (수리율÷4×40 + 수리후탈출시1000) + (연료÷1000×100, MAX:20000)");
}

int32 UA1ScoreBlueprintFunctionLibrary::CalculateSurvivalScore(int32 Days)
{
    int32 TotalScore = 0;

    if (Days <= 0)
    {
        return 0;
    }

    // Day 1~10: 일 당 50점 (MAX 500)
    if (Days >= 1)
    {
        int32 Phase1Days = FMath::Min(Days, 10);
        TotalScore += Phase1Days * 50;
    }

    // Day 11~20: 일 당 100점 (MAX 1000)
    if (Days >= 11)
    {
        int32 Phase2Days = FMath::Min(Days - 10, 10);
        TotalScore += Phase2Days * 100;
    }

    // Day 21~30: 일 당 150점 (MAX 1500)
    if (Days >= 21)
    {
        int32 Phase3Days = FMath::Min(Days - 20, 10);
        TotalScore += Phase3Days * 150;
    }

    // Day 31~40: 일 당 300점 (MAX 3000)
    if (Days >= 31)
    {
        int32 Phase4Days = FMath::Min(Days - 30, 10);
        TotalScore += Phase4Days * 300;
    }

    // Day 41~: 일 당 0점 (존버 방지용)
    // 41일 이후는 점수 추가 없음

    return TotalScore;
}

FString UA1ScoreBlueprintFunctionLibrary::GetSurvivalScoreBreakdown(int32 Days)
{
    if (Days <= 0)
    {
        return TEXT("생존 0일: 0점");
    }

    FString Breakdown = TEXT("생존 점수 상세:\n");
    int32 TotalScore = 0;

    // Day 1~10: 일 당 50점
    if (Days >= 1)
    {
        int32 Phase1Days = FMath::Min(Days, 10);
        int32 Phase1Score = Phase1Days * 50;
        TotalScore += Phase1Score;
        Breakdown += FString::Printf(TEXT("• 1~10일: %d일 × 50점 = %d점\n"), Phase1Days, Phase1Score);
    }

    // Day 11~20: 일 당 100점
    if (Days >= 11)
    {
        int32 Phase2Days = FMath::Min(Days - 10, 10);
        int32 Phase2Score = Phase2Days * 100;
        TotalScore += Phase2Score;
        Breakdown += FString::Printf(TEXT("• 11~20일: %d일 × 100점 = %d점\n"), Phase2Days, Phase2Score);
    }

    // Day 21~30: 일 당 150점
    if (Days >= 21)
    {
        int32 Phase3Days = FMath::Min(Days - 20, 10);
        int32 Phase3Score = Phase3Days * 150;
        TotalScore += Phase3Score;
        Breakdown += FString::Printf(TEXT("• 21~30일: %d일 × 150점 = %d점\n"), Phase3Days, Phase3Score);
    }

    // Day 31~40: 일 당 300점
    if (Days >= 31)
    {
        int32 Phase4Days = FMath::Min(Days - 30, 10);
        int32 Phase4Score = Phase4Days * 300;
        TotalScore += Phase4Score;
        Breakdown += FString::Printf(TEXT("• 31~40일: %d일 × 300점 = %d점\n"), Phase4Days, Phase4Score);
    }

    // Day 41~: 존버 방지
    if (Days >= 41)
    {
        int32 ExtraDays = Days - 40;
        Breakdown += FString::Printf(TEXT("• 41일 이후: %d일 × 0점 = 0점 (존버 방지)\n"), ExtraDays);
    }

    Breakdown += FString::Printf(TEXT("총 생존 점수: %d점"), TotalScore);
    return Breakdown;
}

bool UA1ScoreBlueprintFunctionLibrary::GetDoTutorial()
{
	return UA1ScoreManager::Get()->GetDoTutorial();
}

void UA1ScoreBlueprintFunctionLibrary::SetDoTutorial(bool InTutorial)
{
	UA1ScoreManager::Get()->SetDoTutorial(InTutorial);
}
