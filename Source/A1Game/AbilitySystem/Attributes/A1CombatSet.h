#pragma once

#include "AbilitySystemComponent.h"
#include "A1AttributeSet.h"
#include "A1CombatSet.generated.h"

UCLASS(BlueprintType)
class UA1CombatSet : public UA1AttributeSet
{
	GENERATED_BODY()

public:
	UA1CombatSet();

protected:
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

private:
	void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;
	
public:
	ATTRIBUTE_ACCESSORS(ThisClass, BaseDamage);
	ATTRIBUTE_ACCESSORS(ThisClass, BaseHealth);
	ATTRIBUTE_ACCESSORS(ThisClass, BaseOxygen);

protected:
	UFUNCTION()
	void OnRep_BaseDamage(const FGameplayAttributeData& OldValue);
	
	UFUNCTION()
	void OnRep_BaseHealth(const FGameplayAttributeData& OldValue);
	
	UFUNCTION()
	void OnRep_BaseOxygen(const FGameplayAttributeData& OldValue);

private:
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_BaseDamage, meta=(HideFromModifiers, AllowPrivateAccess="true"))
	FGameplayAttributeData BaseDamage;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_BaseHealth, meta=(AllowPrivateAccess="true"))
	FGameplayAttributeData BaseHealth;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing= OnRep_BaseOxygen, meta=(AllowPrivateAccess="true"))
	FGameplayAttributeData BaseOxygen;

public:
	ATTRIBUTE_ACCESSORS(ThisClass, MoveSpeed);
	ATTRIBUTE_ACCESSORS(ThisClass, MoveSpeedPercent);
	ATTRIBUTE_ACCESSORS(ThisClass, AttackSpeedPercent);

protected:
	UFUNCTION()
	void OnRep_MoveSpeed(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MoveSpeedPercent(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_AttackSpeedPercent(const FGameplayAttributeData& OldValue);

private:
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_MoveSpeed, meta=(AllowPrivateAccess="true"))
	FGameplayAttributeData MoveSpeed;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_MoveSpeedPercent, meta=(AllowPrivateAccess="true"))
	FGameplayAttributeData MoveSpeedPercent;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_AttackSpeedPercent, meta=(AllowPrivateAccess="true"))
	FGameplayAttributeData AttackSpeedPercent;
	
public:
	ATTRIBUTE_ACCESSORS(ThisClass, Resourcefulness);
	
protected:
	// 상호작용 속도
	UFUNCTION()
	void OnRep_Resourcefulness(const FGameplayAttributeData& OldValue);

private:
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_Resourcefulness, meta=(AllowPrivateAccess="true"))
	FGameplayAttributeData Resourcefulness;

public:
	ATTRIBUTE_ACCESSORS(ThisClass, ActiveEffectDuration);

private:
	UPROPERTY(BlueprintReadOnly, meta=(HideFromModifiers, AllowPrivateAccess="true"))
	FGameplayAttributeData ActiveEffectDuration;
};
