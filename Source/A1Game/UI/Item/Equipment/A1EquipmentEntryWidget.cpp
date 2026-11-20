// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#include "A1EquipmentEntryWidget.h"

#include "A1EquipmentSlotsWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Data/A1ItemData.h"
#include "Data/A1UIData.h"
#include "Item/A1ItemInstance.h"
#include "Item/A1ItemTemplate.h"
#include "Item/Managers/A1ItemManagerComponent.h"
#include "UI/Item/A1ItemDragDrop.h"
#include "UI/Item/A1ItemDragWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1EquipmentEntryWidget)

UA1EquipmentEntryWidget::UA1EquipmentEntryWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UA1EquipmentEntryWidget::Init(UA1ItemInstance* InItemInstance, int32 InItemCount, EEquipmentSlotType InEquipmentSlotType, UA1EquipmentManagerComponent* InEquipmentManager)
{
	if (InEquipmentSlotType == EEquipmentSlotType::Count)
		return;

	RefreshUI(InItemInstance, InItemCount);

	EquipmentSlotType = InEquipmentSlotType;
	EquipmentManager = InEquipmentManager;
}

void UA1EquipmentEntryWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	FIntPoint UnitInventorySlotSize = UA1UIData::Get().UnitInventorySlotSize;
	const UA1ItemTemplate& ItemTemplate = UA1ItemData::Get().FindItemTemplateByID(ItemInstance->GetItemTemplateID());

	TSubclassOf<UA1ItemDragWidget> DragWidgetClass = UA1UIData::Get().DragWidgetClass;
	UA1ItemDragWidget* ItemDragWidget = CreateWidget<UA1ItemDragWidget>(GetOwningPlayer(), DragWidgetClass);
	FVector2D DragWidgetSize = FVector2D(ItemTemplate.SlotCount * UnitInventorySlotSize);
	ItemDragWidget->Init(DragWidgetSize, ItemTemplate.IconTexture, ItemCount);

	UA1ItemDragDrop* ItemDragDrop = NewObject<UA1ItemDragDrop>();
	ItemDragDrop->DefaultDragVisual = ItemDragWidget;
	ItemDragDrop->Pivot = EDragPivot::CenterCenter;
	ItemDragDrop->FromEntryWidget = this;
	ItemDragDrop->FromEquipmentManager = EquipmentManager;
	ItemDragDrop->FromEquipmentSlotType = EquipmentSlotType;
	ItemDragDrop->FromItemInstance = ItemInstance;
	ItemDragDrop->DeltaWidgetPos = (DragWidgetSize / 2.f) - (UnitInventorySlotSize / 2.f);
	OutOperation = ItemDragDrop;
}

FReply UA1EquipmentEntryWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	if (Reply.IsEventHandled() == false && UWidgetBlueprintLibrary::IsDragDropping() == false && InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		if (UA1ItemManagerComponent* ItemManager = GetOwningPlayer()->FindComponentByClass<UA1ItemManagerComponent>())
		{
			ItemManager->Server_QuickFromEquipment(EquipmentManager, EquipmentSlotType);
			return FReply::Handled();
		}
	}

	return Reply;
}
