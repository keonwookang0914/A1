// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "A1Define.h"
#include "UI/Item/A1ItemEntryWidget.h"
#include "A1EquipmentEntryWidget.generated.h"

class UA1ItemInstance;
class UA1EquipmentManagerComponent;

UCLASS()
class A1GAME_API UA1EquipmentEntryWidget : public UA1ItemEntryWidget
{
	GENERATED_BODY()

public:
	UA1EquipmentEntryWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	void Init(UA1ItemInstance* InItemInstance, int32 InItemCount, EEquipmentSlotType InEquipmentSlotType, UA1EquipmentManagerComponent* InEquipmentManager);

protected:
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

protected:

protected:
	UPROPERTY()
	TObjectPtr<UA1EquipmentManagerComponent> EquipmentManager;

	EEquipmentSlotType EquipmentSlotType = EEquipmentSlotType::Count;
};
