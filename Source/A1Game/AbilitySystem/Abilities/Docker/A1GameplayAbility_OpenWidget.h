// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "A1GameplayAbility_OpenWidget.generated.h"

/**
 * 
 */
UCLASS()
class A1GAME_API UA1GameplayAbility_OpenWidget : public ULyraGameplayAbility
{
	GENERATED_BODY()
	
public:
	UA1GameplayAbility_OpenWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

public:
	UFUNCTION(BlueprintImplementableEvent)
	void ActiveWidget();
	UFUNCTION(BlueprintImplementableEvent)
	void DeactiveWidget();

	UPROPERTY(EditAnywhere)
	FGameplayTagContainer WidgetTag;
};
