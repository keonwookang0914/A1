// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "Abilities/GameplayAbility.h"
#include "A1GameplayAbility_Raider_HitReact.generated.h"

/**
 * 
 */
UCLASS()
class A1GAME_API UA1GameplayAbility_Raider_HitReact : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UA1GameplayAbility_Raider_HitReact(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

private:
	UFUNCTION()
	void OnNetSync();

	UFUNCTION()
	void OnMontageFinished();
};
