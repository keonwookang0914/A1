#pragma once

#include "AbilitySystem/Abilities/Docker/A1GameplayAbility_Equipment.h"
#include "A1GameplayAbility_Utility_FlashLight_Charge.generated.h"

UCLASS()
class UA1GameplayAbility_Utility_FlashLight_Charge : public UA1GameplayAbility_Equipment
{
	GENERATED_BODY()
	
public:
	UA1GameplayAbility_Utility_FlashLight_Charge(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	UFUNCTION()
	void OnMontageFinished();

protected:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UAnimMontage> ChargeMontage;
};
