// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "Abilities/GameplayAbility.h"
#include "A1GameplayAbility_Raider_Attack.generated.h"

/**
 * 
 */
UCLASS()
class A1GAME_API UA1GameplayAbility_Raider_Attack : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UA1GameplayAbility_Raider_Attack(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	void ParseTargetData(const FGameplayAbilityTargetDataHandle& InTargetDataHandle, TArray<int32>& OutCharacterHitIndexes);
	void ProcessHitResult(FHitResult HitResult, float Damage, bool bBlockingHit);

private:
	UFUNCTION()
	void OnTargetDataReady(FGameplayEventData Payload);

	UFUNCTION()
	void OnMontageEventTriggered(FGameplayEventData Payload);

	UFUNCTION()
	void OnMontageFinished();

protected:
	UAbilitySystemComponent* SourceASC = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "A1|Melee Attack")
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY()
	TSet<TWeakObjectPtr<AActor>> CachedHitActors;
};
