// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "A1GameplayAbility_Raider_HitReact.h"

#include "A1GameplayTags.h"
#include "Abilities/Tasks/AbilityTask_NetworkSyncPoint.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Character/Raider/A1RaiderBase.h"

UA1GameplayAbility_Raider_HitReact::UA1GameplayAbility_Raider_HitReact(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bServerRespectsRemoteAbilityCancellation = true;
	bRetriggerInstancedAbility = true;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

	AbilityTags.AddTag(A1GameplayTags::Ability_HitReact);
	ActivationOwnedTags.AddTag(A1GameplayTags::Status_HitReact);

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		FAbilityTriggerData TriggerData;
		TriggerData.TriggerTag = A1GameplayTags::GameplayEvent_HitReact;
		TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
		AbilityTriggers.Add(TriggerData);
	}
}

void UA1GameplayAbility_Raider_HitReact::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (TriggerEventData == nullptr)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	if(Cast<AA1CreatureBase>(ActorInfo->OwnerActor.Get())->IsDead())
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	if (UAbilityTask_NetworkSyncPoint* NetSyncTask = UAbilityTask_NetworkSyncPoint::WaitNetSync(this, EAbilityTaskNetSyncType::OnlyServerWait))
	{
		NetSyncTask->OnSync.AddDynamic(this, &ThisClass::OnNetSync);
		NetSyncTask->ReadyForActivation();
	}
}

void UA1GameplayAbility_Raider_HitReact::OnNetSync()
{
	AA1RaiderBase* TargetCharacter = Cast<AA1RaiderBase>(CurrentActorInfo->AvatarActor.Get());
	if (TargetCharacter == nullptr)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	const FHitResult* HitResultPtr = CurrentEventData.ContextHandle.GetHitResult();
	if (HitResultPtr == nullptr)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	AActor* InstigatorActor = CurrentEventData.ContextHandle.GetInstigator();
	UAnimMontage* HitMontage = TargetCharacter->GetHitMontage(InstigatorActor, HitResultPtr->ImpactPoint, HitResultPtr->bBlockingHit);

	if (UAbilityTask_PlayMontageAndWait* HitReactMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("HitReactMontage"), HitMontage, 1.f, NAME_None, true))
	{
		HitReactMontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageFinished);
		HitReactMontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageFinished);
		HitReactMontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageFinished);
		HitReactMontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageFinished);
		HitReactMontageTask->ReadyForActivation();
	}
}

void UA1GameplayAbility_Raider_HitReact::OnMontageFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}