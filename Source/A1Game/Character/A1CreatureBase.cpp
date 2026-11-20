// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "Character/A1CreatureBase.h"
#include "AbilitySystemComponent.h"
#include "Controller/Player/A1PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1CreatureBase)

// Sets default values
AA1CreatureBase::AA1CreatureBase(const FObjectInitializer& ObjectInitializer)
{
	ASC = nullptr;
}

UAbilitySystemComponent* AA1CreatureBase::GetAbilitySystemComponent() const
{
	return ASC;
}

void AA1CreatureBase::SetDead()
{
	DeatState = EA1DeathState::DeathStarted;

	if (ASC)
	{
		ASC->CancelAllAbilities();
	}

	GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
	GetMesh()->SetAnimInstanceClass(nullptr);
	if (DeadMontage)
	{
		GetMesh()->PlayAnimation(DeadMontage, false);
		if (DeathSound)
			UGameplayStatics::SpawnSoundAttached(DeathSound, GetRootComponent());
	}
	SetActorEnableCollision(false);
}

