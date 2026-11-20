// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#include "A1VitalWidget.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/Attributes/A1CharacterAttributeSet.h"
#include "Components/TextBlock.h"
#include "Character/LyraCharacter.h"
#include "Player/LyraPlayerController.h"

UA1VitalWidget::UA1VitalWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UA1VitalWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
}

void UA1VitalWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ALyraPlayerController* ownerController = Cast<ALyraPlayerController>(GetOwningPlayer());
	if (ownerController)
	{
		ownerController->GetOnNewPawnNotifier().AddUObject(this, &ThisClass::SetAbilitySystemComponent);
	}
}

void UA1VitalWidget::NativeDestruct()
{
	ASC = nullptr;

	Super::NativeDestruct();
}

void UA1VitalWidget::SetAbilitySystemComponent(APawn* InOwner)
{
	if (IsValid(InOwner))
	{
		ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InOwner);
	}

	if (ASC)
	{
		ASC->GetGameplayAttributeValueChangeDelegate(UA1CharacterAttributeSet::GetHealthAttribute()).AddUObject(this, &UA1VitalWidget::OnHealthChanged);
		ASC->GetGameplayAttributeValueChangeDelegate(UA1CharacterAttributeSet::GetMaxHealthAttribute()).AddUObject(this, &UA1VitalWidget::OnMaxHealthChanged);
		ASC->GetGameplayAttributeValueChangeDelegate(UA1CharacterAttributeSet::GetOxygenAttribute()).AddUObject(this, &UA1VitalWidget::OnOxygenChanged);
		ASC->GetGameplayAttributeValueChangeDelegate(UA1CharacterAttributeSet::GetMaxOxygenAttribute()).AddUObject(this, &UA1VitalWidget::OnMaxOxygenChanged);

		const UA1CharacterAttributeSet* CurrentAttributeSet = ASC->GetSet<UA1CharacterAttributeSet>();
		if (CurrentAttributeSet)
		{
			CurrentHealth = CurrentAttributeSet->GetHealth();
			MaxHealth = CurrentAttributeSet->GetMaxHealth();
			CurrentOxygen = CurrentAttributeSet->GetOxygen();
			MaxOxygen = CurrentAttributeSet->GetMaxOxygen();
	
			UpdateText();
		}
	}
}

UAbilitySystemComponent* UA1VitalWidget::GetAbilitySystemComponent() const
{
	return ASC;
}

void UA1VitalWidget::OnHealthChanged(const FOnAttributeChangeData& ChangeData)
{
	CurrentHealth = ChangeData.NewValue;
	UpdateText();
}

void UA1VitalWidget::OnOxygenChanged(const FOnAttributeChangeData& ChangeData)
{
	CurrentOxygen = ChangeData.NewValue;
	UpdateText();
}

void UA1VitalWidget::OnMaxHealthChanged(const FOnAttributeChangeData& ChangeData)
{
	MaxHealth = ChangeData.NewValue;
	UpdateText();
}

void UA1VitalWidget::OnMaxOxygenChanged(const FOnAttributeChangeData& ChangeData)
{
	MaxOxygen = ChangeData.NewValue;
	UpdateText();
}

void UA1VitalWidget::UpdateText()
{

	if (TxtHpStat)
	{
		TxtHpStat->SetText(FText::FromString(FString::Printf(TEXT("%.0f"), CurrentHealth)));
		//TxtHpStat->SetText(FText::FromString(FString::Printf(TEXT("%.0f"), CurrentHealth/MaxHealth)));
	}
	if (TxtOxygenStat)
	{
		TxtOxygenStat->SetText(FText::FromString(FString::Printf(TEXT("%.0f"), CurrentOxygen)));
		//TxtOxygenStat->SetText(FText::FromString(FString::Printf(TEXT("%.0f"), CurrentOxygen/MaxOxygen)));
	}
}
