// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "Controller/Player/A1PlayerState.h"
#include "AbilitySystemComponent.h"

AA1PlayerState::AA1PlayerState()
{
	ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));
	//TODO: Make AttributeSet CDO
}

class UAbilitySystemComponent* AA1PlayerState::GetAbilitySystemComponent() const
{
	return ASC;
}
