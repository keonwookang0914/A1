// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "A1GameplayAbility_TryInteract.generated.h"

/**
 * 
 */
UCLASS()
class A1GAME_API UA1GameplayAbility_TryInteract : public ULyraGameplayAbility
{
	GENERATED_BODY()

public:
	UA1GameplayAbility_TryInteract(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	void TryPickup();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "A1|")
	FName SpawnSocketName;

	UPROPERTY(EditDefaultsOnly, Category = "A1|Gun Projectile", meta = (EditCondition = "bApplyAnimAssist"))
	float MaxDistance = 1000.f;
};
