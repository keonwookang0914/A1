// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#include "AbilitySystem/Abilities/Docker/A1GameplayAbility_DropItem.h"

#include "A1GameplayTags.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "Character/LyraCharacter.h"
#include "Item/Managers/A1EquipManagerComponent.h"
#include "Item/Managers/A1EquipmentManagerComponent.h"
#include "Item/Managers/A1ItemManagerComponent.h"
#include "Player/LyraPlayerController.h"

UA1GameplayAbility_DropItem::UA1GameplayAbility_DropItem(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bServerRespectsRemoteAbilityCancellation = true;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ClientOrServer;
	ActivationGroup = ELyraAbilityActivationGroup::Independent;

	AbilityTags.AddTag(A1GameplayTags::Ability_DropItem);
}

void UA1GameplayAbility_DropItem::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ULyraAbilitySystemComponent* SourceASC = GetLyraAbilitySystemComponentFromActorInfo();
	if (SourceASC == nullptr)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	FGameplayTag CheckToWidget = FGameplayTag::RequestGameplayTag("GameplayEvent.Inventory.Open");
	FGameplayTag CheckToInteract = FGameplayTag::RequestGameplayTag("Status.TryInteract");
	bool bActivateWidget = SourceASC->HasMatchingGameplayTag(CheckToWidget);
	bool bTryInteract = SourceASC->HasMatchingGameplayTag(CheckToInteract);

	// 위젯이 켜진 상태가 아니고, Interact가 가능한 상태(손에 아이템이 없는 상태)
	if (bActivateWidget == false && bTryInteract)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}


	ALyraPlayerController* LyraPlayerController = GetLyraPlayerControllerFromActorInfo();
	UA1ItemManagerComponent* ItemManager = LyraPlayerController->FindComponentByClass<UA1ItemManagerComponent>();
	if (LyraPlayerController == nullptr || ItemManager == nullptr)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	ItemManager->Server_DropItem(bActivateWidget);

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

bool UA1GameplayAbility_DropItem::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
		return false;
	

	
	return true;
}
