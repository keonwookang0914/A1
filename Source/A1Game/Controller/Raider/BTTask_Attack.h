// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Attack.generated.h"

/**
 * 
 */
UCLASS()
class A1GAME_API UBTTask_Attack : public UBTTaskNode
{
	GENERATED_BODY()


public:
	UBTTask_Attack();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
