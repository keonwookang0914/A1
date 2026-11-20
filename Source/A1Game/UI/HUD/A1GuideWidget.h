// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "A1Define.h"
#include "Blueprint/UserWidget.h"
#include "A1GuideWidget.generated.h"

class UTextBlock;
class UA1EquipmentManagerComponent;
class UA1EquipManagerComponent;
class UA1ItemInstance;

UCLASS()
class A1GAME_API UA1GuideWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UA1GuideWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void TryInitPawn();
	void OnEquipmentEntryChanged(EEquipmentSlotType EquipmentSlotType, UA1ItemInstance* ItemInstance, int32 ItemCount);
	void OnMainHandChanged(EMainHandState NewEquipState);
	void RenewGuide(UA1ItemInstance* ItemInstance);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TxtGrab;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TxtLMB;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TxtRMB;

private:
	UPROPERTY()
	TObjectPtr<UA1EquipmentManagerComponent> EquipmentManager;

	UPROPERTY()
	TObjectPtr<UA1EquipManagerComponent> EquipManager;

private:
	FDelegateHandle EntryChangedDelegateHandle;
	FDelegateHandle ChangeHandDelegateHandle;
};
