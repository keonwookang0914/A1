// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "Camera/LyraCameraMode_FirstPerson.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Engine/Canvas.h"
#include "GameFramework/Pawn.h"
#include "Components/CapsuleComponent.h"
#include "Engine/Engine.h"
#include "Score/A1ScoreManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraCameraMode_FirstPerson)

ULyraCameraMode_FirstPerson::ULyraCameraMode_FirstPerson()
{
	FieldOfView = 90.0f;

	BlendTime = 0.1f;
	BlendFunction = ELyraCameraModeBlendFunction::EaseOut;

	ViewPitchMin = -85.0f;
	ViewPitchMax = 85.0f;
}

void ULyraCameraMode_FirstPerson::UpdateView(float DeltaTime)
{
	if (AActor* TargetActor = GetTargetActor())
	{
		if (APawn* TargetPawn = Cast<APawn>(TargetActor))
		{
			bool bIsSprinting = false;
			if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetPawn))
			{
				FGameplayTag SprintTag = FGameplayTag::RequestGameplayTag(FName("Status.Sprint"));
				bIsSprinting = ASC->HasMatchingGameplayTag(SprintTag);
			}

			float TargetSprintOffset = bIsSprinting ? SprintEyeHeightOffsetX : 0.0f;
			CurrentSprintOffset = FMath::FInterpTo(CurrentSprintOffset, TargetSprintOffset, DeltaTime, SprintOffsetInterpSpeed);

			FVector PivotLocation = TargetActor->GetActorLocation();
			FRotator PivotRotation = TargetPawn->GetViewRotation();

			PivotRotation.Roll = 0.0f;
			PivotRotation.Pitch = FMath::ClampAngle(PivotRotation.Pitch, ViewPitchMin, ViewPitchMax);

			View.Location = PivotLocation;
			if (!bCanRotate )
			{
				PivotRotation.Yaw += 180.f;
				bCanRotate = true;
			}
			View.Rotation = PivotRotation;
			View.ControlRotation = View.Rotation;
			
			View.FieldOfView = FieldOfView;

			FVector AdjustedEyeHeightOffset = EyeHeightOffset;
			AdjustedEyeHeightOffset.X += CurrentSprintOffset;

			View.Location += View.Rotation.RotateVector(AdjustedEyeHeightOffset);
		}
	}
}

FVector ULyraCameraMode_FirstPerson::CalculateHeadBob(float DeltaTime)
{
	if (!bApplyHeadBob)
	{
		return FVector::ZeroVector;
	}

	const APawn* TargetPawn = Cast<APawn>(GetTargetActor());
	if (!TargetPawn || TargetPawn->GetVelocity().SizeSquared() < 100.0f)
	{
		HeadBobTime = FMath::Max(0.0f, HeadBobTime - DeltaTime * 2.0f);
		if (HeadBobTime <= 0.0f)
		{
			return FVector::ZeroVector;
		}
	}
	else
	{
		float MovementSpeed = TargetPawn->GetVelocity().Size();
		float SpeedMultiplier = FMath::Clamp(MovementSpeed / 1000.0f, 0.5f, 1.0f);
		HeadBobTime += DeltaTime * HeadBobFrequency * SpeedMultiplier;
	}

	
	float VerticalBob = FMath::Sin(HeadBobTime) * HeadBobIntensity;
	float HorizontalBob = FMath::Sin(HeadBobTime * 2.0f) * HeadBobIntensity * 0.5f; 

	return FVector(0.0f, HorizontalBob, VerticalBob);
}