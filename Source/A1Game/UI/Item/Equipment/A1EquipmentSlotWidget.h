// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "A1Define.h"
#include "Blueprint/UserWidget.h"
#include "A1EquipmentSlotWidget.generated.h"

class UImage;
class UOverlay;
class UTexture2D;
class UA1ItemInstance;
class UA1EquipmentEntryWidget;
class UA1EquipmentManagerComponent;

UCLASS()
class A1GAME_API UA1EquipmentSlotWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UA1EquipmentSlotWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	void Init(EEquipmentSlotType InItemSlotType, UA1EquipmentManagerComponent* InEquipmentManager);

protected:
	virtual void NativePreConstruct() override;
	virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void OnDragEnded();

public:
	void OnEquipmentEntryChange(EEquipmentSlotType InItemHandType, UA1ItemInstance* InItemInstance, int32 InItemCount);

protected:
	UPROPERTY()
	TObjectPtr<UA1EquipmentManagerComponent> EquipmentManager;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UTexture2D> BaseIconTexture;

private:
	UPROPERTY()
	TObjectPtr<UA1EquipmentEntryWidget> EntryWidget;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UOverlay> Overlay_Entry;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_BaseIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_Red;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_Green;

private:
	EEquipmentSlotType ItemSlotType = EEquipmentSlotType::Count;
	bool bAlreadyHovered = false;
};
