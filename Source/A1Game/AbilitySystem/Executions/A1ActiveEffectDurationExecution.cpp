#include "A1ActiveEffectDurationExecution.h"

#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/A1CombatSet.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1ActiveEffectDurationExecution)

struct FActiveDurationStatics
{
public:
	FActiveDurationStatics()
	{
		TargetActiveEffectDurationDef = FGameplayEffectAttributeCaptureDefinition(UA1CombatSet::GetActiveEffectDurationAttribute(), EGameplayEffectAttributeCaptureSource::Target, true);
	}

public:
	FGameplayEffectAttributeCaptureDefinition TargetActiveEffectDurationDef;
};
 
static FActiveDurationStatics& ActiveDurationStatics()
{
	static FActiveDurationStatics Statics;
	return Statics;
}

UA1ActiveEffectDurationExecution::UA1ActiveEffectDurationExecution()
{
	RelevantAttributesToCapture.Add(ActiveDurationStatics().TargetActiveEffectDurationDef);
}

void UA1ActiveEffectDurationExecution::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
#if WITH_SERVER_CODE
	ULyraAbilitySystemComponent* TargetASC = Cast<ULyraAbilitySystemComponent>(ExecutionParams.GetTargetAbilitySystemComponent());
	check(TargetASC);
	
    FAggregatorEvaluateParameters EvaluateParameters;
	
    TArray<FActiveGameplayEffectHandle> Handles = TargetASC->GetAllActiveEffectHandles();
    for (const FActiveGameplayEffectHandle Handle : Handles)
    {
        FActiveGameplayEffect* ActiveGameplayEffect = TargetASC->GetActiveGameplayEffect_Mutable(Handle);
        if (ActiveGameplayEffect == nullptr || ActiveGameplayEffect->Spec.Def->DurationPolicy != EGameplayEffectDurationType::HasDuration)
            continue;
    	
        FGameplayTagContainer Tags;
        ActiveGameplayEffect->Spec.GetAllAssetTags(Tags);
        EvaluateParameters.TargetTags = &Tags;
    	
        float NewDuration = 0.f;
        ExecutionParams.AttemptCalculateCapturedAttributeMagnitudeWithBase(ActiveDurationStatics().TargetActiveEffectDurationDef, EvaluateParameters, ActiveGameplayEffect->GetDuration(), NewDuration);
        ActiveGameplayEffect->Spec.Duration = FMath::Max(NewDuration, SMALL_NUMBER);
    	
        TargetASC->MarkActiveGameplayEffectDirty(ActiveGameplayEffect);
        TargetASC->CheckActiveEffectDuration(Handle);
    }
#endif // #if WITH_SERVER_CODE
}
