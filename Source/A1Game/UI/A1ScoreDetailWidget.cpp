// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "UI/A1ScoreDetailWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Score/A1ScoreBlueprintFunctionLibrary.h"

void UA1ScoreDetailWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UA1ScoreDetailWidget::NativeDestruct()
{
	Super::NativeDestruct();
}

void UA1ScoreDetailWidget::SetScoreData(const FA1ScoreData& ScoreData)
{
    CurrentScoreData = ScoreData;

    if (TotalScoreText)
    {
        TotalScoreText->SetText(FText::FromString(FString::Printf(TEXT("Total Score: %s"), *FormatScore(ScoreData.TotalScore))));
    }

    // 생존 점수 계산 및 표시
    int32 SurvivalScore = UA1ScoreBlueprintFunctionLibrary::CalculateSurvivalScore(ScoreData.DaysSurvived);
    if (SurvivalScoreText)
    {
        SurvivalScoreText->SetText(FText::FromString(FString::Printf(TEXT("Survival Score: %s"), *FormatScore(SurvivalScore))));
    }

    // 아이템 점수 계산 및 표시
    int32 StoredItemScore = FMath::Min((ScoreData.InventoryItems + ScoreData.StorageItems) * 100, 5200);
    int32 ConsumedItemScore = ScoreData.ConsumedItems * 120;
    int32 TotalItemScore = StoredItemScore + ConsumedItemScore;

    if (ItemScoreText)
    {
        ItemScoreText->SetText(FText::FromString(FString::Printf(TEXT("Item Score: %s"), *FormatScore(TotalItemScore))));
    }

    if (ItemDetailText)
    {
        FString ItemDetail = FString::Printf(
            TEXT("  -  Saved Item: %s (%d)\n  -  Used Item: %s (%d)"),
            *FormatScore(StoredItemScore), ScoreData.InventoryItems + ScoreData.StorageItems,
            *FormatScore(ConsumedItemScore), ScoreData.ConsumedItems
        );
        ItemDetailText->SetText(FText::FromString(ItemDetail));
    }

    // 탈출/사망 보너스
    if (EscapeScoreText)
    {
        FString EscapeText;
        if (ScoreData.GameEndReason == EGameEndReason::Escape)
        {
            EscapeText = TEXT("Escape Bonus: +5,000");
        }
        else
        {
            EscapeText = TEXT("Escape Bonus: 0 (Dead)");
        }
        EscapeScoreText->SetText(FText::FromString(EscapeText));
    }

    // 수리 점수 표시
    int32 BaseRepairScore = ScoreData.CompleteRepair * 20;

    if (RepairScoreText)
    {
        RepairScoreText->SetText(FText::FromString(FString::Printf(TEXT("Repair Score: %s"), *FormatScore(BaseRepairScore))));
    }

    // 연료 점수 계산 및 표시
    int32 FuelScore = FMath::Min((ScoreData.RemainingFuel / 1000) * 100, 20000);
    if (FuelScoreText)
    {
        FuelScoreText->SetText(FText::FromString(FString::Printf(TEXT("Fuel Score: %s"), *FormatScore(FuelScore))));
    }

    if (FuelDetailText)
    {
        FuelDetailText->SetText(FText::FromString(FString::Printf(TEXT("  -  Remaining Fuel Score: %d (%d×100)"), ScoreData.RemainingFuel, ScoreData.RemainingFuel / 1000)));
    }

    // 날짜 정보
    if (DateText)
    {
        DateText->SetText(FText::FromString(FString::Printf(TEXT("Play Date: %s"), *ScoreData.Timestamp.ToString(TEXT("%Y-%m-%d %H:%M")))));
    }
}

FString UA1ScoreDetailWidget::FormatScore(int32 Score) const
{
    // 천 단위 콤마 추가
    FString ScoreString = FString::FromInt(Score);
    FString FormattedScore;

    int32 CommaCount = 0;
    for (int32 i = ScoreString.Len() - 1; i >= 0; i--)
    {
        if (CommaCount == 3)
        {
            FormattedScore = TEXT(",") + FormattedScore;
            CommaCount = 0;
        }
        FormattedScore = FString(1, &ScoreString[i]) + FormattedScore;
        CommaCount++;
    }

    return FormattedScore;
}
