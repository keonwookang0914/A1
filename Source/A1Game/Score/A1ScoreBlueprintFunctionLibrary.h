// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "A1ScoreData.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "A1ScoreBlueprintFunctionLibrary.generated.h"

enum class EGameEndReason : uint8;
/**
 * 
 */
UCLASS()
class A1GAME_API UA1ScoreBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
    // === 게임 세션 관리 ===
    UFUNCTION(BlueprintCallable, Category = "Score")
    static void StartNewGame();

    UFUNCTION(BlueprintCallable, Category = "Score")
    static void EndGame(EGameEndReason EndReason);

    // === 점수 데이터 설정 ===
    UFUNCTION(BlueprintCallable, Category = "Score")
    static void AddDaySurvived(int32 Count = 1);

    UFUNCTION(BlueprintCallable, Category = "Score")
    static void AddInventoryItems(int32 Count = 1);

    UFUNCTION(BlueprintCallable, Category = "Score")
    static void SetInventoryItems(int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Score")
    static int32 GetStorageItems();

    UFUNCTION(BlueprintCallable, Category = "Score")
    static void SetStorageItems(int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Score")
    static void AddConsumedItems(int32 Count = 1);

    UFUNCTION(BlueprintCallable, Category = "Score")
    static float GetTotalRepair();

    UFUNCTION(BlueprintCallable, Category = "Score")
    static void SetTotalRepair(int32 count);

    UFUNCTION(BlueprintCallable, Category = "Score")
    static float GetCompleteRepair();

    UFUNCTION(BlueprintCallable, Category = "Score")
    static void SetCompleteRepair(int32 count);

    UFUNCTION(BlueprintCallable, Category = "Score")
    static void SetRepairedBeforeEscape(bool bRepaired);

    UFUNCTION(BlueprintCallable, Category = "Score")
    static void SetRemainingFuel(int32 Fuel);

    // === 편의 함수들 ===
    UFUNCTION(BlueprintCallable, Category = "Score")
    static void UpdateItemCounts(int32 InventoryCount, int32 StorageCount);

    UFUNCTION(BlueprintCallable, Category = "Score")
    static void ConsumeItem(int32 Count = 1);

    // === 데이터 가져오기 ===
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Score")
    static FA1ScoreData GetCurrentScore();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Score")
    static TArray<FA1ScoreData> GetAllScores();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Score")
    static int32 GetHighestScore();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Score")
    static int32 GetNextGameNumber();

    // === 점수 미리보기 ===
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Score")
    static int32 CalculateCurrentScore();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Score")
    static FString GetCurrentScoreBreakdown();

    // === 유틸리티 ===
    UFUNCTION(BlueprintCallable, Category = "Score")
    static void ClearAllScores();

    // === 점수 매니저 직접 가져오기 (고급 사용자용) ===
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Score")
    static class UA1ScoreManager* GetScoreManager();

    // === 점수 계산 공식 정보 ===
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Score")
    static FString GetScoreFormula();

    // === 생존 점수 계산 (미리보기용) ===
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Score")
    static int32 CalculateSurvivalScore(int32 Days);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Score")
    static FString GetSurvivalScoreBreakdown(int32 Days);

	UFUNCTION(BlueprintCallable, Category = "Score")
	static bool GetDoTutorial();

	UFUNCTION(BlueprintCallable, Category = "Score")
	static void SetDoTutorial(bool InTutorial);
};
