#pragma once

#include "A1GameplayAbility_Weapon_Melee.h"
#include "A1GameplayAbility_Weapon_MeleeAttack.generated.h"

UCLASS()
class UA1GameplayAbility_Weapon_MeleeAttack : public UA1GameplayAbility_Weapon_Melee
{
	GENERATED_BODY()
	
public:
	UA1GameplayAbility_Weapon_MeleeAttack(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void HandleMontageEvent(FGameplayEventData Payload);
	
private:
	UFUNCTION()
	void OnTargetDataReady(FGameplayEventData Payload);

	UFUNCTION()
	void OnMontageEventTriggered(FGameplayEventData Payload);
	
	UFUNCTION()
	void OnMontageFinished();

	void ConsumeOxygen();
	
protected:
	UPROPERTY(EditDefaultsOnly, Category="A1|Melee Attack")
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditDefaultsOnly, Category="A1|Melee Attack")
	TObjectPtr<UAnimMontage> BackwardMontage;
	
protected:
	bool bBlocked = false;
	FTimerHandle BlockMontageTimerHandle;
};
