// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "A1GameplayAbility_DropItem.generated.h"

/**
 * 
 */
UCLASS()
class A1GAME_API UA1GameplayAbility_DropItem : public ULyraGameplayAbility
{
	GENERATED_BODY()


public:
	UA1GameplayAbility_DropItem(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
    virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags,FGameplayTagContainer* OptionalRelevantTags) const override;
};
