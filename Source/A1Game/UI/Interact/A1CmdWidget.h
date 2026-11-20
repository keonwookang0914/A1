// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "UI/A1ActivatableWidget.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "A1CmdWidget.generated.h"

class UImage;
class UTextBlock;
class UEditableText;
class UVerticalBox;
class UProgressBar;
class UAbilitySystemComponent;

USTRUCT(BlueprintType)
struct FASCInitializeMessage
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UAbilitySystemComponent> ASC;

	UPROPERTY(BlueprintReadWrite)
	bool bTutorial;
};

UCLASS()
class A1GAME_API UA1CmdWidget : public UA1ActivatableWidget
{
	GENERATED_BODY()

public:
	UA1CmdWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	//Temp eric1306
	UFUNCTION(BlueprintImplementableEvent)
	void LoadStory();
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void ConstructUI(FGameplayTag Channel, const FASCInitializeMessage& Message);

protected:
	UFUNCTION(BlueprintCallable)
	void InputEnded(FText InText);

	UFUNCTION(Blueprintimplementableevent)
	void ShowRepairPercent(float Percent);

	void ShowTutoMode();
	void ShowMenu();
	void HiddenMenu();

public:
	UPROPERTY(EditAnywhere, meta = (Categories = "Message"))
	FGameplayTag MessageChannelTag;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UEditableText> InputText;

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SuperviseText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> MenuBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> MenuText1;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> MenuText2;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> MenuText3;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> MenuText4;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> MenuText5;


	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> EscapeScreen;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> EscapeGuideTxt;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> EscapeKeyTxt;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> MapScreen;

private:
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> ASC;
	FGameplayMessageListenerHandle MessageListenerHandle;

	float TypingDelta = 0.1f;

	bool EscapeMode;
	bool TutoMode;
};
