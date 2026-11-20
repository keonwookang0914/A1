// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Score/A1ScoreData.h"
#include "UI/LyraActivatableWidget.h"
#include "A1ScoreDetailWidget.generated.h"

struct FA1ScoreData;
class UScrollBox;
class UComboBoxString;
class UButton;
class UTextBlock;
/**
 * 
 */
UCLASS()
class A1GAME_API UA1ScoreDetailWidget : public ULyraActivatableWidget
{
	GENERATED_BODY()
public:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

protected:
    // UI 요소들 (Blueprint에서 바인딩)
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UTextBlock* TotalScoreText;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UTextBlock* SurvivalScoreText;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UTextBlock* ItemScoreText;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UTextBlock* ItemDetailText;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UTextBlock* EscapeScoreText;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UTextBlock* RepairScoreText;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UTextBlock* FuelScoreText;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UTextBlock* FuelDetailText;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    UTextBlock* DateText;

public:
    // 메인 기능
    UFUNCTION(BlueprintCallable)
    void SetScoreData(const FA1ScoreData& ScoreData);

private:
    // 저장된 데이터
    UPROPERTY()
    FA1ScoreData CurrentScoreData;

    // 유틸리티 함수
    FString FormatScore(int32 Score) const;
};
