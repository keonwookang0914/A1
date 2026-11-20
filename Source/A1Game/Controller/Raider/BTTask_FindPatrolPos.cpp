// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "BTTask_FindPatrolPos.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Controller/Raider/A1RaiderController.h"
#include "NavigationSystem.h"

UBTTask_FindPatrolPos::UBTTask_FindPatrolPos()
{
}

EBTNodeResult::Type UBTTask_FindPatrolPos::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	EBTNodeResult::Type Result = Super::ExecuteTask(OwnerComp, NodeMemory);

	auto ControllingPawn = OwnerComp.GetAIOwner()->GetPawn();
	if (ControllingPawn == nullptr)
		return EBTNodeResult::Failed;

	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetNavigationSystem(ControllingPawn->GetWorld());
	if (NavSystem == nullptr)
		return EBTNodeResult::Failed;

	auto bb = OwnerComp.GetBlackboardComponent();
	if(bb == nullptr)
		return EBTNodeResult::Failed;

	FVector Origin = ControllingPawn->GetActorLocation();
	OwnerComp.GetBlackboardComponent()->SetValueAsVector(AA1RaiderController::HomePosKey, Origin);

	FNavLocation NextPatrol;
	if (NavSystem->GetRandomPointInNavigableRadius(Origin, PatrolRange, NextPatrol))
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsVector(AA1RaiderController::PatrolPosKey, NextPatrol.Location);
		UE_LOG(LogTemp, Warning, TEXT("FindPos: %s"), *NextPatrol.Location.ToString());
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}
