// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "Interaction/Abilities/A1GameplayAbility_Interact_Object.h"
#include "A1GameplayAbility_Interact_Bed.generated.h"

/**
 * 
 */
UCLASS()
class UA1GameplayAbility_Interact_Bed : public UA1GameplayAbility_Interact_Object
{
	GENERATED_BODY()
public:
    UA1GameplayAbility_Interact_Bed(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

    UFUNCTION(BlueprintImplementableEvent)
    void PlayScreenFadeEffect(bool bFadeIn);
};
