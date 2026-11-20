// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#include "A1InventorySlotsWidget.h"

#include "A1InventorySlotWidget.h"
#include "A1InventoryEntryWidget.h"
#include "A1InventoryValidWidget.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Overlay.h"
#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "Data/A1ItemData.h"
#include "Data/A1UIData.h"
#include "Item/A1ItemInstance.h"
#include "Item/A1ItemTemplate.h"
#include "Item/Managers/A1EquipmentManagerComponent.h"
#include "Item/Managers/A1InventoryManagerComponent.h"
#include "Item/Managers/A1ItemManagerComponent.h"
#include "UI/Item/A1ItemDragDrop.h"
#include "UI/Item/Equipment/A1EquipmentEntryWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1InventorySlotsWidget)

UA1InventorySlotsWidget::UA1InventorySlotsWidget(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

void UA1InventorySlotsWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
}

void UA1InventorySlotsWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
	MessageListenerHandle = MessageSubsystem.RegisterListener(MessageChannelTag, this, &ThisClass::ConstructUI);
}

void UA1InventorySlotsWidget::NativeDestruct()
{
	DestructUI();

	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
	MessageSubsystem.UnregisterListener(MessageListenerHandle);

	Super::NativeDestruct();
}

bool UA1InventorySlotsWidget::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragOver(InGeometry, InDragDropEvent, InOperation);

	UA1ItemDragDrop* DragDrop = Cast<UA1ItemDragDrop>(InOperation);
	if (DragDrop == nullptr)
		return false;

	FIntPoint UnitInventorySlotSize = UA1UIData::Get().UnitInventorySlotSize;

	FVector2D MouseWidgetPos = GetSlotContainerGeometry().AbsoluteToLocal(InDragDropEvent.GetScreenSpacePosition());
	FVector2D ToWidgetPos = MouseWidgetPos - DragDrop->DeltaWidgetPos;
	FIntPoint ToItemSlotPos = FIntPoint(ToWidgetPos.X / UnitInventorySlotSize.X, ToWidgetPos.Y / UnitInventorySlotSize.Y);

	if (PrevDragOverSlotPos == ToItemSlotPos)
		return true;

	PrevDragOverSlotPos = ToItemSlotPos;

	UA1ItemEntryWidget* FromEntryWidget = DragDrop->FromEntryWidget;
	UA1ItemInstance* FromItemInstance = FromEntryWidget->GetItemInstance();
	if (FromItemInstance == nullptr)
		return false;

	const UA1ItemTemplate& FromItemTemplate = UA1ItemData::Get().FindItemTemplateByID(FromItemInstance->GetItemTemplateID());
	const FIntPoint& FromItemSlotCount = FromItemTemplate.SlotCount;

	int32 MovableCount = 0;
	if (UA1InventoryManagerComponent* FromInventoryManager = DragDrop->FromInventoryManager)
	{
		MovableCount = InventoryManager->CanMoveOrMergeItem(FromInventoryManager, DragDrop->FromItemSlotPos, ToItemSlotPos);
		FromInventoryManager->ClickedIndex = ToItemSlotPos;
	}
	else if (UA1EquipmentManagerComponent* FromEquipmentManager = DragDrop->FromEquipmentManager)
	{
		MovableCount = InventoryManager->CanMoveOrMergeItem(FromEquipmentManager, DragDrop->FromEquipmentSlotType, ToItemSlotPos);
	}

	ResetValidSlots();

	const FIntPoint& InventorySlotCount = InventoryManager->GetInventorySlotCount();

	const FIntPoint StartSlotPos = FIntPoint(FMath::Max(0, ToItemSlotPos.X), FMath::Max(0, ToItemSlotPos.Y));
	const FIntPoint EndSlotPos = FIntPoint(FMath::Min(ToItemSlotPos.X + FromItemSlotCount.X, InventorySlotCount.X),
		FMath::Min(ToItemSlotPos.Y + FromItemSlotCount.Y, InventorySlotCount.Y));

	ESlotState SlotState = (MovableCount > 0) ? ESlotState::Valid : ESlotState::Invalid;
	for (int32 y = StartSlotPos.Y; y < EndSlotPos.Y; y++)
	{
		for (int32 x = StartSlotPos.X; x < EndSlotPos.X; x++)
		{
			int32 Index = y * InventorySlotCount.X + x;
			if (UA1InventoryValidWidget* ValidWidget = ValidWidgets[Index])
			{
				ValidWidget->ChangeSlotState(SlotState);
			}
		}
	}
	return true;
}

