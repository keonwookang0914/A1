// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "Interaction/Abilities/A1GameplayAbility_Interact_Object.h"
#include "A1GameplayAbility_Interact_Flash_Charge.generated.h"

/**
 * 
 */
UCLASS()
class UA1GameplayAbility_Interact_Flash_Charge : public UA1GameplayAbility_Interact_Object
{
	GENERATED_BODY()
public:
	UA1GameplayAbility_Interact_Flash_Charge(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
};
