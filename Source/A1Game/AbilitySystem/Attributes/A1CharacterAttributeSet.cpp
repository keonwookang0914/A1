// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "AbilitySystem/Attributes/A1CharacterAttributeSet.h"
#include "A1LogChannels.h"
#include "GameplayEffectExtension.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1CharacterAttributeSet)

UA1CharacterAttributeSet::UA1CharacterAttributeSet() :
	MaxHealth(120.0f),
	MaxOxygen(500.0f),
	MaxHunger(720.0f),
	MaxWeight(100.0f)
{
	InitHealth(GetMaxHealth());
	InitOxygen(GetMaxOxygen());
	InitHunger(GetMaxHunger());
	InitWeight(0.f);

	bOutOfHealth = false;
	bHalf = { true, true,  true,  true };
}

void UA1CharacterAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	else if (Attribute == GetWeightAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxWeight());
	}
	else if (Attribute == GetOxygenAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxOxygen());
	}
	else if (Attribute == GetHungerAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHunger());
	}
}

void UA1CharacterAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	// TODO Jerry
	// Replication 작업 시 이주
	bool bChange = (NewValue < OldValue);
	int NoticeIndex = -1;

	if (Attribute == GetHealthAttribute())
	{
		const float CurrentHealth = GetHealth();
		UE_LOG(LogA1Player, Log, TEXT("Health : %f -> %f"), OldValue, NewValue);

		if (bOutOfHealth == false && CurrentHealth <= 0.0f)
			OnOutOfHealth.Broadcast(nullptr, OldValue, NewValue);

		if (bChange && CurrentHealth <= GetMaxHealth() / 2)
			NoticeIndex = 2;
		else if(!bChange && CurrentHealth > GetMaxHealth() / 2)
			bHalf[2] = true;

		bOutOfHealth = (CurrentHealth <= 0.0f);
	}
	else if (Attribute == GetWeightAttribute())
	{
		bChange = (NewValue > OldValue);
		UE_LOG(LogA1Player, Log, TEXT("Weight : %f -> %f"), OldValue, NewValue);

		if (bChange && GetWeight() >= GetMaxWeight() / 2)
			NoticeIndex = 1;
		else if (!bChange && GetWeight() < GetMaxHealth() / 2)
			bHalf[1] = true;
	}
	else if (Attribute == GetOxygenAttribute())
	{
		UE_LOG(LogA1Player, Log, TEXT("Oxygen : %f -> %f"), OldValue, NewValue);

		if (bChange && GetOxygen() <= GetMaxOxygen() / 2)
			NoticeIndex = 0;
		else if (!bChange && GetOxygen() > GetMaxOxygen() / 2)
			bHalf[0] = true;
	}
	else if (Attribute == GetHungerAttribute())
	{
		UE_LOG(LogA1Player, Log, TEXT("Hunger : %f -> %f"), OldValue, NewValue);

		if (bChange && GetHunger() <= GetMaxHunger() / 2)
			NoticeIndex = 3;
		else if (!bChange && GetHunger() > GetMaxHunger() / 2)
			bHalf[3] = true;
	}		

	// 50퍼 이하로 감소될 때 한번만 실행되도록
	if (bChange && NoticeIndex >= 0 && bHalf[NoticeIndex])
	{
		OnNoticeWarning.Broadcast("System", NoticeIndex);
		bHalf[NoticeIndex] = false;
	}
}

void UA1CharacterAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	float MinimumHealth = 0.f;
	AActor* Instigator = Data.EffectSpec.GetContext().GetInstigator();
	
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		UE_LOG(LogA1Player, Warning, TEXT("Direct Health Access : %f"), GetHealth());
		SetHealth(FMath::Clamp(GetHealth(), MinimumHealth, GetMaxHealth()));
	}
	else if (Data.EvaluatedData.Attribute == GetDamageAttribute())
	{
		OnHealthChanged.Broadcast(Instigator, GetHealth(), GetHealth() - GetDamage());
		
		UE_LOG(LogA1Player, Log, TEXT("Damage : %f"), GetDamage());
		SetHealth(FMath::Clamp(GetHealth() - GetDamage(), MinimumHealth, GetMaxHealth()));
		SetDamage(0.f);
	}
}
