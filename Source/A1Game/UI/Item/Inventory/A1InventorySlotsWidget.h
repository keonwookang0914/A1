// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.
#pragma once

#include "Blueprint/UserWidget.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "A1InventorySlotsWidget.generated.h"

class UTextBlock;
class UOverlay;
class UUniformGridPanel;
class UCanvasPanel;
class UA1InventorySlotWidget;
class UA1InventoryEntryWidget;
class UA1InventoryValidWidget;
class UA1InventoryManagerComponent;
class UA1ItemInstance;

USTRUCT(BlueprintType)
struct FInventoryInitializeMessage
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UA1InventoryManagerComponent> InventoryManager;
};

UCLASS()
class A1GAME_API UA1InventorySlotsWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UA1InventorySlotsWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation);

private:
	void ConstructUI(FGameplayTag Channel, const FInventoryInitializeMessage& Message);
	void DestructUI();

	void ResetValidSlots();
	void FinishDrag();
	void OnInventoryEntryChanged(const FIntPoint& InItemSlotPos, UA1ItemInstance* InItemInstance, int32 InItemCount);

public:
	UA1InventoryManagerComponent* GetInventoryManager() const { return InventoryManager; }
	const FGeometry& GetSlotContainerGeometry() const;
	void SetHiddenClickedWidget(int InSlotPos);

public:
	UPROPERTY(EditAnywhere, meta = (Categories = "Message"))
	FGameplayTag MessageChannelTag;

private:
	UPROPERTY()
	TArray<TObjectPtr<UA1InventorySlotWidget>> SlotWidgets;

	UPROPERTY()
	TArray<TObjectPtr<UA1InventoryEntryWidget>> EntryWidgets;

	UPROPERTY()
	TArray<TObjectPtr<UA1InventoryValidWidget>> ValidWidgets;

	UPROPERTY()
	TObjectPtr<UA1InventoryManagerComponent> InventoryManager;

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOverlay> Overlay_Slots;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUniformGridPanel> GridPanel_Slots;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> CanvasPanel_Entries;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUniformGridPanel> GridPanel_ValidSlots;

private:
	FDelegateHandle EntryChangedDelegateHandle;
	FIntPoint PrevDragOverSlotPos = FIntPoint(-1, -1);
	FGameplayMessageListenerHandle MessageListenerHandle;
};
