#pragma once

#include "AbilitySystemComponent.h"
#include "A1AttributeSet.h"
#include "NativeGameplayTags.h"
#include "A1VitalSet.generated.h"

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_Damage);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_DamageImmunity);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_DamageSelfDestruct);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_FellOutOfWorld);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Lyra_Damage_Message);

UCLASS(BlueprintType)
class UA1VitalSet : public UA1AttributeSet
{
	GENERATED_BODY()

public:
	UA1VitalSet();

protected:
	virtual bool PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

private:
	void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;
	
public:
	ATTRIBUTE_ACCESSORS(ThisClass, Health);
	ATTRIBUTE_ACCESSORS(ThisClass, MaxHealth);
	ATTRIBUTE_ACCESSORS(ThisClass, Oxygen);
	ATTRIBUTE_ACCESSORS(ThisClass, MaxOxygen);

protected:
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_Oxygen(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxOxygen(const FGameplayAttributeData& OldValue);

private:
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Health, meta=(AllowPrivateAccess="true"))
	FGameplayAttributeData Health;
	
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_MaxHealth, meta=(AllowPrivateAccess="true"))
	FGameplayAttributeData MaxHealth;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Oxygen, meta=(AllowPrivateAccess="true"))
	FGameplayAttributeData Oxygen;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_MaxOxygen, meta=(AllowPrivateAccess="true"))
	FGameplayAttributeData MaxOxygen;

public:
	ATTRIBUTE_ACCESSORS(ThisClass, IncomingHealth);
	ATTRIBUTE_ACCESSORS(ThisClass, IncomingOxygen);
	ATTRIBUTE_ACCESSORS(ThisClass, IncomingDamage);
	
private:
	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	FGameplayAttributeData IncomingHealth;
	
	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	FGameplayAttributeData IncomingOxygen;
	
	UPROPERTY(BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	FGameplayAttributeData IncomingDamage;

public:
	mutable FLyraAttributeEvent OnHealthChanged;
	mutable FLyraAttributeEvent OnMaxHealthChanged;
	mutable FLyraAttributeEvent OnOutOfHealth;

private:
	bool bOutOfHealth;
	
	float MaxHealthBeforeAttributeChange;
	float HealthBeforeAttributeChange;
};
