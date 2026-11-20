// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "A1DayNightWidget.generated.h"

class AA1DayNightManager;
class UTextBlock;
enum class EDayPhase : uint8;
class UImage;
/**
 *
 */
UCLASS()
class A1GAME_API UA1DayNightWidget : public UUserWidget
{
	GENERATED_BODY()
protected:
	//Function Section
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION()
	void OnDayPhaseChanged(EDayPhase NewPhase, int32 CurrentDay);

	UFUNCTION()
	void OnDayChanged(int32 NewDay);

	UFUNCTION()
	void OnTimeChanged(int32 Hour, int32 Minute);

	void UpdateUI();

	void UpdateTimeDisplay(int32 Hour, int32 Minute);

	//Variable Section
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> DayNightIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> DayCount;

	//ICon Image
	UPROPERTY(EditDefaultsOnly, Category = "DayNight|Icon UI")
	TObjectPtr<UTexture2D> DayIcon;

	UPROPERTY(EditDefaultsOnly, Category = "DayNight|Icon UI")
	TObjectPtr<UTexture2D> NightIcon;

	// 시간 표시 텍스트
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Time;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<AA1DayNightManager> DayNightManager;

	// 시간 표시 형식 (12시간/24시간)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight UI")
	bool bUse12HourFormat = true;
};
