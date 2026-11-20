// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "A1ScoreData.h"
#include "UObject/NoExportTypes.h"
#include "A1ScoreManager.generated.h"

enum class EGameEndReason : uint8;
class UA1ScoreSaveGame;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScoreChanged, const FA1ScoreData&, CurrentScore);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameEnded, const FA1ScoreData&, FinalScore);

/**
 * 
 */
UCLASS(BlueprintType)
class A1GAME_API UA1ScoreManager : public UObject
{
	GENERATED_BODY()
public:
    // 싱글톤 인스턴스 가져오기
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Score", meta = (CallInEditor = "true"))
    static UA1ScoreManager* Get();


public:
    // 이벤트 델리게이트
    UPROPERTY(BlueprintAssignable, Category = "Score")
    FOnScoreChanged OnScoreChanged;

    UPROPERTY(BlueprintAssignable, Category = "Score")
    FOnGameEnded OnGameEnded;

public:
    // === 게임 세션 관리 ===
    UFUNCTION(BlueprintCallable, Category = "Score")
    void StartNewGame();

    UFUNCTION(BlueprintCallable, Category = "Score")
    void EndGame(EGameEndReason EndReason);

    // === 점수 데이터 설정 ===
    UFUNCTION(BlueprintCallable, Category = "Score")
    void AddDaySurvived(int32 Count = 1);

    UFUNCTION(BlueprintCallable, Category = "Score")
    void AddInventoryItems(int32 Count = 1);

    UFUNCTION(BlueprintCallable, Category = "Score")
    void SetInventoryItems(int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Score")
    int32 GetStorageItems() const;

    UFUNCTION(BlueprintCallable, Category = "Score")
    void SetStorageItems(int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Score")
    void AddConsumedItems(int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Score")
    float GetTotalRepair() const;

    UFUNCTION(BlueprintCallable, Category = "Score")
    void SetTotalRepair(int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Score")
    float GetCompleteRepair() const;

    UFUNCTION(BlueprintCallable, Category = "Score")
    void SetCompleteRepair(int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Score")
    void SetRepairedBeforeEscape(bool bRepaired);

    UFUNCTION(BlueprintCallable, Category = "Score")
    void SetRemainingFuel(int32 Fuel);

    // === 데이터 저장/로드 ===
    UFUNCTION(BlueprintCallable, Category = "Score")
    bool SaveScores();

    UFUNCTION(BlueprintCallable, Category = "Score")
    bool LoadScores();

    // === 데이터 가져오기 ===
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Score")
    TArray<FA1ScoreData> GetAllScores() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Score")
    int32 GetHighestScore() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Score")
    FA1ScoreData GetCurrentScore() const { return CurrentGameScore; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Score")
    int32 GetNextGameNumber() const;

    // === 유틸리티 ===
    UFUNCTION(BlueprintCallable, Category = "Score")
    void ClearAllData();

    // === 점수 미리보기 (저장 전 확인용) ===
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Score")
    int32 CalculateCurrentScore() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Score")
    FString GetCurrentScoreBreakdown() const;

    bool GetDoTutorial() const { return bDoTutorial; }

    UFUNCTION(BlueprintCallable)
    void SetDoTutorial(bool InDoTutorial);

protected:
    // 현재 게임 세션 점수
    UPROPERTY(BlueprintReadOnly, Category = "Score")
    FA1ScoreData CurrentGameScore;

    // 저장된 모든 점수
    UPROPERTY(BlueprintReadOnly, Category = "Score")
    TObjectPtr<UA1ScoreSaveGame> SavedScores;

private:
    static UA1ScoreManager* Instance;

    static const FString SaveSlotName;

    bool bDoTutorial = false;
};
