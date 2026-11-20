#include "Interaction/Abilities/Tasks/A1AbilityTask_WaitForInteractableTraceHit.h"

#include "AbilitySystemComponent.h"
#include "Interaction/A1Interactable.h"
#include "Interaction/A1InteractionInfo.h"

UA1AbilityTask_WaitForInteractableTraceHit::UA1AbilityTask_WaitForInteractableTraceHit(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

UA1AbilityTask_WaitForInteractableTraceHit* UA1AbilityTask_WaitForInteractableTraceHit::WaitForInteractableTraceHit(UGameplayAbility* OwningAbility, FA1InteractionQuery InteractionQuery, ECollisionChannel TraceChannel,FGameplayAbilityTargetingLocationInfo StartLocation, float InteractionTraceRange, float InteractionTraceRate,bool bShowDebug)
{
	UA1AbilityTask_WaitForInteractableTraceHit* Task = NewAbilityTask<UA1AbilityTask_WaitForInteractableTraceHit>(OwningAbility);
	//Setting AbilityTask

	Task->InteractionTraceRange = InteractionTraceRange;
	Task->InteractionTraceRate = InteractionTraceRate;
	Task->StartLocation = StartLocation;
	Task->InteractionQuery = InteractionQuery;
	Task->TraceChannel = TraceChannel;
	Task->bShowDebug = bShowDebug;

	return Task;
}

void UA1AbilityTask_WaitForInteractableTraceHit::Activate()
{
	Super::Activate();

	SetWaitingOnAvatar();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(TraceTimerHandle, this, &ThisClass::PerformTrace, InteractionTraceRate, true);
	}
}

void UA1AbilityTask_WaitForInteractableTraceHit::OnDestroy(bool bInOwnerFinished)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TraceTimerHandle);
	}

	Super::OnDestroy(bInOwnerFinished);
}

void UA1AbilityTask_WaitForInteractableTraceHit::PerformTrace()
{
	AActor* AvatarActor = Ability->GetCurrentActorInfo()->AvatarActor.Get();
	if (AvatarActor == nullptr)
		return;

	APlayerController* PlayerController = Ability->GetCurrentActorInfo()->PlayerController.Get();
	if (PlayerController == nullptr)
		return;

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(AvatarActor);
	AvatarActor->GetAttachedActors(ActorsToIgnore, false, true);

	FCollisionQueryParams Params(SCENE_QUERY_STAT(A1AbilityTask_WaitForInteractableTraceHit), false);
	Params.AddIgnoredActors(ActorsToIgnore);

	FVector TraceStart;
	FRotator TraceRotation;
	PlayerController->GetPlayerViewPoint(TraceStart, TraceRotation);
	FVector TraceEnd;
	AimWithPlayerController(AvatarActor, Params, TraceStart, InteractionTraceRange, TraceEnd);

	FHitResult HitResult;
	LineTrace(TraceStart, TraceEnd, Params, HitResult);

	TArray<TScriptInterface<IA1Interactable>> Interactables;
	TScriptInterface<IA1Interactable> InteractableActor(HitResult.GetActor());
	if (InteractableActor)
	{
		Interactables.AddUnique(InteractableActor);
	}

	TScriptInterface<IA1Interactable> InteractableComponent(HitResult.GetComponent());
	if (InteractableComponent)
	{
		Interactables.AddUnique(InteractableComponent);
	}

	UpdateInteractionInfos(InteractionQuery, Interactables);

#if ENABLE_DRAW_DEBUG
	if (bShowDebug)
	{
		FColor DebugColor = HitResult.bBlockingHit ? FColor::Red : FColor::Green;
		if (HitResult.bBlockingHit)
		{
			DrawDebugLine(GetWorld(), TraceStart, HitResult.Location, DebugColor, false, InteractionTraceRate);
			DrawDebugSphere(GetWorld(), HitResult.Location, 5.f, 16, DebugColor, false, InteractionTraceRate);
		}
		else
		{
			DrawDebugLine(GetWorld(), TraceStart, TraceEnd, DebugColor, false, InteractionTraceRate);
		}
	}
#endif
}

void UA1AbilityTask_WaitForInteractableTraceHit::AimWithPlayerController(const AActor* InSourceActor,
	FCollisionQueryParams Params, const FVector& TraceStart, float MaxRange, FVector& OutTraceEnd,
	bool bIgnorePitch) const
{
	if (Ability == nullptr)
		return;

	APlayerController* PlayerController = Ability->GetCurrentActorInfo()->PlayerController.Get();
	if (PlayerController == nullptr)
		return;

	FVector CameraStart;
	FRotator CameraRotation;
	PlayerController->GetPlayerViewPoint(CameraStart, CameraRotation);

	const FVector CameraDirection = CameraRotation.Vector();
	FVector CameraEnd = CameraStart + (CameraDirection * MaxRange);

	//Restricts the ray in the direction of the camera to within the interactable range (sphere) based on the player's position.
	ClipCameraRayToAbilityRange(CameraStart, CameraDirection, TraceStart, MaxRange, CameraEnd);

	FHitResult HitResult;
	LineTrace(CameraStart, CameraEnd, Params, HitResult);

	//1. If the hit object is within the interactable range(Sphere), the hit location is set to the TraceEnd location.
	//2. If there is no object hit or the object hit is outside the interactable range (Sphere), the hit location is ignored and CameraEnd is set to TraceEnd.
	// - Afterwards, check the objects between the player and CameraEnd.
	const bool bUseTraceResult = HitResult.bBlockingHit && (FVector::DistSquared(TraceStart, HitResult.Location) <= (MaxRange * MaxRange));
	const FVector AdjustedEnd = bUseTraceResult ? HitResult.Location : CameraEnd;

	FVector AdjustedAimDir = (AdjustedEnd - TraceStart).GetSafeNormal();
	if (AdjustedAimDir.IsZero())
	{
		AdjustedAimDir = CameraDirection;
	}

	// Extend TraceEnd to the maximum interactable location(the surface of the Sphere).
	OutTraceEnd = TraceStart + (AdjustedAimDir * MaxRange);
}

