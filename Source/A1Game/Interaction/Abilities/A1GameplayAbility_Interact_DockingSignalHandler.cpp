// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "Interaction/Abilities/A1GameplayAbility_Interact_DockingSignalHandler.h"

#include "Actors/A1DockingSignalHandlerBase.h"
#include "Actors/A1SignalDetectionBase.h"
#include "Kismet/GameplayStatics.h"
#include "Maps/A1RandomMapGenerator.h"
#include "Score/A1ScoreManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1GameplayAbility_Interact_DockingSignalHandler)

UA1GameplayAbility_Interact_DockingSignalHandler::UA1GameplayAbility_Interact_DockingSignalHandler(
	const FObjectInitializer& ObjectInitializer)
{
	
}

void UA1GameplayAbility_Interact_DockingSignalHandler::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (TriggerEventData == nullptr || bInitialized == false)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	if (HasAuthority(&CurrentActivationInfo) == false)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	AA1DockingSignalHandlerBase* DockingSignalHandlerActor = Cast<AA1DockingSignalHandlerBase>(InteractableActor);
	if (DockingSignalHandlerActor == nullptr)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	AA1SpaceshipBase* Spaceship = DockingSignalHandlerActor->GetOwningSpaceship();
	if (!IsValid(Spaceship))
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}
	//이미 맵이 활성화 되어있는 경우
	if (Spaceship->GetIsExternalMapActive())
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}
	// 시그널 핸들러 사용이 불가능한지
	if (!Spaceship->GetCanUseDockingSignalHandler())
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	AActor* OwningActor = ActorInfo->AvatarActor.Get();
	if (OwningActor)
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AA1RandomMapGenerator::StaticClass(), FoundActors);

		for (AActor* Actor : FoundActors)
		{
			AA1RandomMapGenerator* TargetActor = Cast<AA1RandomMapGenerator>(Actor);
			if (TargetActor)
			{
				TargetActor->StartRandomMap();
				break;
			}
		}
		
		//Stop PlaySound
		Spaceship->GetSignalDetection()->SetSignalDetectionState(ESignalDetectionState::None);
		Spaceship->GetSignalDetection()->StopDetectSignal();
	}


	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
