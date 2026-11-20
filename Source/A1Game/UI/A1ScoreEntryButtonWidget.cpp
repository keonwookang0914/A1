// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "UI/A1ScoreEntryButtonWidget.h"

#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/SizeBox.h"
#include "Score/A1ScoreBlueprintFunctionLibrary.h"
#include "UI/A1ScoreDetailWidget.h"

void UA1ScoreEntryButtonWidget::NativeConstruct()
{
	Super::NativeConstruct();

    // 버튼 클릭 이벤트 바인딩
    if (EntryButton)
    {
        EntryButton->OnClicked.AddDynamic(this, &UA1ScoreEntryButtonWidget::OnButtonClicked);
    }
}

void UA1ScoreEntryButtonWidget::NativeDestruct()
{
	Super::NativeDestruct();

    if (EntryButton)
    {
        EntryButton->OnClicked.Clear();
    }
}

void UA1ScoreEntryButtonWidget::SetupScoreEntry(const FA1ScoreData& ScoreData, int32 Rank, int32 Index)
{
    StoredIndex = Index;

    if (ScoreInfoText)
    {
        // 순위 아이콘
        FString RankIcon;
        RankIcon = FString::Printf(TEXT("%d."), Rank);

        // 결과 아이콘
        bool IsEscape = (ScoreData.GameEndReason == EGameEndReason::Escape);
        FString ResultIcon = IsEscape ? TEXT("Escape") : TEXT("Dead");
        Border_BG->SetBrushFromMaterial(IsEscape ? EscapeMaterial : DeadMaterial);

        // 텍스트 구성
        FString ScoreInfo = FString::Printf(
            TEXT("%s [%s] Score: %d | Survie %d Days| %s | %s"),
            *RankIcon,
            *ScoreData.GetGameName(),
            ScoreData.TotalScore,
            ScoreData.DaysSurvived,
            *ResultIcon,
            *ScoreData.Timestamp.ToString(TEXT("%m-%d"))
        );
        UE_LOG(LogTemp, Log, TEXT("%s"), *ScoreInfo);

        ScoreInfoText->SetText(FText::FromString(ScoreInfo));

        ScoreDetailWidget->SetScoreData(ScoreData);
    }
}

void UA1ScoreEntryButtonWidget::OnButtonClicked()
{
    bool IsOpen = DetailScoreBox->GetVisibility() == ESlateVisibility::Visible;

    if(IsOpen)
        DetailScoreBox->SetVisibility(ESlateVisibility::Collapsed);
    else
        DetailScoreBox->SetVisibility(ESlateVisibility::Visible);
}
