// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "AbilitySystem/Executions/A1OxygenExecution.h"
#include "AbilitySystem/Attributes/A1CharacterAttributeSet.h"
#include "GameplayEffectTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1OxygenExecution)

struct FOxygenStatics
{
    FGameplayEffectAttributeCaptureDefinition OxygenDef;

    FOxygenStatics()
    {
        OxygenDef = FGameplayEffectAttributeCaptureDefinition(UA1CharacterAttributeSet::GetOxygenAttribute(), EGameplayEffectAttributeCaptureSource::Target, false);
    }
};

static FOxygenStatics& OxygenStatics()
{
    static FOxygenStatics Statics;
    return Statics;
}

UA1OxygenExecution::UA1OxygenExecution()
{
    RelevantAttributesToCapture.Add(OxygenStatics().OxygenDef);
}

void UA1OxygenExecution::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
#if WITH_SERVER_CODE
    const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

    const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
    const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

    FAggregatorEvaluateParameters EvaluateParameters;
    EvaluateParameters.SourceTags = SourceTags;
    EvaluateParameters.TargetTags = TargetTags;

    float CurrentOxygen = 0.0f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(OxygenStatics().OxygenDef, EvaluateParameters, CurrentOxygen);

    // 5초마다 Oxygen 감소
    OutExecutionOutput.AddOutputModifier(
        FGameplayModifierEvaluatedData(UA1CharacterAttributeSet::GetOxygenAttribute(), EGameplayModOp::Additive, -2.f));

    // Oxygen이 0 이하면 hp감소, weight 증가
    if (CurrentOxygen <= 0.0f)
    {
        OutExecutionOutput.AddOutputModifier(
            FGameplayModifierEvaluatedData(UA1CharacterAttributeSet::GetHealthAttribute(), EGameplayModOp::Additive, -10.f));

        OutExecutionOutput.AddOutputModifier(
            FGameplayModifierEvaluatedData(UA1CharacterAttributeSet::GetWeightAttribute(), EGameplayModOp::Additive, 20.f));
    }

#endif // #if WITH_SERVER_CODE
}
