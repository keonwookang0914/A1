// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "A1GameplayAbility_Sprint.generated.h"

/**
 * 
 */
UCLASS()
class UA1GameplayAbility_Sprint : public ULyraGameplayAbility
{
	GENERATED_BODY()
public:
    UA1GameplayAbility_Sprint(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

public:
    virtual void InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
    virtual void InputReleased(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;

private:
    UPROPERTY(EditDefaultsOnly)
    TSubclassOf<UGameplayEffect> SprintEffectClass;

    FActiveGameplayEffectHandle SprintEffectHandle;

    float OriginalMaxSpeed;
    bool bIsSprintActive;
};
