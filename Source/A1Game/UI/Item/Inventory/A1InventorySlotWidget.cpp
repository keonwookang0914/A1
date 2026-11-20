// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#include "A1InventorySlotWidget.h"

#include "Components/SizeBox.h"
#include "Data/A1UIData.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1InventorySlotWidget)

UA1InventorySlotWidget::UA1InventorySlotWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UA1InventorySlotWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	FIntPoint UnitInventorySlotSize = UA1UIData::Get().UnitInventorySlotSize;
	SizeBox_Root->SetWidthOverride(UnitInventorySlotSize.X);
	SizeBox_Root->SetHeightOverride(UnitInventorySlotSize.Y);
}
