// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "Animation/A1AnimInstance.h"
#include "Character/A1PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UA1AnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	Character = Cast<ACharacter>(TryGetPawnOwner());
	if (Character)
		MovementComponent = Character->GetCharacterMovement();
}

void UA1AnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (MovementComponent == nullptr)
		return;
	IsInAir = MovementComponent->IsFalling();
	Speed = Character->GetVelocity().Length();
	Velocity = Character->GetVelocity();

	{
		const FRotator AimRotator = Character->GetBaseAimRotation();
		const FRotator ActorRotator = Character->GetActorRotation();
		const FRotator DiffRotator = UKismetMathLibrary::NormalizedDeltaRotator(AimRotator, ActorRotator);

		Pitch = DiffRotator.Pitch;
		Roll = DiffRotator.Roll;
		Yaw = DiffRotator.Yaw;
	}

	{
		const FRotator CurrentRotation = Character->GetActorRotation();
		const FRotator DiffRotation = UKismetMathLibrary::NormalizedDeltaRotator(RotationLastTick, CurrentRotation);
		float Target = DiffRotation.Yaw / DeltaSeconds / 7.f;
		YawDelta = UKismetMathLibrary::FInterpTo(YawDelta, Target, DeltaSeconds, 6.f);
		RotationLastTick = CurrentRotation;
	}

	IsAccelerating = MovementComponent->GetCurrentAcceleration().Length() > 0;

	FullBody = GetCurveValue("FullBody") > 0;
}
