// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "BehaviorTree/BTService.h"
#include "BTService_meetsAttackCriteria.generated.h"

/**
 * 
 */
UCLASS()
class A1GAME_API UBTService_meetsAttackCriteria : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_meetsAttackCriteria();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
};
