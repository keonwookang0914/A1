// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "UI/HUD/A1GuideWidget.h"

#include "Character/LyraCharacter.h"
#include "Components/TextBlock.h"
#include "Data/A1GuideData.h"
#include "Item/Managers/A1EquipManagerComponent.h"
#include "Item/Managers/A1EquipmentManagerComponent.h"
#include "Item/A1ItemInstance.h"
#include "Item/Fragments/A1ItemFragment_Equipable.h"
#include "Item/Fragments/A1ItemFragment_Equipable_Weapon.h"
#include "Item/Fragments/A1ItemFragment_Equipable_Utility.h"

UA1GuideWidget::UA1GuideWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UA1GuideWidget::NativeConstruct()
{
	Super::NativeConstruct();

	GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UA1GuideWidget::TryInitPawn);
}

void UA1GuideWidget::NativeDestruct()
{
	if (EquipmentManager)
	{
		EquipmentManager->OnEquipmentEntryChanged.Remove(EntryChangedDelegateHandle);
		EntryChangedDelegateHandle.Reset();
	}

	if (EquipManager)
	{
		EquipManager->OnMainHandChanged.Remove(ChangeHandDelegateHandle);
		ChangeHandDelegateHandle.Reset();
	}

	Super::NativeDestruct();
}

void UA1GuideWidget::TryInitPawn()
{

	ALyraCharacter* LyraCharacter = Cast<ALyraCharacter>(GetOwningPlayerPawn());
	if (LyraCharacter != nullptr)
	{
		EquipManager = LyraCharacter->GetComponentByClass<UA1EquipManagerComponent>();
		EquipmentManager = LyraCharacter->GetComponentByClass<UA1EquipmentManagerComponent>();
		if (EquipManager == nullptr || EquipmentManager == nullptr)
		{
			GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UA1GuideWidget::TryInitPawn);
			return;
		}

		EntryChangedDelegateHandle = EquipmentManager->OnEquipmentEntryChanged.AddUObject(this, &ThisClass::OnEquipmentEntryChanged);
		ChangeHandDelegateHandle = EquipManager->OnMainHandChanged.AddUObject(this, &ThisClass::OnMainHandChanged);
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UA1GuideWidget::TryInitPawn);
	}
}

void UA1GuideWidget::OnEquipmentEntryChanged(EEquipmentSlotType EquipmentSlotType, UA1ItemInstance* ItemInstance, int32 ItemCount)
{
	EMainHandState CurrentMainHand = EquipManager->GetCurrentMainHand();
	if (EquipmentSlotType!=EEquipmentSlotType::TwoHand && EquipmentSlotType != EquipManager->ConvertToEquipmentSlotType(CurrentMainHand))
		return;

	RenewGuide(ItemInstance);
}

void UA1GuideWidget::OnMainHandChanged(EMainHandState NewEquipState)
{
	UA1ItemInstance* ItemInstance = EquipManager->GetEquippedItemInstance(EquipManager->ConvertToEquipmentSlotType(NewEquipState));
	RenewGuide(ItemInstance);
}

void UA1GuideWidget::RenewGuide(UA1ItemInstance* ItemInstance)
{
	TxtLMB->SetText(FText::FromString(""));
	TxtRMB->SetText(FText::FromString(""));

	const FGuideTextSet& GrabTextSet = UA1GuideData::Get().GetTextSetByLabel("Item");
	if (ItemInstance == nullptr)
	{
		TxtGrab->SetText(GrabTextSet.TextEntries[0]);
	}
	else
	{
		TxtGrab->SetText(GrabTextSet.TextEntries[1]);

		const UA1ItemFragment_Equipable* EquippableFragment = ItemInstance->FindFragmentByClass<UA1ItemFragment_Equipable>();
		if (EquippableFragment == nullptr)
			return;

		FText ItemName = FText::FromString("");
		if (EquippableFragment->EquipmentType == EEquipmentType::Weapon)
		{
			const UA1ItemFragment_Equipable_Weapon* ItemFragment = ItemInstance->FindFragmentByClass<UA1ItemFragment_Equipable_Weapon>();
			const UEnum* EnumPtr = StaticEnum<EWeaponType>();
			ItemName = EnumPtr->GetDisplayNameTextByValue((int64)ItemFragment->WeaponType);
		}
		else if (EquippableFragment->EquipmentType == EEquipmentType::Utility)
		{
			const UA1ItemFragment_Equipable_Utility* ItemFragment = ItemInstance->FindFragmentByClass<UA1ItemFragment_Equipable_Utility>();
			const UEnum* EnumPtr = StaticEnum<EUtilityType>();
			ItemName = EnumPtr->GetDisplayNameTextByValue((int64)ItemFragment->UtilityType);
		}

		// 아이템에 맞는 GuideTextSet 가져오기
		const FGuideTextSet& GuideTextSet = UA1GuideData::Get().GetTextSetByLabel(FName(*ItemName.ToString()));
		if (GuideTextSet.TextEntries.IsEmpty())
			return;
		

		TxtLMB->SetText(GuideTextSet.TextEntries[0]);

		if (GuideTextSet.TextEntries.Num() > 1)
			TxtRMB->SetText(GuideTextSet.TextEntries[1]);
	}
}
