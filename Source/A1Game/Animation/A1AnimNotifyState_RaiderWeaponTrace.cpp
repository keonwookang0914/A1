// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "Animation/A1AnimNotifyState_RaiderWeaponTrace.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Development/A1DeveloperSettings.h"
#include "KismetTraceUtils.h"
#include "Kismet/KismetMathLibrary.h"
#include "Physics/LyraCollisionChannels.h"

UA1AnimNotifyState_RaiderWeaponTrace::UA1AnimNotifyState_RaiderWeaponTrace(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UA1AnimNotifyState_RaiderWeaponTrace::NotifyBegin(USkeletalMeshComponent* MeshComponent, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComponent, Animation, TotalDuration, EventReference);

	if (MeshComponent->GetOwnerRole() != ExecuteNetRole)
		return;

	PreviousSocketTransform = MeshComponent->GetSocketTransform(TraceSocketName);

#if UE_EDITOR
	check(MeshComponent->DoesSocketExist(TraceSocketName));
#endif
		
	HitActors.Empty();
}

void UA1AnimNotifyState_RaiderWeaponTrace::NotifyTick(USkeletalMeshComponent* MeshComponent, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComponent, Animation, FrameDeltaTime, EventReference);

	if (MeshComponent->GetOwnerRole() != ExecuteNetRole)
		return;

	PerformTrace(MeshComponent);
}

void UA1AnimNotifyState_RaiderWeaponTrace::NotifyEnd(USkeletalMeshComponent* MeshComponent, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComponent, Animation, EventReference);

	if (MeshComponent->GetOwnerRole() != ExecuteNetRole)
		return;

	PerformTrace(MeshComponent);
}

void UA1AnimNotifyState_RaiderWeaponTrace::PerformTrace(USkeletalMeshComponent* MeshComponent)
{
	FTransform CurrentSocketTransform = MeshComponent->GetSocketTransform(TraceSocketName);
	float Distance = (PreviousSocketTransform.GetLocation() - CurrentSocketTransform.GetLocation()).Length();

	int SubStepCount = FMath::CeilToInt(Distance / TargetDistance);
	if (SubStepCount <= 0)
		return;

	float SubstepRatio = 1.f / SubStepCount;

	TArray<FHitResult> FinalHitResults;

	for (int32 i = 0; i < SubStepCount; i++)
	{
		FTransform StartTraceTransform = UKismetMathLibrary::TLerp(PreviousSocketTransform, CurrentSocketTransform, SubstepRatio * i, ELerpInterpolationMode::DualQuatInterp);
		FTransform EndTraceTransform = UKismetMathLibrary::TLerp(PreviousSocketTransform, CurrentSocketTransform, SubstepRatio * (i + 1), ELerpInterpolationMode::DualQuatInterp);
		FTransform AverageTraceTransform = UKismetMathLibrary::TLerp(StartTraceTransform, EndTraceTransform, 0.5f, ELerpInterpolationMode::DualQuatInterp);
		FRotator CapsuleRotator = MeshComponent->GetSocketRotation(TraceSocketName);

		FComponentQueryParams Params = FComponentQueryParams::DefaultComponentQueryParams;
		Params.bReturnPhysicalMaterial = true;

		TArray<AActor*> IgnoredActors = { MeshComponent->GetOwner() };
		Params.AddIgnoredActors(IgnoredActors);

		TArray<FHitResult> HitResults;

		bool bHit = MeshComponent->GetWorld()->SweepMultiByChannel(HitResults, StartTraceTransform.GetLocation(), EndTraceTransform.GetLocation(), 
			CapsuleRotator.Quaternion(), A1_TraceChannel_Raider, FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight), Params);

		for (const FHitResult& HitResult : HitResults)
		{
			AActor* HitActor = HitResult.GetActor();
			if (HitActors.Contains(HitActor) == false)
			{
				HitActors.Add(HitActor);
				FinalHitResults.Add(HitResult);
			}
		}

#if UE_EDITOR
		if (GIsEditor)
		{
			const UA1DeveloperSettings* DeveloperSettings = GetDefault<UA1DeveloperSettings>();
			if (DeveloperSettings->bForceDisableDebugTrace == false && bDrawDebugShape)
			{
				FColor Color = (HitResults.Num() > 0) ? HitColor : TraceColor;

				DrawDebugCapsule(MeshComponent->GetWorld(), AverageTraceTransform.GetLocation(), CapsuleHalfHeight, CapsuleRadius, CapsuleRotator.Quaternion(), Color, false, 1.f);
			}
		}
#endif
	}

	PreviousSocketTransform = CurrentSocketTransform;

	if (FinalHitResults.Num() > 0)
	{
		FGameplayAbilityTargetDataHandle TargetDataHandle;
		
		for (const FHitResult& HitResult : FinalHitResults)
		{
			FGameplayAbilityTargetData_SingleTargetHit* NewTargetData = new FGameplayAbilityTargetData_SingleTargetHit();
			NewTargetData->HitResult = HitResult;
			TargetDataHandle.Add(NewTargetData);
		}
		
		FGameplayEventData EventData;
		EventData.TargetData = TargetDataHandle;
		EventData.Instigator = MeshComponent->GetOwner();
		
		if (EventTag.IsValid())
		{
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(MeshComponent->GetOwner(), EventTag, EventData);
		}
	}
}
