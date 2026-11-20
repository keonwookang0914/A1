// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "Interaction/Abilities/A1GameplayAbility_Interact_Entry.h"

#include "Actors/A1EquipmentBase.h"
#include "Actors/A1StorageEntryBase.h"
#include "Character/LyraCharacter.h"
#include "Item/Managers/A1EquipManagerComponent.h"
#include "Item/Managers/A1EquipmentManagerComponent.h"
#include "Player/LyraPlayerController.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1GameplayAbility_Interact_Entry)

UA1GameplayAbility_Interact_Entry::UA1GameplayAbility_Interact_Entry(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UA1GameplayAbility_Interact_Entry::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (TriggerEventData == nullptr || bInitialized == false)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	if (HasAuthority(&CurrentActivationInfo) == false)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	AA1StorageEntryBase* EntryActor = Cast<AA1StorageEntryBase>(InteractableActor);
	if (EntryActor == nullptr)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	if (EntryActor->GetItemEntryState() == EItemEntryState::Exist)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	ALyraCharacter* Character = Cast<ALyraCharacter>(GetAvatarActorFromActorInfo());
	if (Character == nullptr)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	UA1EquipManagerComponent* EquipManager = Character->FindComponentByClass<UA1EquipManagerComponent>();
	UA1EquipmentManagerComponent* EquipmentManager = Character->FindComponentByClass<UA1EquipmentManagerComponent>();

	if (!EquipManager || !EquipmentManager)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	EMainHandState CurrentMainHand = EquipManager->GetCurrentMainHand();
	EEquipmentSlotType MainHandSlot = EquipManager->ConvertToEquipmentSlotType(CurrentMainHand);

	EEquipmentSlotType SlotToCheck = EEquipmentSlotType::Count;

	if (EquipManager->GetEquippedActor(EEquipmentSlotType::TwoHand) != nullptr)
	{
		SlotToCheck = EEquipmentSlotType::TwoHand;
	}
	else if (EquipManager->GetEquippedActor(MainHandSlot) != nullptr)
	{
		SlotToCheck = MainHandSlot;
	}

	if (SlotToCheck == EEquipmentSlotType::Count)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	TArray<FA1EquipEntry>& AllEntries = EquipManager->GetAllEntries();

	FA1EquipEntry& Entry = AllEntries[(int32)SlotToCheck];

	AA1EquipmentBase* CachedItem = Entry.GetEquipmentActor();

	if (!IsValid(CachedItem))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	int32 ItemTemplateID = CachedItem->GetTemplateID();
	EItemRarity ItemRarity = CachedItem->GetItemRarity();

	EquipmentManager->RemoveEquipment_Unsafe(SlotToCheck, 1);

	EquipManager->ChangeEquipState(SlotToCheck, false);

	EquipManager->CanInteract();
	
	EntryActor->SetItemTransform(ItemTemplateID, ItemRarity, SlotToCheck);

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
