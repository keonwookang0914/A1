// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/LyraActivatableWidget.h"
#include "A1ScoreListWidget.generated.h"

class UA1ScoreEntryButtonWidget;
struct FA1ScoreData;
class UTextBlock;
class UButton;
class UComboBoxString;
class UScrollBox;
/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class A1GAME_API UA1ScoreListWidget : public ULyraActivatableWidget
{
    GENERATED_BODY()
public:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;

protected:
    // UI 요소들 (Blueprint에서 바인딩)
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UScrollBox> ScoreScrollBox;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UComboBoxString> SortComboBox;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UButton> RefreshButton;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UButton> ClearButton;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UTextBlock> TotalGamesText;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UTextBlock> HighestScoreText;

    // 블루프린트 위젯 클래스들 (Blueprint에서 할당)
    UPROPERTY(EditAnywhere, Category = "UI", BlueprintReadOnly)
    TSubclassOf<UA1ScoreEntryButtonWidget> ScoreEntryWidgetClass;

public:
    // 메인 기능
    UFUNCTION(BlueprintCallable)
    void RefreshScoreList();

protected:
    // 이벤트 핸들러
    UFUNCTION()
    void OnSortChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

    UFUNCTION()
    void OnRefreshClicked();

    UFUNCTION()
    void OnClearClicked();

private:
    // 현재 표시된 점수 목록
    UPROPERTY()
    TArray<FA1ScoreData> CurrentScores;

    // 내부 함수
    void PopulateScoreList(const TArray<FA1ScoreData>& AllScores);
    void SortScores(TArray<FA1ScoreData>& Scores, int32 SortType);
    UA1ScoreEntryButtonWidget* CreateScoreEntry(const FA1ScoreData& ScoreData, int32 Rank, int32 Index);
};