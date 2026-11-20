// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffectTypes.h"
#include "A1VitalWidget.generated.h"

class UAbilitySystemComponent;

UCLASS()
class A1GAME_API UA1VitalWidget : public UUserWidget, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	UA1VitalWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

public:
	virtual void SetAbilitySystemComponent(APawn* InOwner);
	virtual class UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	virtual void OnHealthChanged(const FOnAttributeChangeData& ChangeData);
	virtual void OnMaxHealthChanged(const FOnAttributeChangeData& ChangeData);
	virtual void OnOxygenChanged(const FOnAttributeChangeData& ChangeData);
	virtual void OnMaxOxygenChanged(const FOnAttributeChangeData& ChangeData);

	void UpdateText();

protected:
	UPROPERTY(EditAnywhere, Category = GAS)
	TObjectPtr<UAbilitySystemComponent> ASC;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> TxtHpStat;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> TxtOxygenStat;

	float CurrentHealth = 0.1f;
	float MaxHealth = 0.1f;
	float CurrentOxygen = 0.1f;
	float MaxOxygen = 0.1f;
	
};
