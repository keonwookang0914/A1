// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "CommonActivatableWidget.h"
#include "A1ActivatableWidget.generated.h"

class UTextBlock;

struct FTypingState
{
	FString FullText;
	FString CurrentText;
	int32 CurrentIndex = 0;
	UTextBlock* TargetTextBlock;
	FTimerHandle TimerHandle;
};

UCLASS()
class A1GAME_API UA1ActivatableWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UA1ActivatableWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	UFUNCTION(BlueprintCallable)
	void Deactivate();

protected:
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	UFUNCTION(BlueprintCallable)
	void AffectTypingEffect(UTextBlock* TargetTextBlock, FString InText, float delta, float startdelay);
	UFUNCTION(BlueprintCallable)
	void PlayErrorSound();


	UPROPERTY(EditAnywhere)
	USoundBase* TypingSound;

	UPROPERTY(EditAnywhere)
	USoundBase* ErrorSound;
};
