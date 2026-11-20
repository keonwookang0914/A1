// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "A1NoticeWidget.generated.h"

/**
 * 
 */
UCLASS()
class A1GAME_API UA1NoticeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UA1NoticeWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void NativeConstruct() override;

	void TryInitPawn();
	void ShowWarning(FText WarningText);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> TxtNotice;	
};
