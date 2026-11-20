// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "A1Define.h"
#include "Blueprint/DragDropOperation.h"
#include "A1ItemDragDrop.generated.h"

class UA1InventoryManagerComponent;
class UA1EquipmentManagerComponent;
class UA1ItemEntryWidget;
class UA1ItemInstance;

UCLASS()
class A1GAME_API UA1ItemDragDrop : public UDragDropOperation
{
	GENERATED_BODY()

public:
	UA1ItemDragDrop(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
public:
	UPROPERTY()
	TObjectPtr<UA1InventoryManagerComponent> FromInventoryManager;

	FIntPoint FromItemSlotPos = FIntPoint::ZeroValue;

public:
	UPROPERTY()
	TObjectPtr<UA1EquipmentManagerComponent> FromEquipmentManager;

	EEquipmentSlotType FromEquipmentSlotType = EEquipmentSlotType::Count;

public:
	UPROPERTY()
	TObjectPtr<UA1InventoryManagerComponent> ToInventoryManager;

	FIntPoint ToItemSlotPos = FIntPoint::ZeroValue;

public:
	UPROPERTY()
	TObjectPtr<UA1EquipmentManagerComponent> ToEquipmentManager;

	EEquipmentSlotType ToEquipmentSlotType = EEquipmentSlotType::Count;

public:
	UPROPERTY()
	TObjectPtr<UA1ItemEntryWidget> FromEntryWidget;

	UPROPERTY()
	TObjectPtr<UA1ItemInstance> FromItemInstance;

	FVector2D DeltaWidgetPos = FVector2D::ZeroVector;
};
