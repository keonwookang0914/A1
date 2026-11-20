#pragma once

#include "GameplayEffectExecutionCalculation.h"
#include "A1ActiveEffectDurationExecution.generated.h"

UCLASS()
class UA1ActiveEffectDurationExecution : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	UA1ActiveEffectDurationExecution();

protected:
	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
