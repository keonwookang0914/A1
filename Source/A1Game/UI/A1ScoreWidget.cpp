// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "UI/A1ScoreWidget.h"

#include "Components/TextBlock.h"
#include "Score/A1ScoreBlueprintFunctionLibrary.h"
#include "Score/A1ScoreData.h"
#include "Score/A1ScoreManager.h"

void UA1ScoreWidget::SetEndingText()
{
	const FA1ScoreData& Data = UA1ScoreManager::Get()->GetCurrentScore();
	AffectTypingEffect(Survival, IntegertoString(FText::FromString("Survival Day : "), Data.CalculateSurvivalScore(Data.DaysSurvived)), 0.1f, 0.0f);
	//Survival->SetText(FText::FromString(IntegertoString(Survival->GetText(), Data.CalculateSurvivalScore(Data.DaysSurvived))));

	int32 TotalItemCount = (Data.InventoryItems + Data.StorageItems) * 100;
	AffectTypingEffect(Item, IntegertoString(FText::FromString("Item Score : "), TotalItemCount), 0.1f, 1.0f);
	//Item->SetText(FText::FromString(IntegertoString(Item->GetText(), TotalItemCount)));

	AffectTypingEffect(UsedItem, IntegertoString(FText::FromString("Used Item Score : "), Data.ConsumedItems * 120), 0.1f, 2.0f);
	//UsedItem->SetText(FText::FromString(IntegertoString(UsedItem->GetText(), Data.ConsumedItems * 120)));

	int32 BonusValue = Data.bRepairedBeforeEscape ? 5000 : 0;
	AffectTypingEffect(Escape, IntegertoString(FText::FromString("Escape Bonuse Score : "), BonusValue), 0.1f, 3.0f);
	//Escape->SetText(FText::FromString(IntegertoString(Escape->GetText(), BonusValue)));

	AffectTypingEffect(Repair, IntegertoString(FText::FromString("Repair Score : "), Data.CompleteRepair * 20), 0.1f, 4.0f);
	//Repair->SetText(FText::FromString(IntegertoString(Repair->GetText(), Data.CompleteRepair * 20)));

	AffectTypingEffect(Fuel, IntegertoString(FText::FromString("Fuel Score : "), (Data.RemainingFuel / 1000) * 100), 0.1f, 5.0f);
	//Fuel->SetText(FText::FromString(IntegertoString(Fuel->GetText(), (Data.RemainingFuel / 1000) * 100)));

	AffectTypingEffect(Total, IntegertoString(FText::FromString("Total : "), Data.TotalScore), 0.1f, 6.0f);
	//Total->SetText(FText::FromString(IntegertoString(Total->GetText(), Data.TotalScore)));
}

FString UA1ScoreWidget::IntegertoString(FText InText, int32 value)
{
	FString CurrentString = InText.ToString();
	return CurrentString + FString::FromInt(value);
}

FString UA1ScoreWidget::FloatToString(FText InText, float value)
{
	FString CurrentString = InText.ToString();
	return CurrentString + FString::SanitizeFloat(value);
}

TOptional<FUIInputConfig> UA1ScoreWidget::GetDesiredInputConfig() const
{
	return Super::GetDesiredInputConfig();
}
