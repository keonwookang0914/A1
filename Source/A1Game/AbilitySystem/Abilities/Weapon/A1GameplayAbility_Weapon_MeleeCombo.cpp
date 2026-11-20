#include "A1GameplayAbility_Weapon_MeleeCombo.h"

#include "A1LogChannels.h"
#include "A1GameplayTags.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitConfirmCancel.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "AbilitySystem/Abilities/Tasks/A1AbilityTask_WaitInputStart.h"
#include "Item/Managers/A1EquipManagerComponent.h"
#include "System/LyraGameData.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1GameplayAbility_Weapon_MeleeCombo)

UA1GameplayAbility_Weapon_MeleeCombo::UA1GameplayAbility_Weapon_MeleeCombo(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ActivationGroup = ELyraAbilityActivationGroup::Exclusive_Replaceable;
	ActivationOwnedTags.AddTag(A1GameplayTags::Status_Attack);
	ActivationRequiredTags.AddTag(A1GameplayTags::Status_MainHand_Right);
	ActivationBlockedTags.AddTag(A1GameplayTags::Status_MainHand_Left);
}

void UA1GameplayAbility_Weapon_MeleeCombo::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	bInputPressed = false;
	bInputReleased = false;

	if (UAbilityTask_WaitInputRelease* InputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this, true))
	{
		InputReleaseTask->OnRelease.AddDynamic(this, &ThisClass::OnInputReleased);
		InputReleaseTask->ReadyForActivation();
	}
	
	WaitInputContinue();
	WaitInputStop();
}

void UA1GameplayAbility_Weapon_MeleeCombo::HandleMontageEvent(FGameplayEventData Payload)
{
	if (HasAuthority(&CurrentActivationInfo) == false)
		return;
	
	bool bCanContinue = NextAbilityClass && (bInputPressed || bInputReleased == false) && (bBlocked == false);
	if (bCanContinue)
	{
		if (UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponentFromActorInfo())
		{
			AbilitySystemComponent->TryActivateAbilityByClass(NextAbilityClass);
		}
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
	else if (bBlocked == false)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

void UA1GameplayAbility_Weapon_MeleeCombo::WaitInputContinue()
{
	if (UA1AbilityTask_WaitInputStart* InputStartTask = UA1AbilityTask_WaitInputStart::WaitInputStart(this))
	{
		InputStartTask->OnStart.AddDynamic(this, &ThisClass::OnInputStart);
		InputStartTask->ReadyForActivation();
	}
}

void UA1GameplayAbility_Weapon_MeleeCombo::WaitInputStop()
{
	if (UAbilityTask_WaitConfirmCancel* InputConfirmCancelTask = UAbilityTask_WaitConfirmCancel::WaitConfirmCancel(this))
	{
		InputConfirmCancelTask->OnCancel.AddDynamic(this, &ThisClass::OnInputCancel);
		InputConfirmCancelTask->ReadyForActivation();
	}
}

void UA1GameplayAbility_Weapon_MeleeCombo::OnInputReleased(float TimeHeld)
{
	bInputReleased = true;
}

void UA1GameplayAbility_Weapon_MeleeCombo::OnInputStart()
{
	bInputPressed = true;
	WaitInputContinue();
}

void UA1GameplayAbility_Weapon_MeleeCombo::OnInputCancel()
{
	bInputPressed = false;
	WaitInputStop();
}
