// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "UI/Item/A1ItemEntryWidget.h"
#include "A1InventoryEntryWidget.generated.h"

class USizeBox;
class UImage;
class UA1ItemInstance;
class UA1InventorySlotsWidget;

UCLASS()
class A1GAME_API UA1InventoryEntryWidget : public UA1ItemEntryWidget
{
	GENERATED_BODY()

public:
	UA1InventoryEntryWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	void Init(UA1InventorySlotsWidget* InSlotsWidget, UA1ItemInstance* InItemInstance, int32 InItemCount);
	void ChangeStateClickedWidget(bool bVisible);

protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;

private:
	UPROPERTY()
	TObjectPtr<UA1InventorySlotsWidget> SlotsWidget;

	FIntPoint CachedFromSlotPos = FIntPoint::ZeroValue;
	FVector2D CachedDeltaWidgetPos = FVector2D::ZeroVector;

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USizeBox> SizeBox_Root;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_Clicked;
};
