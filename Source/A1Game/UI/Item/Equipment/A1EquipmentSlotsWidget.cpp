// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#include "A1EquipmentSlotsWidget.h"

#include "A1EquipmentSlotWidget.h"
#include "Item/Managers/A1EquipManagerComponent.h"
#include "Item/Managers/A1EquipmentManagerComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1EquipmentSlotsWidget)

UA1EquipmentSlotsWidget::UA1EquipmentSlotsWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UA1EquipmentSlotsWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	ItemSlotWidgets = { Widget_Left_Hand, Widget_Right_Hand };
}

void UA1EquipmentSlotsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
	MessageListenerHandle = MessageSubsystem.RegisterListener(MessageChannelTag, this, &ThisClass::ConstructUI);
}

void UA1EquipmentSlotsWidget::NativeDestruct()
{
	DestructUI();

	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
	MessageSubsystem.UnregisterListener(MessageListenerHandle);

	Super::NativeDestruct();
}

void UA1EquipmentSlotsWidget::ConstructUI(FGameplayTag Channel, const FEquipmentInitializeMessage& Message)
{
	if (Message.EquipmentManager == nullptr || Message.EquipManager == nullptr)
		return;

	EquipmentManager = Message.EquipmentManager;
	EquipManager = Message.EquipManager;

	for (int32 i = 0; i < ItemSlotWidgets.Num(); i++)
	{
		ItemSlotWidgets[i]->Init((EEquipmentSlotType)i, EquipmentManager);
	}

	const TArray<FA1EquipmentEntry>& Entries = EquipmentManager->GetAllEntries();
	for (int32 i = 0; i < Entries.Num(); i++)
	{
		const FA1EquipmentEntry& Entry = Entries[i];
		if (UA1ItemInstance* ItemInstance = Entry.GetItemInstance())
		{
			OnEquipmentEntryChanged((EEquipmentSlotType)i, ItemInstance, Entry.GetItemCount());
		}
	}
	EntryChangedDelegateHandle = EquipmentManager->OnEquipmentEntryChanged.AddUObject(this, &ThisClass::OnEquipmentEntryChanged);

	//EquipStateChangedDelegateHandle = EquipManager->OnEquipStateChanged.AddUObject(this, &ThisClass::OnEquipStateChanged);
}

void UA1EquipmentSlotsWidget::DestructUI()
{
	if (EquipmentManager)
	{
		EquipmentManager->OnEquipmentEntryChanged.Remove(EntryChangedDelegateHandle);
		EntryChangedDelegateHandle.Reset();
	}

	// Jerry
	//if (EquipManager)
	//{
	//	EquipManager->OnEquipStateChanged.Remove(EquipStateChangedDelegateHandle);
	//	EquipStateChangedDelegateHandle.Reset();
	//}

	for (UA1EquipmentSlotWidget* SlotItemWidget : ItemSlotWidgets)
	{
		if (SlotItemWidget)
		{
			for (int32 i = 0; i < (int32)EEquipmentSlotType::Count; i++)
			{
				SlotItemWidget->OnEquipmentEntryChange((EEquipmentSlotType)i, nullptr, 0);
			}
		}
	}
}

void UA1EquipmentSlotsWidget::OnEquipmentEntryChanged(EEquipmentSlotType EquipmentSlotType, UA1ItemInstance* ItemInstance, int32 ItemCount)
{
	if (EquipmentSlotType == EEquipmentSlotType::Count)
		return;

	const int32 SlotIndex = (int32)EquipmentSlotType;
	if (ItemSlotWidgets.IsValidIndex(SlotIndex))
	{
		if (UA1EquipmentSlotWidget* ItemSlotWidget = ItemSlotWidgets[SlotIndex])
		{
			ItemSlotWidget->OnEquipmentEntryChange(EquipmentSlotType, ItemInstance, ItemCount);
		}
	}
	else
		UE_LOG(LogTemp, Warning, TEXT("Index Valid: %d"), SlotIndex);
	// TODO Twohand 처리 구문 
}