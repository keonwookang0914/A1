#pragma once

//#include "A1GameplayAbility_Weapon_Gun_Projectile.h"
#include "AbilitySystem/Abilities/Docker/A1GameplayAbility_Equipment.h"
#include "A1GameplayAbility_Weapon_Gun_NormalShoot.generated.h"

UCLASS()
class UA1GameplayAbility_Weapon_Gun_NormalShoot : public UA1GameplayAbility_Equipment
{
	GENERATED_BODY()
	
public:
	UA1GameplayAbility_Weapon_Gun_NormalShoot(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
protected:
	UFUNCTION()
	void OnMontageFinished();

protected:
	bool Shoot();
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "A1|Gun Projectile")
	FName SpawnSocketName;

	UPROPERTY(EditDefaultsOnly, Category = "A1|Gun Projectile")
	bool bApplyAimAssist = true;

	UPROPERTY(EditDefaultsOnly, Category = "A1|Gun Projectile", meta = (EditCondition = "bApplyAnimAssist"))
	float AimAssistMinDistance = 100.f;

	UPROPERTY(EditDefaultsOnly, Category = "A1|Gun Projectile", meta = (EditCondition = "bApplyAnimAssist"))
	float AimAssistMaxDistance = 10000.f;

protected:
	UPROPERTY(EditDefaultsOnly, Category="A1|Gun NormalShoot")
	TObjectPtr<UAnimMontage> ShootMontage;
	
	UPROPERTY(EditDefaultsOnly, Category="A1|Gun NormalShoot")
	TObjectPtr<UAnimMontage> ReloadMontage;
	
	UPROPERTY(EditDefaultsOnly, Category="A1|Gun NormalShoot")
	TObjectPtr<UAnimMontage> ReleaseReloadMontage;

	UPROPERTY(EditAnywhere, Category = "GAS")
	TSubclassOf<class UGameplayEffect> AttackDamageEffect;

	UPROPERTY(EditAnywhere)
	USoundBase* GunSound;
};
