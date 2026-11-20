#include "A1GameplayAbility_Weapon_MeleeAttack.h"

#include "A1GameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "Actors/A1EquipmentBase.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Data/A1AbilityData.h"
#include "System/LyraAssetManager.h"
#include "System/LyraGameData.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1GameplayAbility_Weapon_MeleeAttack)

UA1GameplayAbility_Weapon_MeleeAttack::UA1GameplayAbility_Weapon_MeleeAttack(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ActivationGroup = ELyraAbilityActivationGroup::Exclusive_Replaceable;
}

void UA1GameplayAbility_Weapon_MeleeAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	bBlocked = false;

	if (UAbilityTask_WaitGameplayEvent* GameplayEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, A1GameplayTags::GameplayEvent_Trace, nullptr, false, true))
    {
    	GameplayEventTask->EventReceived.AddDynamic(this, &ThisClass::OnTargetDataReady);
    	GameplayEventTask->ReadyForActivation();
    }

    if (UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("MeleeAttack"), AttackMontage, GetSnapshottedAttackRate(), NAME_None, false, 1.f, 0.f, false))
    {
    	PlayMontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageFinished);
    	PlayMontageTask->ReadyForActivation();
    }

	if (UAbilityTask_WaitGameplayEvent* GameplayEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, A1GameplayTags::GameplayEvent_Montage_End, nullptr, true, true))
	{
		GameplayEventTask->EventReceived.AddDynamic(this, &ThisClass::OnMontageEventTriggered);
		GameplayEventTask->ReadyForActivation();
	}


	ConsumeOxygen();
}

void UA1GameplayAbility_Weapon_MeleeAttack::OnTargetDataReady(FGameplayEventData Payload)
{
	if (bBlocked)
		return;

	AA1EquipmentBase* WeaponActor = const_cast<AA1EquipmentBase*>(Cast<AA1EquipmentBase>(Payload.Instigator));
	if (WeaponActor == nullptr)
		return;
	
	ULyraAbilitySystemComponent* SourceASC = GetLyraAbilitySystemComponentFromActorInfo();
	if (SourceASC == nullptr)
		return;
	
	if (SourceASC->FindAbilitySpecFromHandle(CurrentSpecHandle))
	{
		FGameplayAbilityTargetDataHandle LocalTargetDataHandle(MoveTemp(const_cast<FGameplayAbilityTargetDataHandle&>(Payload.TargetData)));

		TArray<int32> CharacterHitIndexes;
		TArray<int32> BlockHitIndexes;
		ParseTargetData(LocalTargetDataHandle, CharacterHitIndexes, BlockHitIndexes);

		float Damage = GetEquipmentStatValue(A1GameplayTags::SetByCaller_BaseDamage, WeaponActor);
		
		if (BlockHitIndexes.Num() > 0)
		{
			FHitResult HitResult = *(LocalTargetDataHandle.Data[BlockHitIndexes[0]]->GetHitResult());
			ProcessHitResult(HitResult, Damage, false, BackwardMontage, WeaponActor);
			bBlocked = true;
		}
		else
		{
			for (int32 CharacterHitIndex : CharacterHitIndexes)
			{
				FHitResult HitResult = *LocalTargetDataHandle.Data[CharacterHitIndex]->GetHitResult();
				ProcessHitResult(HitResult, Damage, false, nullptr, WeaponActor);
			}
		}
	}
}

void UA1GameplayAbility_Weapon_MeleeAttack::OnMontageEventTriggered(FGameplayEventData Payload)
{
	HandleMontageEvent(Payload);
}

void UA1GameplayAbility_Weapon_MeleeAttack::HandleMontageEvent(FGameplayEventData Payload)
{
	OnMontageFinished();
}

void UA1GameplayAbility_Weapon_MeleeAttack::OnMontageFinished()
{
	if (HasAuthority(&CurrentActivationInfo))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

void UA1GameplayAbility_Weapon_MeleeAttack::ConsumeOxygen()
{
	AA1EquipmentBase* WeaponActor = GetFirstEquipmentActor();
	if (WeaponActor == nullptr)
		return;

	ULyraAbilitySystemComponent* SourceASC = GetLyraAbilitySystemComponentFromActorInfo();
	if (SourceASC == nullptr)
		return;

	ALyraCharacter* LyraCharacter = GetLyraCharacterFromActorInfo();
	if (LyraCharacter->IsOutSide())
	{
		// 가해자 정보
		FGameplayEffectContextHandle EffectContextHandle = SourceASC->MakeEffectContext();
		EffectContextHandle.AddInstigator(SourceASC->AbilityActorInfo->AvatarActor.Get(), WeaponActor);

		// 산소 소모
		float Oxygen = GetEquipmentStatValue(A1GameplayTags::SetByCaller_BaseOxygen, WeaponActor);
		FGameplayAbilityTargetDataHandle TargetDataHandle = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(SourceASC->AbilityActorInfo->AvatarActor.Get());

		const TSubclassOf<UGameplayEffect> OxygenGE = UA1AbilityData::Get().GetGameplayEffect("GE_ConsumeOxygenByAttack");
		if (OxygenGE)
		{
			FGameplayEffectSpecHandle OxygenEffectSpecHandle = MakeOutgoingGameplayEffectSpec(OxygenGE);
			OxygenEffectSpecHandle.Data->SetContext(EffectContextHandle);

			if (OxygenEffectSpecHandle.IsValid())
			{
				// 무기에 희귀도에 따른 대미지 차별화			
				OxygenEffectSpecHandle.Data->SetSetByCallerMagnitude(A1GameplayTags::SetByCaller_BaseOxygen, Oxygen);
				float DamageSet = OxygenEffectSpecHandle.Data->GetSetByCallerMagnitude(A1GameplayTags::SetByCaller_BaseOxygen, false);
				ApplyGameplayEffectSpecToTarget(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, OxygenEffectSpecHandle, TargetDataHandle);
			}
		}
	}
}
