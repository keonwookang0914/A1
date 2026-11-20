// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "UI/A1ScoreListWidget.h"

#include "A1ScoreEntryButtonWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "Components/HorizontalBox.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Score/A1ScoreBlueprintFunctionLibrary.h"

void UA1ScoreListWidget::NativeConstruct()
{
	Super::NativeConstruct();

    // 정렬 옵션 추가
    if (SortComboBox)
    {
        SortComboBox->ClearOptions();
        SortComboBox->AddOption(TEXT("Highest Score"));
        SortComboBox->AddOption(TEXT("Lowest Score"));
        SortComboBox->AddOption(TEXT("Current"));
        SortComboBox->AddOption(TEXT("Oldest"));
        SortComboBox->SetSelectedIndex(0);
        SortComboBox->OnSelectionChanged.AddDynamic(this, &UA1ScoreListWidget::OnSortChanged);
    }

    // 버튼 이벤트 바인딩
    if (RefreshButton)
    {
        RefreshButton->OnClicked.AddDynamic(this, &UA1ScoreListWidget::OnRefreshClicked);
    }

    if (ClearButton)
    {
        ClearButton->OnClicked.AddDynamic(this, &UA1ScoreListWidget::OnClearClicked);
    }

    // 초기 데이터 로드
    RefreshScoreList();

}

void UA1ScoreListWidget::NativeDestruct()
{
	Super::NativeDestruct();
    if (SortComboBox)
    {
        SortComboBox->OnSelectionChanged.Clear();
    }
    if (RefreshButton)
    {
        RefreshButton->OnClicked.Clear();
    }
    if (ClearButton)
    {
        ClearButton->OnClicked.Clear();
    }
}

TOptional<FUIInputConfig> UA1ScoreListWidget::GetDesiredInputConfig() const
{
	return Super::GetDesiredInputConfig();
}

void UA1ScoreListWidget::RefreshScoreList()
{
    // 현재 표시된 점수 목록 업데이트
    CurrentScores = UA1ScoreBlueprintFunctionLibrary::GetAllScores();

    // 정렬 적용
    if (SortComboBox)
    {
        int32 SortType = SortComboBox->GetSelectedIndex();
        SortScores(CurrentScores, SortType);
    }

    // UI 업데이트
    PopulateScoreList(CurrentScores);

    // 통계 업데이트
    if (TotalGamesText)
    {
        TotalGamesText->SetText(FText::FromString(FString::Printf(TEXT("Total Game Count: %d"), CurrentScores.Num())));
    }

    if (HighestScoreText)
    {
        int32 HighScore = UA1ScoreBlueprintFunctionLibrary::GetHighestScore();
        HighestScoreText->SetText(FText::FromString(FString::Printf(TEXT("Highest Score: %d"), HighScore)));
    }
}

void UA1ScoreListWidget::OnSortChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
    RefreshScoreList();
}

void UA1ScoreListWidget::OnRefreshClicked()
{
    RefreshScoreList();
}

void UA1ScoreListWidget::OnClearClicked()
{
    UA1ScoreBlueprintFunctionLibrary::ClearAllScores();
    RefreshScoreList();
}

void UA1ScoreListWidget::PopulateScoreList(const TArray<FA1ScoreData>& AllScores)
{
    if (!ScoreScrollBox)
    {
        UE_LOG(LogA1ScoreSystem, Warning, TEXT("[ScoreList] ScoreScrollBox is null!"));
        return;
    }

    // 기존 항목들 제거
    ScoreScrollBox->ClearChildren();

    if (AllScores.Num() == 0)
    {
        // 빈 상태 메시지
        UTextBlock* EmptyText = WidgetTree->ConstructWidget<UTextBlock>();
        if (EmptyText)
        {
            EmptyText->SetText(FText::FromString(TEXT("No Saved Data.")));
            EmptyText->SetJustification(ETextJustify::Center);
            ScoreScrollBox->AddChild(EmptyText);
        }
        return;
    }

    // 각 점수 항목 생성
    for (int32 i = 0; i < AllScores.Num(); i++)
    {
        const FA1ScoreData& ScoreData = AllScores[i];
        UA1ScoreEntryButtonWidget* EntryWidget = CreateScoreEntry(ScoreData, i + 1, i);

        if (EntryWidget)
        {
            ScoreScrollBox->AddChild(EntryWidget);
            UE_LOG(LogA1ScoreSystem, Log, TEXT("[ScoreList] Added entry %d: %s"), i, *ScoreData.GetGameName());
        }
        else
        {
            UE_LOG(LogA1ScoreSystem, Warning, TEXT("[ScoreList] Failed to create entry %d"), i);
        }
        UE_LOG(LogA1ScoreSystem, Log, TEXT("[ScoreList] Populated %d score entries"), AllScores.Num());
    }
}

void UA1ScoreListWidget::SortScores(TArray<FA1ScoreData>& Scores, int32 SortType)
{
    switch (SortType)
    {
    case 0: // 점수 높은순
        Scores.Sort([](const FA1ScoreData& A, const FA1ScoreData& B) {
            return A.TotalScore > B.TotalScore;
            });
        break;

    case 1: // 점수 낮은순
        Scores.Sort([](const FA1ScoreData& A, const FA1ScoreData& B) {
            return A.TotalScore < B.TotalScore;
            });
        break;

    case 2: // 최신순
        Scores.Sort([](const FA1ScoreData& A, const FA1ScoreData& B) {
            return A.Timestamp > B.Timestamp;
            });
        break;

    case 3: // 오래된순
        Scores.Sort([](const FA1ScoreData& A, const FA1ScoreData& B) {
            return A.Timestamp < B.Timestamp;
            });
        break;
    }
}

UA1ScoreEntryButtonWidget* UA1ScoreListWidget::CreateScoreEntry(const FA1ScoreData& ScoreData, int32 Rank,
	int32 Index)
{
    // 블루프린트 위젯 클래스가 설정되었는지 확인
    if (!ScoreEntryWidgetClass)
    {
        UE_LOG(LogA1ScoreSystem, Error, TEXT("[ScoreList] ScoreEntryWidgetClass is not set! Please assign WBP_ScoreEntry in Blueprint."));
        return nullptr;
    }

    // 블루프린트 위젯 생성
    UA1ScoreEntryButtonWidget* EntryWidget = CreateWidget<UA1ScoreEntryButtonWidget>(this, ScoreEntryWidgetClass);

    if (EntryWidget)
    {
        // 데이터 설정
        EntryWidget->SetupScoreEntry(ScoreData, Rank, Index);

        UE_LOG(LogA1ScoreSystem, Log, TEXT("[ScoreList] Created entry widget for %s"), *ScoreData.GetGameName());
    }
    else
    {
        UE_LOG(LogA1ScoreSystem, Error, TEXT("[ScoreList] Failed to create widget from class: %s"),
            ScoreEntryWidgetClass ? *ScoreEntryWidgetClass->GetName() : TEXT("NULL"));
    }

    return EntryWidget;
}
