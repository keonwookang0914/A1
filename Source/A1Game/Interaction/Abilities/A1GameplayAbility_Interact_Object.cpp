#include "Interaction/Abilities/A1GameplayAbility_Interact_Object.h"

#include "A1GameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Actors/A1EquipmentBase.h"
#include "Character/LyraCharacter.h"
#include "Interaction/A1Interactable.h"
#include "Interaction/A1WorldInteractable.h"
#include "Item/Managers/A1EquipManagerComponent.h"
#include "Tasks/A1AbilityTask_WaitForInvalidInteraction.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1GameplayAbility_Interact_Object)

UA1GameplayAbility_Interact_Object::UA1GameplayAbility_Interact_Object(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ActivationPolicy = ELyraAbilityActivationPolicy::Manual;

	AbilityTags.AddTag(A1GameplayTags::Ability_Interact_Object);
	ActivationOwnedTags.AddTag(A1GameplayTags::Status_Interact);
}

void UA1GameplayAbility_Interact_Object::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (TriggerEventData == nullptr)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	AActor* TargetActor = const_cast<AActor*>(TriggerEventData->Target.Get());
	if (InitializeAbility(TargetActor) == false)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	bInitialized = false;

	FA1InteractionQuery Query;
	Query.RequestingAvatar = GetAvatarActorFromActorInfo();
	Query.RequestingController = GetControllerFromActorInfo();

	if (Interactable->CanInteraction(Query) == false)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	if (CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo) == false)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	bInitialized = true;

	if (AA1WorldInteractable* WorldInteractable = Cast<AA1WorldInteractable>(TargetActor))
	{
		WorldInteractable->OnInteractionSuccess(GetAvatarActorFromActorInfo());
	}

	if (UA1AbilityTask_WaitForInvalidInteraction* InvalidInteractionTask = UA1AbilityTask_WaitForInvalidInteraction::WaitForInvalidInteraction(this, AcceptanceAngle, AcceptanceDistance))
	{
		InvalidInteractionTask->OnInvalidInteraction.AddDynamic(this, &ThisClass::OnInvalidInteraction);
		InvalidInteractionTask->ReadyForActivation();
	}

	if (UAnimMontage* ActiveEndMontage = InteractionInfo.ActiveEndMontage)
	{
		if (UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("InteractMontage"), ActiveEndMontage, 1.f, NAME_None, true, 1.f, 0.f, false))
		{
			PlayMontageTask->ReadyForActivation();
		}
	}
}

void UA1GameplayAbility_Interact_Object::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	if (InteractionInfo.Duration > 0.f)
	{
		if (ALyraCharacter* LyraCharacter = GetLyraCharacterFromActorInfo())
		{
			if (UA1EquipManagerComponent* EquipManager = LyraCharacter->GetComponentByClass<UA1EquipManagerComponent>())
			{
				EquipManager->ChangeShouldHiddenEquipments(false);

				if (EquipManager->GetCurrentEquipState() != EEquipState::Unarmed)
				{
					if (AA1EquipmentBase* EquippedActor = EquipManager->GetEquippedActor(EquipManager->GetItemHandType()))
					{
						if (UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("EquipMontage"), EquippedActor->GetEquipMontage(), 1.f, NAME_None, false, 1.f, 0.f, false))
						{
							PlayMontageTask->ReadyForActivation();
						}
					}
				}
			}
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UA1GameplayAbility_Interact_Object::OnInvalidInteraction()
{
	CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
}
