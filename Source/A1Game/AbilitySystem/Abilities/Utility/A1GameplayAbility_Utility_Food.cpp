// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "AbilitySystem/Abilities/Utility/A1GameplayAbility_Utility_Food.h"

#include "A1GameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Actors/A1EquipmentBase.h"
#include "Data/A1ItemData.h"
#include "Item/A1ItemTemplate.h"
#include "Item/A1ItemInstance.h"
#include "Item/Fragments/A1ItemFragment_Equipable_Utility.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Item/Managers/A1EquipmentManagerComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Score/A1ScoreBlueprintFunctionLibrary.h"

UA1GameplayAbility_Utility_Food::UA1GameplayAbility_Utility_Food(const FObjectInitializer& ObjectInitializer)
{
	AbilityTags.AddTag(A1GameplayTags::Ability_Food);
	ActivationOwnedTags.AddTag(A1GameplayTags::Status_Food);
	ActivationOwnedTags.AddTag(A1GameplayTags::Status_ActiveUtility);
	ActivationRequiredTags.AddTag(A1GameplayTags::Status_MainHand_Left);
}

void UA1GameplayAbility_Utility_Food::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (HasAuthority(&CurrentActivationInfo) == false)
		return;

	if (UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("EatFood"), EatMontage, GetSnapshottedAttackRate(), NAME_None, false, 1.f, 0.f, false))
	{
		PlayMontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageFinished);
		PlayMontageTask->ReadyForActivation();
	}
}

void UA1GameplayAbility_Utility_Food::OnMontageFinished()
{
	
	ALyraCharacter* LyraCharacter = Cast<ALyraCharacter>(CurrentActorInfo->AvatarActor.Get());
	UA1EquipmentManagerComponent* EquipmentManager = LyraCharacter->GetComponentByClass<UA1EquipmentManagerComponent>();
	if (LyraCharacter == nullptr || EquipmentManager == nullptr)
		return;

	AA1EquipmentBase* EquipedItem = Cast<AA1EquipmentBase>(GetFirstEquipmentActor());
	const UA1ItemTemplate& ItemTemplate = UA1ItemData::Get().FindItemTemplateByID(EquipedItem->GetTemplateID());
	const UA1ItemFragment_Equipable_Utility* ItemFragment = ItemTemplate.FindFragmentByClass<UA1ItemFragment_Equipable_Utility>();
	if (ItemFragment == nullptr)
		return;

	// Ability 적용
	FGameplayAbilityTargetDataHandle TargetDataHandle = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(LyraCharacter);

	for (TSubclassOf<UGameplayEffect> UtilityEffectClass : ItemFragment->UtilityEffectClasses)
	{
		FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingGameplayEffectSpec(UtilityEffectClass);
		if (EffectSpecHandle.IsValid())
		{
			ApplyGameplayEffectSpecToTarget(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, EffectSpecHandle, TargetDataHandle);
		}
	}

	// 장착 장비 제거
	EquipmentManager->RemoveEquipment_Unsafe(EquipedItem->GetEquipmentSlotType(), 1);

	UA1ScoreBlueprintFunctionLibrary::AddConsumedItems();
	
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	
}
