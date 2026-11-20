// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Plunderer_ReAttack.generated.h"

/**
 * 
 */
UCLASS()
class A1GAME_API UBTTask_Plunderer_ReAttack : public UBTTaskNode
{
	GENERATED_BODY()


public:
	UBTTask_Plunderer_ReAttack();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
