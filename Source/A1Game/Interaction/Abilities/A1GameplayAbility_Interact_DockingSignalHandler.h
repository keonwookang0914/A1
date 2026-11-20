// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "A1GameplayAbility_Interact_Object.h"
#include "A1GameplayAbility_Interact_DockingSignalHandler.generated.h"

/**
 * 
 */
UCLASS()
class UA1GameplayAbility_Interact_DockingSignalHandler : public UA1GameplayAbility_Interact_Object
{
	GENERATED_BODY()
public:
	UA1GameplayAbility_Interact_DockingSignalHandler(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
