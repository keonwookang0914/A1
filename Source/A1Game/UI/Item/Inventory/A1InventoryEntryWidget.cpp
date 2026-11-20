// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#include "A1InventoryEntryWidget.h"

#include "A1InventorySlotsWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/SizeBox.h"
#include "Components/Image.h"
#include "Data/A1UIData.h"
#include "Data/A1ItemData.h"
#include "Item/A1ItemInstance.h"
#include "Item/A1ItemTemplate.h"
#include "Item/Managers/A1InventoryManagerComponent.h"
#include "Item/Managers/A1ItemManagerComponent.h"
#include "UI/Item/A1ItemDragDrop.h"
#include "UI/Item/A1ItemDragWidget.h"

UA1InventoryEntryWidget::UA1InventoryEntryWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UA1InventoryEntryWidget::Init(UA1InventorySlotsWidget* InSlotsWidget, UA1ItemInstance* InItemInstance, int32 InItemCount)
{
	if (InSlotsWidget == nullptr || InItemInstance == nullptr)
		return;

	SlotsWidget = InSlotsWidget;

	FIntPoint UnitInventorySlotSize = UA1UIData::Get().UnitInventorySlotSize;
	const UA1ItemTemplate& ItemTemplate = UA1ItemData::Get().FindItemTemplateByID(InItemInstance->GetItemTemplateID());

	FVector2D WidgetSize = FVector2D(ItemTemplate.SlotCount.X * UnitInventorySlotSize.X, ItemTemplate.SlotCount.Y * UnitInventorySlotSize.Y);
	SizeBox_Root->SetWidthOverride(WidgetSize.X);
	SizeBox_Root->SetHeightOverride(WidgetSize.Y);

	Image_Clicked->SetVisibility(ESlateVisibility::Hidden);

	RefreshUI(InItemInstance, InItemCount);
}

FReply UA1InventoryEntryWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	FIntPoint UnitInventorySlotSize = UA1UIData::Get().UnitInventorySlotSize;

	FVector2D MouseWidgetPos = SlotsWidget->GetSlotContainerGeometry().AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
	FVector2D ItemWidgetPos = SlotsWidget->GetSlotContainerGeometry().AbsoluteToLocal(InGeometry.LocalToAbsolute(UnitInventorySlotSize / 2.f));
	FIntPoint ItemSlotPos = FIntPoint(ItemWidgetPos.X / UnitInventorySlotSize.X, ItemWidgetPos.Y / UnitInventorySlotSize.Y);

	CachedFromSlotPos = ItemSlotPos;
	CachedDeltaWidgetPos = MouseWidgetPos - ItemWidgetPos;

	if (Reply.IsEventHandled() == false && UWidgetBlueprintLibrary::IsDragDropping() == false)
	{
		UA1ItemManagerComponent* ItemManager = GetOwningPlayer()->FindComponentByClass<UA1ItemManagerComponent>();
		UA1InventoryManagerComponent* FromInventoryManager = SlotsWidget->GetInventoryManager();

		if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
		{
			if (ItemManager && FromInventoryManager)
			{
				ItemManager->Server_QuickFromInventory(FromInventoryManager, ItemSlotPos);
				return FReply::Handled();
			}
		}

		// 좌클릭 시 위젯 클릭
		if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
		{
			if (ItemManager && FromInventoryManager)
			{
				// 이전 clicked widget hidden
				FIntPoint PreviousClickedPos = FromInventoryManager->ClickedIndex;
				if (PreviousClickedPos != NULL)
				{
					int32 SlotIndex = PreviousClickedPos.Y * FromInventoryManager->GetInventorySlotCount().X + PreviousClickedPos.X;
					SlotsWidget->SetHiddenClickedWidget(SlotIndex);
				}

				// Clicked Widget On
				Image_Clicked->SetVisibility(ESlateVisibility::Visible);

				// 선택된 인덱스 전달
				FromInventoryManager->ClickedIndex = ItemSlotPos;

				return FReply::Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
			}
		}
	}

	return Reply;
}

void UA1InventoryEntryWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	FIntPoint UnitInventorySlotSize = UA1UIData::Get().UnitInventorySlotSize;
	const UA1ItemTemplate& ItemTemplate = UA1ItemData::Get().FindItemTemplateByID(ItemInstance->GetItemTemplateID());

	TSubclassOf<UA1ItemDragWidget> DragWidgetClass = UA1UIData::Get().DragWidgetClass;
	UA1ItemDragWidget* DragWidget = CreateWidget<UA1ItemDragWidget>(GetOwningPlayer(), DragWidgetClass);
	FVector2D DragWidgetSize = FVector2D(ItemTemplate.SlotCount.X * UnitInventorySlotSize.X, ItemTemplate.SlotCount.Y * UnitInventorySlotSize.Y);
	DragWidget->Init(DragWidgetSize, ItemTemplate.IconTexture, ItemCount);

	UA1ItemDragDrop* DragDrop = NewObject<UA1ItemDragDrop>();
	DragDrop->DefaultDragVisual = DragWidget;
	DragDrop->Pivot = EDragPivot::TopLeft;
	DragDrop->Offset = -((CachedDeltaWidgetPos + UnitInventorySlotSize / 2.f) / DragWidgetSize);
	DragDrop->FromEntryWidget = this;
	DragDrop->FromInventoryManager = SlotsWidget->GetInventoryManager();
	DragDrop->FromItemSlotPos = CachedFromSlotPos;
	DragDrop->FromItemInstance = ItemInstance;
	DragDrop->DeltaWidgetPos = CachedDeltaWidgetPos;
	OutOperation = DragDrop;
}

void UA1InventoryEntryWidget::ChangeStateClickedWidget(bool bVisible)
{
	if(bVisible)
		Image_Clicked->SetVisibility(ESlateVisibility::Visible);
	else
		Image_Clicked->SetVisibility(ESlateVisibility::Hidden);
}