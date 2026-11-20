#include "Interaction/Abilities/A1GameplayAbility_Interact_Active.h"

#include "AbilitySystemComponent.h"
#include "A1GameplayAbility_Interact.h"
#include "A1GameplayTags.h"
#include "Abilities/Tasks/AbilityTask_NetworkSyncPoint.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Actors/A1EquipmentBase.h"
#include "Character/LyraCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Interaction/A1Interactable.h"
#include "Interaction/A1WorldInteractable.h"
#include "Item/Managers/A1EquipManagerComponent.h"
#include "Tasks/A1AbilityTask_WaitForInvalidInteraction.h"
#include "Player/LyraPlayerController.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1GameplayAbility_Interact_Active)

UA1GameplayAbility_Interact_Active::UA1GameplayAbility_Interact_Active(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ActivationPolicy = ELyraAbilityActivationPolicy::Manual;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	bServerRespectsRemoteAbilityCancellation = true;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ServerOnlyTermination;

	AbilityTags.AddTag(A1GameplayTags::Ability_Interact_Active);
	ActivationOwnedTags.AddTag(A1GameplayTags::Status_Interact);

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		FAbilityTriggerData TriggerData;
		TriggerData.TriggerTag = A1GameplayTags::Ability_Interact_Active;
		TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
		AbilityTriggers.Add(TriggerData);
	}
}

void UA1GameplayAbility_Interact_Active::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (TriggerEventData == nullptr)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	if (InitializeAbility(const_cast<AActor*>(TriggerEventData->Target.Get())) == false)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	if (AA1WorldInteractable* WorldInteractable = Cast<AA1WorldInteractable>(InteractableActor))
	{
		WorldInteractable->OnInteractActiveStarted(GetLyraCharacterFromActorInfo());
	}

	if (InteractionInfo.Duration <= 0.f)
	{
		TriggerInteraction();
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	FlushPressedInput(MoveInputAction);

	if (ALyraCharacter* LyraCharacter = GetLyraCharacterFromActorInfo())
	{
		if (UCharacterMovementComponent* CharacterMovement = LyraCharacter->GetCharacterMovement())
		{
			CharacterMovement->StopMovementImmediately();
		}

		if (UA1EquipManagerComponent* EquipManager = LyraCharacter->GetComponentByClass<UA1EquipManagerComponent>())
		{
			EquipManager->ChangeShouldHiddenEquipments(true);
		}

		ALyraPlayerController* PlayerController = GetLyraPlayerControllerFromActorInfo();
		if (PlayerController == nullptr)
		{
			CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
			return;
		}
		LyraCharacter->DisableInput(PlayerController);
	}

	FGameplayCueParameters Parameters;
	Parameters.Instigator = InteractableActor;
	K2_AddGameplayCueWithParams(InteractionInfo.ActiveLoopGameplayCueTag, Parameters, true);

	FA1InteractionMessage Message;
	Message.Instigator = GetAvatarActorFromActorInfo();
	Message.bShouldRefresh = true;
	Message.bSwitchActive = true;
	Message.InteractionInfo = InteractionInfo;

	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
	MessageSubsystem.BroadcastMessage(A1GameplayTags::Message_Interaction_Progress, Message);

	if (UAnimMontage* ActiveStartMontage = InteractionInfo.ActiveStartMontage)
	{
		if (UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("InteractMontage"), ActiveStartMontage, 1.f, NAME_None, true, 1.f, 0.f, false))
		{
			PlayMontageTask->ReadyForActivation();
		}
	}

	if (UA1AbilityTask_WaitForInvalidInteraction* InvalidInteractionTask = UA1AbilityTask_WaitForInvalidInteraction::WaitForInvalidInteraction(this, AcceptanceAngle, AcceptanceDistance))
	{
		InvalidInteractionTask->OnInvalidInteraction.AddDynamic(this, &ThisClass::OnInvalidInteraction);
		InvalidInteractionTask->ReadyForActivation();
	}

	//if (UAbilityTask_WaitInputRelease* InputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this, false))
	//{
	//	InputReleaseTask->OnRelease.AddDynamic(this, &ThisClass::OnInputReleased);
	//	InputReleaseTask->ReadyForActivation();
	//}

	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &ThisClass::OnDurationEnded, InteractionInfo.Duration, false);
}

void UA1GameplayAbility_Interact_Active::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	// 입력 가능
	ALyraCharacter* Character = Cast<ALyraCharacter>(GetAvatarActorFromActorInfo());
	ALyraPlayerController* PlayerController = GetLyraPlayerControllerFromActorInfo();
	Character->EnableInput(PlayerController);

	if (ALyraCharacter* LyraCharacter = GetLyraCharacterFromActorInfo())
	{
		if (bWasCancelled)
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

		if (AA1WorldInteractable* WorldInteractable = Cast<AA1WorldInteractable>(InteractableActor))
		{
			WorldInteractable->OnInteractActiveEnded(LyraCharacter);
		}

		FA1InteractionMessage Message;
		Message.Instigator = LyraCharacter;
		Message.bShouldRefresh = false;
		Message.bSwitchActive = true;

		UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
		MessageSubsystem.BroadcastMessage(A1GameplayTags::Message_Interaction_Notice, Message);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UA1GameplayAbility_Interact_Active::OnInvalidInteraction()
{
	CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
}

void UA1GameplayAbility_Interact_Active::OnInputReleased(float TimeHeld)
{
	CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
}

void UA1GameplayAbility_Interact_Active::OnDurationEnded()
{
	if (UAbilityTask_NetworkSyncPoint* NetSyncTask = UAbilityTask_NetworkSyncPoint::WaitNetSync(this, EAbilityTaskNetSyncType::OnlyServerWait))
	{
		NetSyncTask->OnSync.AddDynamic(this, &ThisClass::OnNetSync);
		NetSyncTask->ReadyForActivation();
	}
}

void UA1GameplayAbility_Interact_Active::OnNetSync()
{
	if (TriggerInteraction())
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
	else
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
	}
}

bool UA1GameplayAbility_Interact_Active::TriggerInteraction()
{
	bool bTriggerSuccessful = false;
	bool bCanActivate = false;

	FGameplayEventData Payload;
	Payload.EventTag = A1GameplayTags::Ability_Interact;
	Payload.Instigator = GetAvatarActorFromActorInfo();
	Payload.Target = InteractableActor;

	Interactable->CustomizeInteractionEventData(A1GameplayTags::Ability_Interact, Payload);

	if (UAbilitySystemComponent* AbilitySystem = GetAbilitySystemComponentFromActorInfo())
	{
		if (FGameplayAbilitySpec* AbilitySpec = AbilitySystem->FindAbilitySpecFromClass(InteractionInfo.AbilityToGrant))
		{
			bCanActivate = AbilitySpec->Ability->CanActivateAbility(AbilitySpec->Handle, AbilitySystem->AbilityActorInfo.Get());
			bTriggerSuccessful = AbilitySystem->TriggerAbilityFromGameplayEvent(
				AbilitySpec->Handle,
				AbilitySystem->AbilityActorInfo.Get(),
				A1GameplayTags::Ability_Interact,
				&Payload,
				*AbilitySystem
			);
		}
	}

	return bCanActivate || bTriggerSuccessful;
}
