#include "A1WeaponSlotWidget.h"

#include "CommonVisibilitySwitcher.h"
#include "Animation/UMGSequencePlayer.h"
#include "Actors/A1GunBase.h"
#include "Character/LyraCharacter.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Data/A1ItemData.h"
#include "Item/A1ItemTemplate.h"
#include "Item/Managers/A1EquipManagerComponent.h"
#include "Item/Managers/A1EquipmentManagerComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1WeaponSlotWidget)

UA1WeaponSlotWidget::UA1WeaponSlotWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
    
}

void UA1WeaponSlotWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
}

void UA1WeaponSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UA1WeaponSlotWidget::TryInitPawn);
}

void UA1WeaponSlotWidget::TryInitPawn()
{
	ALyraCharacter* LyraCharacter = Cast<ALyraCharacter>(GetOwningPlayerPawn());
	if (LyraCharacter != nullptr)
	{
		LyraCharacter->OnGunEquipped.AddUObject(this, &ThisClass::DisplayBullet);
		DisplayBullet(LyraCharacter->bullets);

		EquipManager = LyraCharacter->GetComponentByClass<UA1EquipManagerComponent>();
		EquipmentManager = LyraCharacter->GetComponentByClass<UA1EquipmentManagerComponent>();
		if (EquipManager == nullptr || EquipmentManager == nullptr)
		{
			GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UA1WeaponSlotWidget::TryInitPawn);
			return;
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

		EMainHandState CurrentMainHand = EquipManager->GetCurrentMainHand();
		OnMainHandChanged(CurrentMainHand);
		MainHandChangedDelegateHandle = EquipManager->OnMainHandChanged.AddUObject(this, &ThisClass::OnMainHandChanged);
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UA1WeaponSlotWidget::TryInitPawn);
	}
}

void UA1WeaponSlotWidget::DisplayBullet(int count)
{
	for (int i = 0; i < 6; i++)
	{
		if(i<count)
			Bullets[i]->SetVisibility(ESlateVisibility::Visible);
		else
			Bullets[i]->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UA1WeaponSlotWidget::NativeDestruct()
{
	if (EquipmentManager)
	{
		EquipmentManager->OnEquipmentEntryChanged.Remove(EntryChangedDelegateHandle);
		EntryChangedDelegateHandle.Reset();
	}

	if (EquipManager)
	{
		EquipManager->OnMainHandChanged.Remove(MainHandChangedDelegateHandle);
		MainHandChangedDelegateHandle.Reset();
	}
	
	Super::NativeDestruct();
}

void UA1WeaponSlotWidget::OnEquipmentEntryChanged(EEquipmentSlotType EquipmentSlotType, UA1ItemInstance* ItemInstance, int32 ItemCount)
{
	if (EquipmentSlotType == EEquipmentSlotType::LeftHand)
	{
		if (ItemInstance)
		{
			const UA1ItemTemplate& ItemTemplate = UA1ItemData::Get().FindItemTemplateByID(ItemInstance->GetItemTemplateID());
			Image_SlotLeft->SetBrushFromTexture(ItemTemplate.IconTexture, true);
			Image_SlotLeft->SetVisibility(ESlateVisibility::HitTestInvisible);

			if (Switcher_Slots->GetActiveWidgetIndex() != 1)
			{
				//PlayAnimationForward(Animation_ShowCrossLine);
				Switcher_Slots->SetActiveWidgetIndex(1);
			}
		}
		else
		{
			Image_SlotLeft->SetVisibility(ESlateVisibility::Hidden);
		}
	}
	else if (EquipmentSlotType == EEquipmentSlotType::RightHand)
	{
		if (ItemInstance)
		{
			const UA1ItemTemplate& ItemTemplate = UA1ItemData::Get().FindItemTemplateByID(ItemInstance->GetItemTemplateID());
			Image_SlotRight->SetBrushFromTexture(ItemTemplate.IconTexture, true);
			Image_SlotRight->SetVisibility(ESlateVisibility::HitTestInvisible);

			if (Switcher_Slots->GetActiveWidgetIndex() != 1)
			{
				//PlayAnimationForward(Animation_ShowCrossLine);
				Switcher_Slots->SetActiveWidgetIndex(1);
			}
		}
		else
		{
			Image_SlotRight->SetVisibility(ESlateVisibility::Hidden);
		}
	}
	else if (EquipmentSlotType == EEquipmentSlotType::TwoHand)
	{
		if (ItemInstance)
		{
			const UA1ItemTemplate& ItemTemplate = UA1ItemData::Get().FindItemTemplateByID(ItemInstance->GetItemTemplateID());
			Image_SlotBothHand->SetBrushFromTexture(ItemTemplate.IconTexture, true);
			Image_SlotBothHand->SetVisibility(ESlateVisibility::HitTestInvisible);

			if (Switcher_Slots->GetActiveWidgetIndex() != 0)
			{
				//PlayAnimationReverse(Animation_ShowCrossLine);
				Switcher_Slots->SetActiveWidgetIndex(0);
			}
		}
		else
		{
			Image_SlotBothHand->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void UA1WeaponSlotWidget::OnMainHandChanged(EMainHandState NewState)
{
	if (ChoosedItemSlot == NewState)
		return;

	ChoosedItemSlot = NewState;
	switch (ChoosedItemSlot)
	{
	case EMainHandState::Left:
		Image_OverlayLeft->SetVisibility(ESlateVisibility::Visible);
		Image_OverlayRight->SetVisibility(ESlateVisibility::Hidden);
		break;
	case EMainHandState::Right:
		Image_OverlayLeft->SetVisibility(ESlateVisibility::Hidden);
		Image_OverlayRight->SetVisibility(ESlateVisibility::Visible);
		break;
	}
}
