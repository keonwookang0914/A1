// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#include "AbilitySystem/Abilities/Docker/A1GameplayAbility_TryInteract.h"

#include "A1GameplayTags.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Physics/LyraCollisionChannels.h"
#include "Player/LyraPlayerController.h"
#include "Character/LyraCharacter.h"
#include "Character/A1CreatureBase.h"
#include "Item/Managers/A1ItemManagerComponent.h"
#include "Actors/A1EquipmentBase.h"
#include "Interaction/A1InteractionQuery.h"
#include "Interaction/A1WorldInteractable.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1GameplayAbility_TryInteract)

UA1GameplayAbility_TryInteract::UA1GameplayAbility_TryInteract(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ActivationRequiredTags.AddTag(A1GameplayTags::Status_TryInteract);
	ActivationBlockedTags.AddTag(A1GameplayTags::GameplayEvent_Inventory_Open);
}

void UA1GameplayAbility_TryInteract::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (K2_CheckAbilityCooldown() == false || K2_CheckAbilityCost() == false)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	if (CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo) == false)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	if (HasAuthority(&CurrentActivationInfo))
	{
		TryPickup();
	}
}

void UA1GameplayAbility_TryInteract::TryPickup()
{
	if (HasAuthority(&CurrentActivationInfo) == false)
		return;

	ALyraCharacter* LyraCharacter = GetLyraCharacterFromActorInfo();
	ALyraPlayerController* LyraPlayerController = GetLyraPlayerControllerFromActorInfo();

	if (LyraCharacter && LyraPlayerController)
	{
		FVector CameraLocation;
		FRotator CameraRotation;
		LyraPlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);

		FVector StartLocation = CameraLocation;
		FVector EndLocation = StartLocation + (CameraRotation.Vector() * MaxDistance);

		FHitResult HitResult;
		TArray<AActor*> ActorsToIgnore = { LyraCharacter };

		bool bHit = UKismetSystemLibrary::LineTraceSingle(GetWorld(), StartLocation, EndLocation, UEngineTypes::ConvertToTraceType(A1_TraceChannel_AimAssist), false, ActorsToIgnore, EDrawDebugTrace::ForDuration, HitResult, true);

		if (bHit)
		{
			AA1EquipmentBase* Target = Cast<AA1EquipmentBase>(HitResult.GetActor());
			if (Target)
			{
				Target->OnItemPickupChanged.Broadcast();
				Target->OnItemPickupChanged.Clear();

				UA1ItemManagerComponent* ItemManager = LyraPlayerController->GetComponentByClass<UA1ItemManagerComponent>();
				ItemManager->TryPickItem(Target);
			}

			/*
			 * TODO eric1013
			 * Test this Code
			 */

			AA1WorldInteractable* InteractableTarget = Cast<AA1WorldInteractable>(HitResult.GetActor());
			if (InteractableTarget)
			{
				FA1InteractionQuery InteractionQuery;
				InteractionQuery.RequestingAvatar = LyraCharacter;
				InteractionQuery.RequestingController = LyraPlayerController;

				if (InteractableTarget->CanInteraction(InteractionQuery))
				{
					FGameplayEventData Payload;
					Payload.EventTag = A1GameplayTags::Ability_Interact_Active;
					Payload.Instigator = LyraCharacter;
					Payload.Target = InteractableTarget;

					UAbilitySystemComponent* AbilitySystem = LyraCharacter->GetAbilitySystemComponent();
					if (AbilitySystem)
					{
						AbilitySystem->HandleGameplayEvent(A1GameplayTags::Ability_Interact_Active, &Payload);
					}
				}
			}
		}
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
