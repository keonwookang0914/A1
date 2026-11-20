// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "Interaction/Abilities/A1GameplayAbility_Interact_Cmd.h"

#include "A1GameplayTags.h"
#include "A1LogChannels.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Actors/A1CMDBase.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Player/LyraPlayerController.h"
#include "UIExtensionSystem.h"
#include "Kismet/GameplayStatics.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1GameplayAbility_Interact_Cmd)

UA1GameplayAbility_Interact_Cmd::UA1GameplayAbility_Interact_Cmd()
{
}

void UA1GameplayAbility_Interact_Cmd::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	UE_LOG(LogTemp, Log, TEXT("Activate Ability!"));

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
	
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	ExitWidgetTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, A1GameplayTags::GameplayEvent_Cmd_Exit, nullptr, false, true);
	if (ExitWidgetTask)
	{
		ExitWidgetTask->EventReceived.AddDynamic(this, &ThisClass::CloseCmd);
		ExitWidgetTask->ReadyForActivation();
	}

	// CMD 오픈에 따른 UI 비활성화
	FA1WidgetActiveMessage Message;
	Message.bActive = false;

	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
	MessageSubsystem.BroadcastMessage(A1GameplayTags::Message_HUD_Active, Message);

	// 시점 전환
	ALyraPlayerController* LyraPlayerController = GetLyraPlayerControllerFromActorInfo();
	if (LyraPlayerController == nullptr)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	if (AA1CMDBase* CMD = Cast<AA1CMDBase>(InteractableActor))
	{
		LyraPlayerController->SetViewTargetWithBlend(CMD, 0.5f, EViewTargetBlendFunction::VTBlend_Linear);
	}
}

void UA1GameplayAbility_Interact_Cmd::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UA1GameplayAbility_Interact_Cmd::CloseCmd(FGameplayEventData Payload)
{
	// 시점 전환
	ALyraPlayerController* LyraPlayerController = GetLyraPlayerControllerFromActorInfo();
	if (LyraPlayerController == nullptr)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}
	AActor* LyraCharacter = Cast<AActor>(CurrentActorInfo->AvatarActor.Get());

	LyraPlayerController->SetViewTargetWithBlend(LyraCharacter, 0.5f, EViewTargetBlendFunction::VTBlend_Linear);

	DeactiveWidget();
	
	// CMD 오픈에 따른 UI 활성화
	FA1WidgetActiveMessage Message;
	Message.bActive = true;

	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
	MessageSubsystem.BroadcastMessage(A1GameplayTags::Message_HUD_Active, Message);
}
