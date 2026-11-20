#pragma once

#include "A1Define.h"
#include "Blueprint/UserWidget.h"
#include "Item/A1ItemInstance.h"
#include "A1WeaponSlotWidget.generated.h"

class UImage;
class UOverlay;
class UTextBlock;
class UCommonVisibilitySwitcher;
class UA1EquipManagerComponent;
class UA1EquipmentManagerComponent;

UCLASS()
class UA1WeaponSlotWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UA1WeaponSlotWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void OnEquipmentEntryChanged(EEquipmentSlotType EquipmentSlotType, UA1ItemInstance* ItemInstance, int32 ItemCount);
	void OnMainHandChanged(EMainHandState NewEquipState);
	void TryInitPawn();
	void DisplayBullet(int count);
	
public:
	EMainHandState ChoosedItemSlot = EMainHandState::Count;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TObjectPtr<UImage>> Bullets;

private:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UCommonVisibilitySwitcher> Switcher_Slots;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Image_SlotBothHand;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Image_SlotLeft;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> Image_SlotRight;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_OverlayLeft;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_OverlayRight;



//private:
//	UPROPERTY(meta=(BindWidgetAnim), Transient)
//	TObjectPtr<UWidgetAnimation> Animation_ExpandSlot;
//	
//	UPROPERTY(meta=(BindWidgetAnim), Transient)
//	TObjectPtr<UWidgetAnimation> Animation_ShowCrossLine;

private:
	UPROPERTY()
	TObjectPtr<UA1EquipmentManagerComponent> EquipmentManager;
	
	UPROPERTY()
	TObjectPtr<UA1EquipManagerComponent> EquipManager;
	
private:
	FDelegateHandle EntryChangedDelegateHandle;
	FDelegateHandle MainHandChangedDelegateHandle;
};
