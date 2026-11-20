// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UI/A1ActivatableWidget.h"
#include "A1PlayTutorialVideoWidget.generated.h"

class UMediaPlayer;
class UImage;
/**
 * 
 */
UCLASS()
class A1GAME_API UA1PlayTutorialVideoWidget : public UA1ActivatableWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	void StartMovie();

protected:
	UPROPERTY(meta = (Bindwidget))
	TObjectPtr<UImage> TutorialVideo;

	UPROPERTY()
	TObjectPtr<UMediaPlayer> MediaPlayer;
};
