// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "A1GameplayAbility_Plunderer_Attack.h"

#include "A1GameplayTags.h"
#include "Character/Raider/A1Plunderer.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "Actors/A1StorageBase.h"
#include "Actors/A1EquipmentBase.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "Controller/Raider/A1RaiderController.h"
#include "System/LyraAssetManager.h"
#include "System/LyraGameData.h"


UA1GameplayAbility_Plunderer_Attack::UA1GameplayAbility_Plunderer_Attack(const FObjectInitializer& ObjectInitializer)
{
	AbilityTags.AddTag(A1GameplayTags::Ability_Attack);
	ActivationOwnedTags.AddTag(A1GameplayTags::Status_Attack);
}

void UA1GameplayAbility_Plunderer_Attack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	SourceASC = Cast<AA1Plunderer>(ActorInfo->AvatarActor.Get())->GetAbilitySystemComponent();
	if (SourceASC == nullptr)
		return;

	if (UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("MeleeAttack"), AttackMontage, 1.0f, NAME_None, false, 1.f, 0.f, false))
	{
		PlayMontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageFinished);
		PlayMontageTask->ReadyForActivation();
	}
}

void UA1GameplayAbility_Plunderer_Attack::OnMontageFinished()
{
	AA1Plunderer* ControllingPawn = Cast<AA1Plunderer>(CurrentActorInfo->AvatarActor.Get());
	AA1RaiderController* PawnController = Cast<AA1RaiderController>(ControllingPawn->GetController());

	UBlackboardComponent* BlackBoard = PawnController->GetBlackboardComponent();
	if (BlackBoard != nullptr)
	{
		FNavLocation NextPatrol;
		if (BlackBoard->GetValueAsObject(AA1RaiderController::AggroTargetKey) != nullptr)
		{
			AA1StorageBase* Storage = Cast<AA1StorageBase>(BlackBoard->GetValueAsObject(AA1RaiderController::AggroTargetKey));
			BlackBoard->SetValueAsBool(AA1RaiderController::CanAttackKey, false);

			if (Storage->RemoveItem(ControllingPawn))					// 털 아이템이 없는 경우
			{
				BlackBoard->SetValueAsObject(AA1RaiderController::AggroTargetKey, nullptr);
			}
		}
	}

    if (HasAuthority(&CurrentActivationInfo))
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
    }
}