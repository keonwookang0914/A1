// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "A1AttributeSet.h"
#include "A1CharacterAttributeSet.generated.h"

UCLASS()
class A1GAME_API UA1CharacterAttributeSet : public UA1AttributeSet
{
	GENERATED_BODY()

public:
	UA1CharacterAttributeSet();

protected:
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	//virtual bool PreGameplayEffectExecute(struct FGameplayEffectModCallbackData& Data) override;
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;

public:
	ATTRIBUTE_ACCESSORS(ThisClass, Health);
	ATTRIBUTE_ACCESSORS(ThisClass, MaxHealth);
	ATTRIBUTE_ACCESSORS(ThisClass, Oxygen);
	ATTRIBUTE_ACCESSORS(ThisClass, MaxOxygen);
	ATTRIBUTE_ACCESSORS(ThisClass, Hunger);
	ATTRIBUTE_ACCESSORS(ThisClass, MaxHunger);
	ATTRIBUTE_ACCESSORS(ThisClass, Weight);
	ATTRIBUTE_ACCESSORS(ThisClass, MaxWeight);
	ATTRIBUTE_ACCESSORS(ThisClass, Damage);

private:
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData Health;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData MaxHealth;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData Oxygen;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData MaxOxygen;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData Hunger;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData MaxHunger;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData Weight;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData MaxWeight;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FGameplayAttributeData Damage;


public:
	mutable FA1AttributeEvent OnHealthChanged;
	mutable FA1AttributeEvent OnOutOfHealth;
	mutable FAttributeEvent OnNoticeWarning;
	
private:
	bool bOutOfHealth;

	TArray<bool> bHalf;
};
