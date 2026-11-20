// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "Controller/Raider/BTTask_Attack.h"

#include "A1GameplayTags.h"
#include "A1LogChannels.h"
#include "A1RaiderController.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Raider/A1RaiderBase.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"


UBTTask_Attack::UBTTask_Attack()
{
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	EBTNodeResult::Type Result = Super::ExecuteTask(OwnerComp, NodeMemory);

	AA1RaiderBase* ControllingPawn = Cast<AA1RaiderBase>(OwnerComp.GetAIOwner()->GetPawn());
	if (ControllingPawn == nullptr)
		return EBTNodeResult::Failed;

	UAbilitySystemComponent* ASC = ControllingPawn->FindComponentByClass<UAbilitySystemComponent>();
	if (ASC)
	{
		FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromInputID(0);
		if (ASC->IsOwnerActorAuthoritative())
			ASC->TryActivateAbility(Spec->Handle);

		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}