void UA1InventorySlotsWidget::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragLeave(InDragDropEvent, InOperation);

	FinishDrag();
}

bool UA1InventorySlotsWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);

	FinishDrag();

	//Tag를 가지고 있다면 Drag 중이라는 태그 제거
	FGameplayTag TagToCheck = FGameplayTag::RequestGameplayTag(FName("GameplayEvent.Inventory.Drag"));
	bool HasTag = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwningPlayerPawn())->HasMatchingGameplayTag(TagToCheck);
	if (HasTag)
	{
		FGameplayTagContainer TagContainer;
		TagContainer.AddTag(TagToCheck);
		UAbilitySystemBlueprintLibrary::RemoveLooseGameplayTags(GetOwningPlayerPawn(), TagContainer, true);
	}

	FIntPoint UnitInventorySlotSize = UA1UIData::Get().UnitInventorySlotSize;

	UA1ItemDragDrop* DragDrop = Cast<UA1ItemDragDrop>(InOperation);
	check(DragDrop);

	UA1ItemEntryWidget* FromEntryWidget = DragDrop->FromEntryWidget;
	FromEntryWidget->RefreshWidgetOpacity(true);

	FVector2D MouseWidgetPos = GetSlotContainerGeometry().AbsoluteToLocal(InDragDropEvent.GetScreenSpacePosition());
	FVector2D ToWidgetPos = MouseWidgetPos - DragDrop->DeltaWidgetPos;
	FIntPoint ToItemSlotPos = FIntPoint(ToWidgetPos.X / UnitInventorySlotSize.X, ToWidgetPos.Y / UnitInventorySlotSize.Y);

	DragDrop->ToInventoryManager = InventoryManager;
	DragDrop->ToItemSlotPos = ToItemSlotPos;

	UA1ItemManagerComponent* ItemManager = GetOwningPlayer()->FindComponentByClass<UA1ItemManagerComponent>();
	check(ItemManager);

	if (UA1InventoryManagerComponent* FromInventoryManager = DragDrop->FromInventoryManager)
	{
		ItemManager->Server_InventoryToInventory(FromInventoryManager, DragDrop->FromItemSlotPos, InventoryManager, ToItemSlotPos);
	}
	else if (UA1EquipmentManagerComponent* FromEquipmentManager = DragDrop->FromEquipmentManager)
	{
		ItemManager->Server_EquipmentToInventory(FromEquipmentManager, DragDrop->FromEquipmentSlotType, InventoryManager, ToItemSlotPos);
	}
	return true;
}

void UA1InventorySlotsWidget::ConstructUI(FGameplayTag Channel, const FInventoryInitializeMessage& Message)
{
	if (Message.InventoryManager == nullptr)
		return;

	InventoryManager = Message.InventoryManager;

	const FIntPoint& InventorySlotCount = InventoryManager->GetInventorySlotCount();
	const int32 InventorySlotNum = InventorySlotCount.X * InventorySlotCount.Y;
	SlotWidgets.SetNum(InventorySlotNum);
	ValidWidgets.SetNum(InventorySlotNum);
	EntryWidgets.SetNum(InventorySlotNum);

	for (int32 y = 0; y < InventorySlotCount.Y; y++)
	{
		for (int32 x = 0; x < InventorySlotCount.X; x++)
		{
			TSubclassOf<UA1InventorySlotWidget> SlotWidgetClass = UA1UIData::Get().InventorySlotWidgetClass;
			UA1InventorySlotWidget* SlotWidget = CreateWidget<UA1InventorySlotWidget>(GetOwningPlayer(), SlotWidgetClass);
			SlotWidgets[y * InventorySlotCount.X + x] = SlotWidget;
			GridPanel_Slots->AddChildToUniformGrid(SlotWidget, y, x);

			TSubclassOf<UA1InventoryValidWidget> ValidWidgetClass = UA1UIData::Get().InventoryValidWidgetClass;
			UA1InventoryValidWidget* ValidWidget = CreateWidget<UA1InventoryValidWidget>(GetOwningPlayer(), ValidWidgetClass);
			ValidWidgets[y * InventorySlotCount.X + x] = ValidWidget;
			GridPanel_ValidSlots->AddChildToUniformGrid(ValidWidget, y, x);
		}
	}

	const TArray<FA1InventoryEntry>& Entries = InventoryManager->GetAllEntries();
	for (int32 i = 0 ; i< Entries.Num();i++)
	{
		const FA1InventoryEntry& Entry = Entries[i];
		if (UA1ItemInstance* ItemInstance = Entry.GetItemInstance())
		{
			FIntPoint ItemSlotPos = FIntPoint(i % InventorySlotCount.X, i / InventorySlotCount.X);
			OnInventoryEntryChanged(ItemSlotPos, ItemInstance, Entry.GetItemCount());
		}
	}

	EntryChangedDelegateHandle = InventoryManager->OnInventoryEntryChanged.AddUObject(this, &ThisClass::OnInventoryEntryChanged);
}

