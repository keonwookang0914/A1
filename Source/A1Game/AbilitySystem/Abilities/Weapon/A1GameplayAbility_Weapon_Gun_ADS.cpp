#include "A1GameplayAbility_Weapon_Gun_ADS.h"

#include "A1GameplayTags.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayTag.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1GameplayAbility_Weapon_Gun_ADS)

UA1GameplayAbility_Weapon_Gun_ADS::UA1GameplayAbility_Weapon_Gun_ADS(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AbilityTags.AddTag(A1GameplayTags::Ability_ADS_Gun);
	ActivationOwnedTags.AddTag(A1GameplayTags::Status_ADS_Gun);
	ActivationBlockedTags.AddTag(A1GameplayTags::Status_Attack);
	//ActivationBlockedTags.AddTag(A1GameplayTags::Status_MainHand_Left);
	//ActivationRequiredTags.AddTag(A1GameplayTags::Status_MainHand_Right);
}

void UA1GameplayAbility_Weapon_Gun_ADS::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	ADSEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, A1GameplayTags::GameplayEvent_Gun_ADS, nullptr, false, true);
	if (ADSEventTask)
	{
		ADSEventTask->EventReceived.AddDynamic(this, &ThisClass::OnADSEvent);
		ADSEventTask->ReadyForActivation();
	}
	
	SetCameraMode(CameraModeClass);

	InputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this, true);
	if (InputReleaseTask)
	{
		InputReleaseTask->OnRelease.AddDynamic(this, &ThisClass::OnInputRelease);
		InputReleaseTask->ReadyForActivation();
	}

	StartADS();
}

void UA1GameplayAbility_Weapon_Gun_ADS::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	ResetADS();

	if (bWasCancelled)
	{
		//if (UAbilityTask_PlayMontageAndWait* ADSEndMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("ADSEndMontage"), ADSEndMontage, 1.f, NAME_None, false))
		//{
		//	ADSEndMontageTask->ReadyForActivation();
		//}
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UA1GameplayAbility_Weapon_Gun_ADS::OnADSEvent(FGameplayEventData Payload)
{
	StartADS();
}

void UA1GameplayAbility_Weapon_Gun_ADS::OnADSStartBegin(FGameplayEventData Payload)
{
	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(A1GameplayTags::Status_ADS_Ready);
	UAbilitySystemBlueprintLibrary::AddLooseGameplayTags(GetAvatarActorFromActorInfo(), TagContainer, true);
}

void UA1GameplayAbility_Weapon_Gun_ADS::OnInputRelease(float TimeHeld)
{
	ResetADS();

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC == nullptr || ASC->HasMatchingGameplayTag(A1GameplayTags::Status_Attack) == false)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	if (UAbilityTask_WaitGameplayTagRemoved* WaitAttackEndTask = UAbilityTask_WaitGameplayTagRemoved::WaitGameplayTagRemove(this, A1GameplayTags::Status_Attack, nullptr, true))
	{
		WaitAttackEndTask->Removed.AddDynamic(this, &ThisClass::OnAttackEnd);
		WaitAttackEndTask->ReadyForActivation();
	}
}

void UA1GameplayAbility_Weapon_Gun_ADS::OnAttackEnd()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UA1GameplayAbility_Weapon_Gun_ADS::StartADS()
{
	if (UAbilityTask_PlayMontageAndWait* ADSStartMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("ADSStartMontage"), ADSStartMontage, 1.f, NAME_None, true))
	{
		ADSStartMontageTask->ReadyForActivation();
	}
	
	ADSStartBeginEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, A1GameplayTags::GameplayEvent_Montage_Begin, nullptr, true, true);
	if (ADSStartBeginEventTask)
	{
		ADSStartBeginEventTask->EventReceived.AddDynamic(this, &ThisClass::OnADSStartBegin);
		ADSStartBeginEventTask->ReadyForActivation();
	}
}

void UA1GameplayAbility_Weapon_Gun_ADS::ResetADS()
{
	if (ADSEventTask)
	{
		ADSEventTask->EndTask();
	}

	if (InputReleaseTask)
	{
		InputReleaseTask->EndTask();
	}

	if (ADSStartBeginEventTask)
	{
		ADSStartBeginEventTask->EndTask();
	}

	ClearCameraMode();

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (ASC && ASC->HasMatchingGameplayTag(A1GameplayTags::Status_ADS_Ready))
	{
		FGameplayTagContainer TagContainer;
		TagContainer.AddTag(A1GameplayTags::Status_ADS_Ready);
		UAbilitySystemBlueprintLibrary::RemoveLooseGameplayTags(GetAvatarActorFromActorInfo(), TagContainer, true);
	}
}
