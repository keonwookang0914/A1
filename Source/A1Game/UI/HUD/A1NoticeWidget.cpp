// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#include "UI/HUD/A1NoticeWidget.h"

#include "Character/LyraCharacter.h"
#include "Components/TextBlock.h"

UA1NoticeWidget::UA1NoticeWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UA1NoticeWidget::NativeConstruct()
{
	Super::NativeConstruct();

	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UA1NoticeWidget::TryInitPawn);

	//ALyraCharacter* LyraCharacter = Cast<ALyraCharacter>(GetOwningPlayerPawn());
	//if (LyraCharacter == nullptr)
	//	return;
	//
	//LyraCharacter->OnNotice.AddUObject(this, &ThisClass::ShowWarning);
}

void UA1NoticeWidget::TryInitPawn()
{
	ALyraCharacter* LyraCharacter = Cast<ALyraCharacter>(GetOwningPlayerPawn());
	if (LyraCharacter != nullptr)
	{
		LyraCharacter->OnNotice.AddUObject(this, &ThisClass::ShowWarning);
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UA1NoticeWidget::TryInitPawn);
	}
}

void UA1NoticeWidget::ShowWarning(FText WarningText)
{
	TxtNotice->SetText(WarningText);
	SetVisibility(ESlateVisibility::Visible);

	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle,
		[this]()
		{
			SetVisibility(ESlateVisibility::Hidden);
		},
		2.0f,
		false
	);
}
