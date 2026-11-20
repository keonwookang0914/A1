#include "Interaction/Abilities/A1GameplayAbility_Interact.h"

#include "AbilitySystemComponent.h"
#include "A1GameplayTags.h"
#include "AbilitySystem/Abilities/Tasks/A1AbilityTask_WaitInputStart.h"
#include "Character/LyraCharacter.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Interaction/A1InteractionQuery.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Interaction/A1InteractionInfo.h"
#include "Physics/LyraCollisionChannels.h"
#include "Tasks/A1AbilityTask_GrantNearbyInteraction.h"
#include "Tasks/A1AbilityTask_WaitForInteractableTraceHit.h"
#include "UI/IndicatorSystem/LyraIndicatorManagerComponent.h"

//#include UE_INLINE_GENERATED_CPP_BY_NAME(A1GamePlayAbility_Interact) -> 이거 때문에 Link Error 발생.

UA1GameplayAbility_Interact::UA1GameplayAbility_Interact(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ActivationPolicy = ELyraAbilityActivationPolicy::OnSpawn;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UA1GameplayAbility_Interact::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	FA1InteractionQuery InteractionQuery;
	InteractionQuery.RequestingAvatar = GetAvatarActorFromActorInfo();
	InteractionQuery.RequestingController = GetControllerFromActorInfo();

	if (UA1AbilityTask_WaitForInteractableTraceHit* TraceHitTask = UA1AbilityTask_WaitForInteractableTraceHit::WaitForInteractableTraceHit(this, InteractionQuery, ECC_GameTraceChannel12, MakeTargetLocationInfoFromOwnerActor(), InteractionTraceRange, InteractionTraceRate, bShowTraceDebug))
	{
		TraceHitTask->InteractableChanged.AddDynamic(this, &ThisClass::UpdateInteractions);
		TraceHitTask->ReadyForActivation();
	}

	UAbilitySystemComponent* AbilitySystem = GetAbilitySystemComponentFromActorInfo();
	if (AbilitySystem && AbilitySystem->GetOwnerRole() == ROLE_Authority)
	{
		UA1AbilityTask_GrantNearbyInteraction* GrantAbilityTask = UA1AbilityTask_GrantNearbyInteraction::GrantAbilitiesForNearbyInteractables(this, InteractionScanRange, InteractionScanRate);
		GrantAbilityTask->ReadyForActivation();
	}

	WaitInputStart();
}

void UA1GameplayAbility_Interact::UpdateInteractions(const TArray<FA1InteractionInfo>& InteractionInfos)
{
	FA1InteractionMessage Message;
	Message.Instigator = GetAvatarActorFromActorInfo();
	Message.bShouldRefresh = true;
	Message.bSwitchActive = (GetAbilitySystemComponentFromActorInfo()->HasMatchingGameplayTag(A1GameplayTags::Status_Interact) == false);
	Message.InteractionInfo = InteractionInfos.Num() > 0 ? InteractionInfos[0] : FA1InteractionInfo();

	UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetAvatarActorFromActorInfo());
	MessageSystem.BroadcastMessage(A1GameplayTags::Message_Interaction_Notice, Message);

	CurrentInteractionInfos = InteractionInfos;
}

void UA1GameplayAbility_Interact::TriggerInteraction()
{
	if (CurrentInteractionInfos.Num() == 0)
		return;

	ALyraCharacter* LyraCharacter = Cast<ALyraCharacter>(GetAvatarActorFromActorInfo());
	if (LyraCharacter && LyraCharacter->GetMovementComponent()->IsFalling())
		return;

	if (GetAbilitySystemComponentFromActorInfo())
	{
		const FA1InteractionInfo& InteractionInfo = CurrentInteractionInfos[0];

		AActor* Instigator = GetAvatarActorFromActorInfo();
		AActor* InteractableActor = nullptr;

		if (UObject* Object = InteractionInfo.Interactable.GetObject())
		{
			if (AActor* Actor = Cast<AActor>(Object))
			{
				InteractableActor = Actor;
			}
			else if (UActorComponent* ActorComponent = Cast<UActorComponent>(Object))
			{
				InteractableActor = ActorComponent->GetOwner();
			}
		}

		FGameplayEventData Payload;
		Payload.EventTag = A1GameplayTags::Ability_Interact_Active;
		Payload.Instigator = Instigator;
		Payload.Target = InteractableActor;

		SendGameplayEvent(A1GameplayTags::Ability_Interact_Active, Payload);
	}
}

void UA1GameplayAbility_Interact::WaitInputStart()
{
	if (UA1AbilityTask_WaitInputStart* InputStartTask = UA1AbilityTask_WaitInputStart::WaitInputStart(this))
	{
		InputStartTask->OnStart.AddDynamic(this, &ThisClass::OnInputStart);
		InputStartTask->ReadyForActivation();
	}
}

void UA1GameplayAbility_Interact::OnInputStart()
{
	TriggerInteraction();
	WaitInputStart();
}
