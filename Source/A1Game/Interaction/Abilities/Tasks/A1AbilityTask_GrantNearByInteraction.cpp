#include "Interaction/Abilities/Tasks/A1AbilityTask_GrantNearbyInteraction.h"

#include "Interaction/A1Interactable.h"
#include "AbilitySystemComponent.h"
#include "Engine/OverlapResult.h"
#include "Interaction/A1Interactable.h"
#include "Interaction/A1InteractionQuery.h"
#include "Physics/LyraCollisionChannels.h"

UA1AbilityTask_GrantNearbyInteraction* UA1AbilityTask_GrantNearbyInteraction::GrantAbilitiesForNearbyInteractables(
	UGameplayAbility* OwningAbility, float InteractionAbilityScanRange, float InteractionAbilityScanRate)
{
	UA1AbilityTask_GrantNearbyInteraction* Task = NewAbilityTask<UA1AbilityTask_GrantNearbyInteraction>(OwningAbility);

	Task->InteractionAbilityScanRange = InteractionAbilityScanRange;
	Task->InteractionAbilityScanRate = InteractionAbilityScanRate;

	return Task;
}

void UA1AbilityTask_GrantNearbyInteraction::Activate()
{
	Super::Activate();

	SetWaitingOnAvatar();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(QueryTimerHandle, this, &ThisClass::QueryInteractables, InteractionAbilityScanRate, true);
	}
}

void UA1AbilityTask_GrantNearbyInteraction::OnDestroy(bool bInOwnerFinished)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(QueryTimerHandle);
	}

	Super::OnDestroy(bInOwnerFinished);
}

void UA1AbilityTask_GrantNearbyInteraction::QueryInteractables()
{
	UWorld* World = GetWorld();
	AActor* AvatarActor = GetAvatarActor();

	if (World && AvatarActor)
	{
		TSet<FObjectKey> RemoveKeys;
		GrantedInteractionAbilities.GetKeys(RemoveKeys);

		FCollisionQueryParams Params(SCENE_QUERY_STAT(UA1AbilityTask_GrantNearbyInteraction), false);

		APlayerController* PlayerController = Ability->GetCurrentActorInfo()->PlayerController.Get();
		if (PlayerController == nullptr)
			return;

		FVector CameraStart;
		FRotator CameraRotation;
		PlayerController->GetPlayerViewPoint(CameraStart, CameraRotation);
		const FVector CameraDirection = CameraRotation.Vector();
		FVector CameraEnd = CameraStart + (CameraDirection * 1000.f); //Temp eric1306 hardcoding, after FP, must fix distance
		

		FA1InteractionQuery InteractionQuery;
		InteractionQuery.RequestingAvatar = AvatarActor;
		InteractionQuery.RequestingController = Cast<AController>(AvatarActor->GetOwner());

		TArray<FHitResult> HitResults;
		World->LineTraceMultiByChannel(OUT HitResults, CameraStart, CameraEnd, A1_TraceChannel_Interaction, Params);
		if (HitResults.Num() > 0)
		{
			TArray<TScriptInterface<IA1Interactable>> Interactables;
			for (const FHitResult& HitResult : HitResults)
			{
				TScriptInterface<IA1Interactable> InteractableActor(HitResult.GetActor());
				
				if (InteractableActor)
				{
					if (InteractableActor->CanInteraction(InteractionQuery))
					{
						Interactables.AddUnique(InteractableActor);
					}
				}

				TScriptInterface<IA1Interactable> InteractableComponent(HitResult.GetComponent());		
				if (InteractableComponent)
				{
					if (InteractableComponent->CanInteraction(InteractionQuery))
					{
						Interactables.AddUnique(InteractableComponent);
					}
				}
			}

			TArray<FA1InteractionInfo> InteractionInfos;
			for (TScriptInterface<IA1Interactable>& Interactable : Interactables)
			{
				FA1InteractionInfoBuilder InteractionInfoBuilder(Interactable, InteractionInfos);
				Interactable->GatherPostInteractionInfos(InteractionQuery, InteractionInfoBuilder);
			}

			for (FA1InteractionInfo& InteractionInfo : InteractionInfos)
			{
				if (InteractionInfo.AbilityToGrant)
				{
					FObjectKey ObjectKey(InteractionInfo.AbilityToGrant);
					if (GrantedInteractionAbilities.Find(ObjectKey))
					{
						RemoveKeys.Remove(ObjectKey);
					}
					else
					{
						FGameplayAbilitySpec Spec(InteractionInfo.AbilityToGrant, 1, INDEX_NONE, this);
						FGameplayAbilitySpecHandle SpecHandle = AbilitySystemComponent->GiveAbility(Spec);
						GrantedInteractionAbilities.Add(ObjectKey, SpecHandle);
					}
				}
			}
		}

		for (const FObjectKey& RemoveKey : RemoveKeys)
		{
			AbilitySystemComponent->ClearAbility(GrantedInteractionAbilities[RemoveKey]);
			GrantedInteractionAbilities.Remove(RemoveKey);
		}
	}
}
