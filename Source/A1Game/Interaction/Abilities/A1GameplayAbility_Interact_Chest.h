// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Interaction/Abilities/A1GameplayAbility_Interact_Object.h"
#include "A1GameplayAbility_Interact_Chest.generated.h"

/**
 * 
 */
UCLASS()
class A1GAME_API UA1GameplayAbility_Interact_Chest : public UA1GameplayAbility_Interact_Object
{
	GENERATED_BODY()
public:
	UA1GameplayAbility_Interact_Chest(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
