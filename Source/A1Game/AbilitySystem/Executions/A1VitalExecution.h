// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "GameplayEffectExecutionCalculation.h"
#include "A1VitalExecution.generated.h"

UCLASS()
class A1GAME_API UA1VitalExecution : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
    UA1VitalExecution();

    virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
