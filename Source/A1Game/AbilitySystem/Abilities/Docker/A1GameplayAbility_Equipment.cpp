#include "A1GameplayAbility_Equipment.h"

#include "A1GameplayTags.h"
#include "AbilitySystemGlobals.h"
#include "Abilities/Async/AbilityAsync_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/A1CombatSet.h"
#include "Actors/A1EquipmentBase.h"
#include "Character/LyraCharacter.h"
#include "Item/A1ItemInstance.h"
#include "Item/Fragments/A1ItemFragment_Equipable_Utility.h"
#include "Item/Fragments/A1ItemFragment_Equipable_Weapon.h"
#include "Item/Managers/A1EquipManagerComponent.h"
#include "System/LyraGameData.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1GameplayAbility_Equipment)

UA1GameplayAbility_Equipment::UA1GameplayAbility_Equipment(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bServerRespectsRemoteAbilityCancellation = false;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ServerOnlyTermination;

	ActivationBlockedTags.AddTag(A1GameplayTags::GameplayEvent_Inventory_Open);
}

void UA1GameplayAbility_Equipment::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	ALyraCharacter* LyraCharacter = Cast<ALyraCharacter>(ActorInfo->AvatarActor.Get());
	if (LyraCharacter == nullptr)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	UA1EquipManagerComponent* EquipManager = LyraCharacter->FindComponentByClass<UA1EquipManagerComponent>();
	if (EquipManager == nullptr)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	UA1ItemInstance* ItemInstance = nullptr;
	
	for (FA1EquipmentInfo& EquipmentInfo : EquipmentInfos)
	{
		switch (EquipmentInfo.EquipmentType)
		{
		case EEquipmentType::Weapon:
		case EEquipmentType::Utility:
			ItemInstance = EquipManager->GetEquippedItemInstance(EquipmentInfo.ItemSlotType);
			EquipmentInfo.EquipmentActor = EquipManager->GetEquippedActor(EquipmentInfo.ItemSlotType);
			break;
		}
		
		if ((ItemInstance == nullptr) || (EquipmentInfo.EquipmentType != EEquipmentType::Armor && EquipmentInfo.EquipmentActor == nullptr))
		{
			CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
			return;
		}
	}
	
	SnapshottedAttackRate = DefaultAttackRate;
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		if (const UA1CombatSet* CombatSet = Cast<UA1CombatSet>(ASC->GetAttributeSet(UA1CombatSet::StaticClass())))
		{
			float AttackSpeedPercent = CombatSet->GetAttackSpeedPercent();
			SnapshottedAttackRate = DefaultAttackRate + (DefaultAttackRate * (AttackSpeedPercent / 100.f));
		}
	}
}

bool UA1GameplayAbility_Equipment::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags) == false)
		return false;

	ALyraCharacter* LyraCharacter = Cast<ALyraCharacter>(ActorInfo->AvatarActor.Get());
	if (LyraCharacter == nullptr)
		return false;

	UA1EquipManagerComponent* EquipManager = LyraCharacter->FindComponentByClass<UA1EquipManagerComponent>();
	if (EquipManager == nullptr)
		return false;
	
	for (const FA1EquipmentInfo& EquipmentInfo : EquipmentInfos)
	{
		if (EquipmentInfo.EquipmentType == EEquipmentType::Count)
			return false;

		if (EquipmentInfo.EquipmentType == EEquipmentType::Weapon)
		{
			UA1ItemInstance* ItemInstance = EquipManager->GetEquippedItemInstance(EquipmentInfo.ItemSlotType);
			if (ItemInstance == nullptr)
				return false;

			const UA1ItemFragment_Equipable_Weapon* WeaponFragment = ItemInstance->FindFragmentByClass<UA1ItemFragment_Equipable_Weapon>();
			if (WeaponFragment == nullptr)
				return false;
	
			if (WeaponFragment->WeaponType != EquipmentInfo.RequiredWeaponType)
				return false;
		}
		else if (EquipmentInfo.EquipmentType == EEquipmentType::Utility)
		{
			UA1ItemInstance* ItemInstance = EquipManager->GetEquippedItemInstance(EquipmentInfo.ItemSlotType);
			if (ItemInstance == nullptr)
				return false;

			const UA1ItemFragment_Equipable_Utility* UtilityFragment = ItemInstance->FindFragmentByClass<UA1ItemFragment_Equipable_Utility>();
			if (UtilityFragment == nullptr)
				return false;

			if (UtilityFragment->UtilityType != EquipmentInfo.RequiredUtilityType)
				return false;
		}
	}

	return true;
}

AA1EquipmentBase* UA1GameplayAbility_Equipment::GetFirstEquipmentActor() const
{
	for (const FA1EquipmentInfo& EquipmentInfo : EquipmentInfos)
	{
		if (AA1EquipmentBase* EquipmentActor = EquipmentInfo.EquipmentActor.Get())
			return EquipmentActor;
	}
	return nullptr;
}

UA1ItemInstance* UA1GameplayAbility_Equipment::GetEquipmentItemInstance(const AA1EquipmentBase* InEquipmentActor) const
{
	if (InEquipmentActor == nullptr)
		return nullptr;
	
	UA1EquipManagerComponent* EquipManager = GetLyraCharacterFromActorInfo()->FindComponentByClass<UA1EquipManagerComponent>();
	if (EquipManager == nullptr)
		return nullptr;
	
	for (const FA1EquipmentInfo& EquipmentInfo : EquipmentInfos)
	{
		if (EquipmentInfo.EquipmentActor != InEquipmentActor)
			continue;
		
		return EquipManager->GetEquippedItemInstance(InEquipmentActor->GetEquipmentSlotType());
	}

	return nullptr;
}

int32 UA1GameplayAbility_Equipment::GetEquipmentStatValue(FGameplayTag InStatTag, const AA1EquipmentBase* InEquipmentActor) const
{
	if (InStatTag.IsValid() == false || InEquipmentActor == nullptr)
		return 0;
	
	if (UA1ItemInstance* ItemInstance = GetEquipmentItemInstance(InEquipmentActor))
		return ItemInstance->GetStackCountByTag(InStatTag);

	check(true);
	
	return 0;
}
