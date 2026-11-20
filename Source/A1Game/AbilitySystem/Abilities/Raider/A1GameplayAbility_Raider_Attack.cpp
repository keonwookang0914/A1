// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "A1GameplayAbility_Raider_Attack.h"

#include "A1GameplayTags.h"
#include "Character/A1CreatureBase.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "Data/A1AbilityData.h"
#include "System/LyraAssetManager.h"
#include "System/LyraGameData.h"

UA1GameplayAbility_Raider_Attack::UA1GameplayAbility_Raider_Attack(const FObjectInitializer& ObjectInitializer)
{
	AbilityTags.AddTag(A1GameplayTags::Ability_Attack);
	ActivationOwnedTags.AddTag(A1GameplayTags::Status_Attack);
}

void UA1GameplayAbility_Raider_Attack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	CachedHitActors.Reset();

	SourceASC = Cast<AA1CreatureBase>(ActorInfo->AvatarActor.Get())->GetAbilitySystemComponent();
	if (SourceASC == nullptr)
		return;

    if (UAbilityTask_WaitGameplayEvent* GameplayEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, A1GameplayTags::GameplayEvent_Trace, nullptr, false, true))
    {
        GameplayEventTask->EventReceived.AddDynamic(this, &ThisClass::OnTargetDataReady);
        GameplayEventTask->ReadyForActivation();
    }

    if (UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("MeleeAttack"), AttackMontage, 1.0f, NAME_None, false, 1.f, 0.f, false))
    {
        PlayMontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageFinished);
        PlayMontageTask->ReadyForActivation();
    }

    if (UAbilityTask_WaitGameplayEvent* GameplayEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, A1GameplayTags::GameplayEvent_Montage_End, nullptr, true, true))
    {
        GameplayEventTask->EventReceived.AddDynamic(this, &ThisClass::OnMontageEventTriggered);
        GameplayEventTask->ReadyForActivation();
    }
}

void UA1GameplayAbility_Raider_Attack::OnTargetDataReady(FGameplayEventData Payload)
{
	if (SourceASC->FindAbilitySpecFromHandle(CurrentSpecHandle))
	{
		FGameplayAbilityTargetDataHandle LocalTargetDataHandle(MoveTemp(const_cast<FGameplayAbilityTargetDataHandle&>(Payload.TargetData)));

		TArray<int32> CharacterHitIndexes;
		ParseTargetData(LocalTargetDataHandle, CharacterHitIndexes);

		// TODO Jerry
		// Data Asset ¿¬µ¿
		float Damage = 20.f;

		for (int32 CharacterHitIndex : CharacterHitIndexes)
		{
			FHitResult HitResult = *LocalTargetDataHandle.Data[CharacterHitIndex]->GetHitResult();
			ProcessHitResult(HitResult, Damage, false);
		}
	}
}

void UA1GameplayAbility_Raider_Attack::ParseTargetData(const FGameplayAbilityTargetDataHandle& InTargetDataHandle, TArray<int32>& OutCharacterHitIndexes)
{
	for (int32 i = 0; i < InTargetDataHandle.Data.Num(); i++)
	{
		const TSharedPtr<FGameplayAbilityTargetData>& TargetData = InTargetDataHandle.Data[i];

		if (FHitResult* HitResult = const_cast<FHitResult*>(TargetData->GetHitResult()))
		{
			if (AActor* HitActor = HitResult->GetActor())
			{
				if (CachedHitActors.Contains(HitActor))
					continue;

				CachedHitActors.Add(HitActor);

				if (HitActor)
				{
					OutCharacterHitIndexes.Add(i);
				}
			}
		}
	}
}

void UA1GameplayAbility_Raider_Attack::ProcessHitResult(FHitResult HitResult, float Damage, bool bBlockingHit)
{
	FScopedPredictionWindow	ScopedPrediction(SourceASC, GetCurrentActivationInfo().GetActivationPredictionKey());

	FGameplayCueParameters SourceCueParams;
	SourceCueParams.Location = HitResult.ImpactPoint;
	SourceCueParams.Normal = HitResult.ImpactNormal;
	SourceCueParams.PhysicalMaterial = bBlockingHit ? nullptr : HitResult.PhysMaterial;
	SourceASC->ExecuteGameplayCue(A1GameplayTags::GameplayCue_Weapon_Impact, SourceCueParams);

	if (HasAuthority(&CurrentActivationInfo))
	{
		FGameplayAbilityTargetDataHandle TargetDataHandle = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(HitResult.GetActor());
		const TSubclassOf<UGameplayEffect> DamageGE = UA1AbilityData::Get().GetGameplayEffect("Attack");
		if (DamageGE)
		{
			FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingGameplayEffectSpec(DamageGE);

			//FGameplayEffectContextHandle EffectContextHandle = SourceASC->MakeEffectContext();
			//HitResult.bBlockingHit = bBlockingHit;
			//EffectContextHandle.AddHitResult(HitResult);
			//EffectContextHandle.AddInstigator(SourceASC->AbilityActorInfo->OwnerActor.Get(), nullptr);
			//EffectSpecHandle.Data->SetContext(EffectContextHandle);

			EffectSpecHandle.Data->SetSetByCallerMagnitude(A1GameplayTags::SetByCaller_BaseDamage, Damage);
			ApplyGameplayEffectSpecToTarget(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, EffectSpecHandle, TargetDataHandle);
		}
	}
}

void UA1GameplayAbility_Raider_Attack::OnMontageEventTriggered(FGameplayEventData Payload)
{
    OnMontageFinished();
}

void UA1GameplayAbility_Raider_Attack::OnMontageFinished()
{
    if (HasAuthority(&CurrentActivationInfo))
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
    }
}