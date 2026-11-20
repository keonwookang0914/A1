#pragma once

#include "Interaction/Abilities/A1GameplayAbility_Interact_Info.h"
#include "A1GameplayAbility_Interact_Active.generated.h"

class UInputAction;

UCLASS()
class A1GAME_API UA1GameplayAbility_Interact_Active : public UA1GameplayAbility_Interact_Info
{
	GENERATED_BODY()
public:
	UA1GameplayAbility_Interact_Active(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	UFUNCTION()
	void OnInvalidInteraction();

	UFUNCTION()
	void OnInputReleased(float TimeHeld);

	UFUNCTION()
	void OnDurationEnded();

	UFUNCTION()
	void OnNetSync();

	UFUNCTION()
	bool TriggerInteraction();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "A1|Interaction")
	TObjectPtr<UInputAction> MoveInputAction;

	UPROPERTY(EditDefaultsOnly, Category = "A1|Interaction")
	float AcceptanceAngle = 65.f;

	UPROPERTY(EditDefaultsOnly, Category = "A1|Interaction")
	float AcceptanceDistance = 10.f;
};
