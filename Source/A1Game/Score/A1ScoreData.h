#pragma once

#include "CoreMinimal.h"
#include "A1LogChannels.h"
#include "A1ScoreData.generated.h"

UENUM(BlueprintType)
enum class EGameEndReason : uint8
{
    Death,
    Escape,
    Unknown
};

USTRUCT(BlueprintType)
struct FA1ScoreData
{
    GENERATED_BODY()

public:
    FA1ScoreData()
    {
        GameNumber = 1;
        DaysSurvived = 1;
        InventoryItems = 0;
        StorageItems = 0;
        ConsumedItems = 0;
        TotalRepair = 0;
        CompleteRepair = 0;
        bRepairedBeforeEscape = false;
        RemainingFuel = 0;
        TotalScore = 0;
        GameEndReason = EGameEndReason::Unknown;
        Timestamp = FDateTime::Now();
    }

public:
    // 게임 회차 (1, 2, 3, 4...)
    UPROPERTY(BlueprintReadWrite, Category = "Score")
    int32 GameNumber;

    // 생존 일수
    UPROPERTY(BlueprintReadWrite, Category = "Score")
    int32 DaysSurvived;

    // 아이템 관련
    UPROPERTY(BlueprintReadWrite, Category = "Score")
    int32 InventoryItems;        // 인벤토리 아이템 수

    UPROPERTY(BlueprintReadWrite, Category = "Score")
    int32 StorageItems;          // 창고 아이템 수

    UPROPERTY(BlueprintReadWrite, Category = "Score")
    int32 ConsumedItems;         // 소모된 아이템 수

    // 수리 관련
    UPROPERTY(BlueprintReadWrite, Category = "Score")
    int32 TotalRepair;

    UPROPERTY(BlueprintReadWrite, Category = "Score")
    int32 CompleteRepair;

    UPROPERTY(BlueprintReadWrite, Category = "Score")
    bool bRepairedBeforeEscape;  // 수리 후 탈출 여부

    // 연료 관련
    UPROPERTY(BlueprintReadWrite, Category = "Score")
    int32 RemainingFuel;         // 남은 연료량

    // 결과
    UPROPERTY(BlueprintReadWrite, Category = "Score")
    int32 TotalScore;

    UPROPERTY(BlueprintReadWrite, Category = "Score")
    EGameEndReason GameEndReason;

    UPROPERTY(BlueprintReadWrite, Category = "Score")
    FDateTime Timestamp;

public:
    // 점수 계산
    void CalculateScore()
    {
        // 1. 생존 일수 점수
        int32 SurvivalScore = CalculateSurvivalScore(DaysSurvived);
        UE_LOG(LogA1ScoreSystem, Log, TEXT("SurvivalScore : %d"), SurvivalScore);

        // 2. 아이템 점수 (MAX 5200 + 소모된 아이템 점수)
        int32 StoredItemScore = FMath::Min((InventoryItems + StorageItems) * 100, 5200);
        int32 ConsumedItemScore = ConsumedItems * 120;
        int32 ItemScore = StoredItemScore + ConsumedItemScore;

        // 3. 탈출/사망 보너스
        int32 EndGameBonus = 0;
        if (GameEndReason == EGameEndReason::Escape)
        {
            EndGameBonus = 5000; // 구조선 탈출
        }
        // 사망시 0점

        // 4. 수리 점수
        int32 RepairScore = CompleteRepair * 20;

        // 5. 연료 점수 (MAX 20000점)
        int32 FuelScore = FMath::Min((RemainingFuel / 1000) * 100, 20000);

        // 총점 계산
        TotalScore = SurvivalScore + ItemScore + EndGameBonus + RepairScore + FuelScore;
    }

    // 게임 이름 반환 (Game 1, Game 2, ...)
    FORCEINLINE FString GetGameName() const
    {
        return FString::Printf(TEXT("Game %d"), GameNumber);
    }

    // 점수 상세 정보 반환 (디버깅용)
    FString GetScoreBreakdown() const
    {
        int32 SurvivalScore = CalculateSurvivalScore(DaysSurvived);
        int32 StoredItemScore = FMath::Min((InventoryItems + StorageItems) * 100, 5200);
        int32 ConsumedItemScore = ConsumedItems * 120;
        int32 EndGameBonus = (GameEndReason == EGameEndReason::Escape) ? 5000 : 0;
        int32 RepairScore = CompleteRepair * 20;
        if (bRepairedBeforeEscape && GameEndReason == EGameEndReason::Escape)
        {
            RepairScore += 1000;
        }
        int32 FuelScore = FMath::Min((RemainingFuel / 1000) * 100, 20000);

        return FString::Printf(
            TEXT("생존: %d, 보관아이템: %d, 소모아이템: %d, 탈출: %d, 수리: %d, 연료: %d = 총점: %d"),
            SurvivalScore, StoredItemScore, ConsumedItemScore, EndGameBonus, RepairScore, FuelScore, TotalScore
        );
    }
	public:
        // 생존 일수별 점수 계산
        static int32 CalculateSurvivalScore(int32 Days)
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
};