bool UA1AbilityTask_WaitForInteractableTraceHit::ClipCameraRayToAbilityRange(FVector CameraLocation, FVector CameraDirection, FVector AbilityCenter, float AbilityRange, FVector& OutClippedPosition) const
{
	FVector CameraToCenter = AbilityCenter - CameraLocation;
	float DistanceCameraToDot = FVector::DotProduct(CameraToCenter, CameraDirection);
	if (DistanceCameraToDot >= 0)
	{
		float DistanceSquared = CameraToCenter.SizeSquared() - (DistanceCameraToDot * DistanceCameraToDot);
		float RadiusSquared = (AbilityRange * AbilityRange);
		if (DistanceSquared <= RadiusSquared)
		{
			float DistanceDotToSphere = FMath::Sqrt(RadiusSquared - DistanceSquared);
			float DistanceCameraToSphere = DistanceCameraToDot + DistanceDotToSphere;
			OutClippedPosition = CameraLocation + (DistanceCameraToSphere * CameraDirection);
			return true;
		}
	}
	return false;
}

void UA1AbilityTask_WaitForInteractableTraceHit::LineTrace(const FVector& Start, const FVector& End,
	const FCollisionQueryParams& Params, FHitResult& OutHitResult) const
{
	TArray<FHitResult> HitResults;
	GetWorld()->LineTraceMultiByChannel(HitResults, Start, End, TraceChannel, Params);

	if (HitResults.Num() > 0)
	{
		OutHitResult = HitResults[0];
	}
	else
	{
		OutHitResult = FHitResult();
		OutHitResult.TraceStart = Start;
		OutHitResult.TraceEnd = End;
	}
}

void UA1AbilityTask_WaitForInteractableTraceHit::UpdateInteractionInfos(const FA1InteractionQuery& InteractQuery,
	const TArray<TScriptInterface<IA1Interactable>>& Interactables)
{
	TArray<FA1InteractionInfo> NewInteractionInfos;

	for (const TScriptInterface<IA1Interactable>& Interactable : Interactables)
	{
		TArray<FA1InteractionInfo> TempInteractionInfos;
		FA1InteractionInfoBuilder InteractionInfoBuilder(Interactable, TempInteractionInfos);
		Interactable->GatherPostInteractionInfos(InteractQuery, InteractionInfoBuilder);

		for (FA1InteractionInfo& InteractionInfo : TempInteractionInfos)
		{
			if (InteractionInfo.AbilityToGrant)
			{
				FGameplayAbilitySpec* InteractionAbilitySpec = AbilitySystemComponent->FindAbilitySpecFromClass(InteractionInfo.AbilityToGrant);
				if (InteractionAbilitySpec)
				{
					if (Interactable->CanInteraction(InteractionQuery) && InteractionAbilitySpec->Ability->CanActivateAbility(InteractionAbilitySpec->Handle, AbilitySystemComponent->AbilityActorInfo.Get()))
					{
						NewInteractionInfos.Add(InteractionInfo);
					}
				}
			}
		}
	}

	bool bInfosChanged = false;
	if (NewInteractionInfos.Num() == CurrentInteractionInfos.Num())
	{
		NewInteractionInfos.Sort();

		for (int InfoIndex = 0; InfoIndex < NewInteractionInfos.Num(); InfoIndex++)
		{
			const FA1InteractionInfo& NewInfo = NewInteractionInfos[InfoIndex];
			const FA1InteractionInfo& CurrentInfo = CurrentInteractionInfos[InfoIndex];

			if (NewInfo != CurrentInfo)
			{
				bInfosChanged = true;
				break;
			}
		}
	}
	else
	{
		bInfosChanged = true;
	}

	if (bInfosChanged)
	{
		HighlightInteractables(CurrentInteractionInfos, false);
		CurrentInteractionInfos = NewInteractionInfos;
		HighlightInteractables(CurrentInteractionInfos, true);

		InteractableChanged.Broadcast(CurrentInteractionInfos);
	}
}

void UA1AbilityTask_WaitForInteractableTraceHit::HighlightInteractables(
	const TArray<FA1InteractionInfo>& InteractionInfos, bool bShouldHighlight)
{
	TArray<UMeshComponent*> MeshComponents;
	for (const FA1InteractionInfo& InteractionInfo : InteractionInfos)
	{
		if (IA1Interactable* Interactable = InteractionInfo.Interactable.GetInterface())
		{
			Interactable->GetMeshComponents(MeshComponents);
		}
	}

	for (UMeshComponent* MeshComponent : MeshComponents)
	{
		MeshComponent->SetRenderCustomDepth(bShouldHighlight);
	}
}
