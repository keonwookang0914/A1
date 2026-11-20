// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#include "A1HoverWidget.h"

#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1HoverWidget)

UA1HoverWidget::UA1HoverWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UA1HoverWidget::SetPosition(const FVector2D& AbsolutePosition)
{
	HorizontalBox_Hovers->ForceLayoutPrepass();
	FVector2D HoversWidgetSize = HorizontalBox_Hovers->GetDesiredSize();

	FVector2D CanvasWidgetSize = CanvasPanel_Root->GetCachedGeometry().GetLocalSize();
	if (CanvasWidgetSize.IsZero())
		return;

	FVector2D Margin = FVector2D(10.f, 5.0f);
	FVector2D MouseWidgetPos = CanvasPanel_Root->GetCachedGeometry().AbsoluteToLocal(AbsolutePosition);
	FVector2D HoverWidgetStartPos = MouseWidgetPos + Margin;
	FVector2D HoverWidgetEndPos = HoverWidgetStartPos + HoversWidgetSize;

	FVector2D OutSize = (HoverWidgetEndPos - CanvasWidgetSize);
	if (OutSize.X > 0.f)
	{
		HoverWidgetStartPos.X -= OutSize.X;
	}
	if (OutSize.Y > 0.f)
	{
		HoverWidgetStartPos.Y -= OutSize.Y;
	}

	if (UCanvasPanelSlot* CanvasPanelSlot = Cast<UCanvasPanelSlot>(HorizontalBox_Hovers->Slot))
	{
		CanvasPanelSlot->SetPosition(HoverWidgetStartPos);
	}
}
