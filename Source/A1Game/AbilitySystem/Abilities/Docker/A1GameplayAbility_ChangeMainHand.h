// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "A1Define.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "A1GameplayAbility_ChangeMainHand.generated.h"

/**
 * 
 */
UCLASS()
class A1GAME_API UA1GameplayAbility_ChangeMainHand : public ULyraGameplayAbility
{
	GENERATED_BODY()

public:
	UA1GameplayAbility_ChangeMainHand(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	void BlockedAnotherHandAbility(EMainHandState CurrentState);
};
