// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "Controller/Raider/BTTask_Plunderer_ReAttack.h"

#include "A1GameplayTags.h"
#include "A1LogChannels.h"
#include "A1RaiderController.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Raider/A1RaiderBase.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"


UBTTask_Plunderer_ReAttack::UBTTask_Plunderer_ReAttack()
{
}

EBTNodeResult::Type UBTTask_Plunderer_ReAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	EBTNodeResult::Type Result = Super::ExecuteTask(OwnerComp, NodeMemory);

	AA1RaiderBase* ControllingPawn = Cast<AA1RaiderBase>(OwnerComp.GetAIOwner()->GetPawn());
	if (ControllingPawn == nullptr)
		return EBTNodeResult::Failed;

	UBlackboardComponent* BlackBoard = OwnerComp.GetBlackboardComponent();
	if (BlackBoard->GetValueAsObject(AA1RaiderController::AggroTargetKey) != nullptr)
	{
		BlackBoard->SetValueAsBool(AA1RaiderController::CanAttackKey, true);
	}

	return EBTNodeResult::Failed;
}