void UA1InventorySlotsWidget::DestructUI()
{
	InventoryManager->OnInventoryEntryChanged.Remove(EntryChangedDelegateHandle);
	EntryChangedDelegateHandle.Reset();

	CanvasPanel_Entries->ClearChildren();
	EntryWidgets.Reset();

	GridPanel_ValidSlots->ClearChildren();
	ValidWidgets.Reset();

	GridPanel_Slots->ClearChildren();
	SlotWidgets.Reset();
}

void UA1InventorySlotsWidget::ResetValidSlots()
{
	for (UA1InventoryValidWidget* ValidWidget : ValidWidgets)
	{
		if (ValidWidget)
		{
			ValidWidget->ChangeSlotState(ESlotState::Default);
		}
	}
}

void UA1InventorySlotsWidget::FinishDrag()
{
	ResetValidSlots();
	PrevDragOverSlotPos = FIntPoint(-1, -1);
}

void UA1InventorySlotsWidget::OnInventoryEntryChanged(const FIntPoint& InItemSlotPos, UA1ItemInstance* InItemInstance, int32 InItemCount)
{
	const FIntPoint& InventorySlotCount = InventoryManager->GetInventorySlotCount();
	int32 SlotIndex = InItemSlotPos.Y * InventorySlotCount.X + InItemSlotPos.X;

	if (UA1InventoryEntryWidget* EntryWidget = EntryWidgets[SlotIndex])
	{
		UA1ItemInstance* ItemInstance = EntryWidget->GetItemInstance();
		if (ItemInstance && ItemInstance == InItemInstance)
		{
			EntryWidget->RefreshItemCount(InItemCount);
			return;
		}
		CanvasPanel_Entries->RemoveChild(EntryWidget);
		EntryWidgets[SlotIndex] = nullptr;
	}

	if (InItemInstance)
	{
		FIntPoint UnitInventorySlotSize = UA1UIData::Get().UnitInventorySlotSize;

		TSubclassOf<UA1InventoryEntryWidget> EntryWidgetClass = UA1UIData::Get().InventoryEntryWidgetClass;
		UA1InventoryEntryWidget* EntryWidget = CreateWidget<UA1InventoryEntryWidget>(GetOwningPlayer(), EntryWidgetClass);
		EntryWidgets[SlotIndex] = EntryWidget;

		EntryWidget->Init(this, InItemInstance, InItemCount);
		if (InventoryManager->ClickedIndex == InItemSlotPos)
			EntryWidget->ChangeStateClickedWidget(true);

		UCanvasPanelSlot* CanvasPanelSlot = CanvasPanel_Entries->AddChildToCanvas(EntryWidget);
		CanvasPanelSlot->SetAutoSize(true);
		CanvasPanelSlot->SetPosition(FVector2D(InItemSlotPos.X * UnitInventorySlotSize.X, InItemSlotPos.Y * UnitInventorySlotSize.Y));
	}
}

const FGeometry& UA1InventorySlotsWidget::GetSlotContainerGeometry() const
{
	return Overlay_Slots->GetCachedGeometry();
}

void UA1InventorySlotsWidget::SetHiddenClickedWidget(int InSlotPos)
{
	// TEMP Jerry
	// Range Over를 위한 임시 조치
	if (InSlotPos < 25 && EntryWidgets[InSlotPos])
		EntryWidgets[InSlotPos]->ChangeStateClickedWidget(false);
}