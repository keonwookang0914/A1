// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#include "A1GameplayAbility_ChangeMainHand.h"

#include "A1GameplayTags.h"
#include "A1LogChannels.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Character/LyraCharacter.h"
#include "Item/Managers/A1EquipManagerComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1GameplayAbility_ChangeMainHand)

UA1GameplayAbility_ChangeMainHand::UA1GameplayAbility_ChangeMainHand(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bServerRespectsRemoteAbilityCancellation = true;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ClientOrServer;
	ActivationGroup = ELyraAbilityActivationGroup::Independent;
	
	AbilityTags.AddTag(A1GameplayTags::Ability_ChangeHand);
	ActivationBlockedTags.AddTag(A1GameplayTags::GameplayEvent_Inventory_Open);
	ActivationBlockedTags.AddTag(A1GameplayTags::Status_Attack);
	ActivationBlockedTags.AddTag(A1GameplayTags::Status_ActiveUtility);
}

void UA1GameplayAbility_ChangeMainHand::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
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

	EquipManager->ChangeMainHand();
	BlockedAnotherHandAbility(EquipManager->GetCurrentMainHand());
	EquipManager->CanInteract();

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UA1GameplayAbility_ChangeMainHand::BlockedAnotherHandAbility(EMainHandState CurrentState)
{
	FGameplayTagContainer RemooveTagContainer;
	FGameplayTagContainer AddTagContainer;

	switch (CurrentState)
	{
	case EMainHandState::Left:
		AddTagContainer.AddTag(A1GameplayTags::Status_MainHand_Left);
		RemooveTagContainer.AddTag(A1GameplayTags::Status_MainHand_Right);
		break;
	case EMainHandState::Right:
		AddTagContainer.AddTag(A1GameplayTags::Status_MainHand_Right);
		RemooveTagContainer.AddTag(A1GameplayTags::Status_MainHand_Left);
		break;
	}

	UAbilitySystemBlueprintLibrary::RemoveLooseGameplayTags(GetAvatarActorFromActorInfo(), RemooveTagContainer, true);
	UAbilitySystemBlueprintLibrary::AddLooseGameplayTags(GetAvatarActorFromActorInfo(), AddTagContainer, true);
}