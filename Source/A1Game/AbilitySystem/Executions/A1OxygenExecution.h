// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "GameplayEffectExecutionCalculation.h"
#include "A1OxygenExecution.generated.h"

/**
 * 
 */
UCLASS()
class A1GAME_API UA1OxygenExecution : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
    UA1OxygenExecution();

    virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
