// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#include "A1TutorialStep.h"

#include "A1GameplayTags.h"
#include "A1LogChannels.h"
#include "A1TutorialManager.h"
#include "EngineUtils.h"
#include "Data/A1TutorialData.h"
#include "GameModes/LyraGameMode.h"
#include "Character/LyraCharacter.h"
#include "LevelSequence.h"
#include "LevelSequencePlayer.h"
#include "LevelSequenceActor.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "Actors/A1BedBase.h"
#include "Actors/A1EquipmentBase.h"
#include "Actors/A1RepairBase.h"
#include "Camera/LyraCameraComponent.h"
#include "Camera/LyraCameraMode.h"
#include "Camera/LyraCameraMode_FirstPerson.h"
#include "Data/A1ItemData.h"
#include "Interaction/A1InteractionQuery.h"
#include "Interaction/Abilities/A1GameplayAbility_Interact_Cmd.h"
#include "Item/A1ItemInstance.h"
#include "Item/Fragments/A1ItemFragment_Equipable_Attachment.h"
#include "Kismet/GameplayStatics.h"
#include "Player/LyraPlayerController.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1TutorialStep)

UA1TutorialStep::UA1TutorialStep()
{
	bIsCompleted = false;
	bIsActive = false;
	CurrentActionIndex = 0;
	bCurrentActionCompleted = false;
	bWaitingForDialogueInput = false;
	bWaitingForEvent = false;
	bPlayerMovementWasDisabled = false;
}

void UA1TutorialStep::Initialize(UA1TutorialManager* InManager, const FA1TutorialStepInfo& InStepInfo)
{
	TutorialManager = InManager;
	StepInfo = InStepInfo;
	bIsCompleted = false;
	bIsActive = false;
	CurrentActionIndex = 0;
	bCurrentActionCompleted = false;
	bWaitingForDialogueInput = false;
	bWaitingForEvent = false;
	bPlayerMovementWasDisabled = false;
}

void UA1TutorialStep::StartStep()
{
	if ( bIsActive )
		return;

	bIsActive = true;
	bIsCompleted = false;
	CurrentActionIndex = 0;

	UE_LOG(LogA1Tutorial, Log, TEXT("[Tutorial] Starting Step: %s with %d actions"),
		*StepInfo.StepName.ToString(), StepInfo.Actions.Num());

	BindToGameplayEvent();

	BP_OnStepStarted();

	// 첫 번째 액션 시작
	if ( StepInfo.Actions.Num() > 0 )
	{
		StartCurrentAction();
	}
	else
	{
		// 액션이 없으면 즉시 완료
		CompleteStep();
	}
}

void UA1TutorialStep::StartCurrentAction()
{
	if ( CurrentActionIndex >= StepInfo.Actions.Num() )
	{
		// 모든 액션 완료
		CompleteStep();
		return;
	}

	const FA1TutorialAction& CurrentAction = StepInfo.Actions[ CurrentActionIndex ];

	// 조건 확인
	if ( !CheckActionCondition(CurrentAction) )
	{
		UE_LOG(LogA1Tutorial, Warning, TEXT("[Tutorial] Action condition failed, skipping action: %s"),
			*CurrentAction.ActionName.ToString());
		AdvanceToNextAction();
		return;
	}

	bCurrentActionCompleted = false;
	bWaitingForDialogueInput = false;
	bWaitingForEvent = false;

	UE_LOG(LogA1Tutorial, Log, TEXT("[Tutorial] Starting Action %d/%d: %s"),
		CurrentActionIndex + 1, StepInfo.Actions.Num(), *CurrentAction.ActionName.ToString());

	BP_OnActionStarted(CurrentAction, CurrentActionIndex);

	// 액션 타입에 따라 실행
	switch ( CurrentAction.ActionType )
	{
	case ETutorialActionType::Fade:
		ExecuteFadeAction(CurrentAction);
		break;
	case ETutorialActionType::HideHUD:
		ExecuteHideHUD(CurrentAction);
	case ETutorialActionType::Dialogue:
	case ETutorialActionType::SystemMessage:
		ExecuteDialogueAction(CurrentAction);
		break;
	case ETutorialActionType::Mission:
		ExecuteMissionAction(CurrentAction);
		break;
	case ETutorialActionType::WaitForEvent:
	case ETutorialActionType::WaitForInteraction:
		ExecuteWaitAction(CurrentAction);
		break;
	case ETutorialActionType::DisableMovement:
	case ETutorialActionType::EnableMovement:
		ExecuteMovementAction(CurrentAction);
		break;
	case ETutorialActionType::TriggerEvent:
		ExecuteTriggerEventAction(CurrentAction);
		break;
	case ETutorialActionType::Cutscene:
		ExecuteCutsceneAction(CurrentAction);
		break;
	case ETutorialActionType::Delay:
		ExecuteDelayAction(CurrentAction);
		break;
	case ETutorialActionType::LyingBed:
		ExecuteLyingBedAction();
		break;
	case ETutorialActionType::SpawnItem:
		ExecuteSpawnItem(CurrentAction);
		break;
	case ETutorialActionType::Navigate:
		ExecuteNavigateMinimap(CurrentAction);
		break;
	case ETutorialActionType::Highlight:
		ExecuteHighlightActor(CurrentAction);
		break;
	case ETutorialActionType::ActivatePipeRepairBase:
		ExecuteActivatePipeRepair();
		break;
	case ETutorialActionType::SpawnChecker:
		ExecuteSpawnChecker(CurrentAction);
		break;
	default:
		// 알 수 없는 액션 타입, 즉시 완료
		CompleteCurrentAction();
		break;
	}
}

void UA1TutorialStep::CompleteCurrentAction()
{
	if ( bCurrentActionCompleted )
		return;

	bCurrentActionCompleted = true;

	const FA1TutorialAction& CompletedAction = StepInfo.Actions[ CurrentActionIndex ];

	UE_LOG(LogA1Tutorial, Log, TEXT("[Tutorial] Completed Action %d/%d: %s"),
		CurrentActionIndex + 1, StepInfo.Actions.Num(), *CompletedAction.ActionName.ToString());

	BP_OnActionCompleted(CompletedAction, CurrentActionIndex);

	// Auto Progress 딜레이가 있으면 적용
	if ( CompletedAction.bAutoProgress && CompletedAction.AutoProgressDelay > 0.0f )
	{
		GetWorld()->GetTimerManager().SetTimer(AutoProgressTimer,
			[ this ] ()
			{
				AdvanceToNextAction();
			},
			CompletedAction.AutoProgressDelay, false);
	}
	else
	{
		AdvanceToNextAction();
	}
}

void UA1TutorialStep::AdvanceToNextAction()
{
	CurrentActionIndex++;
	StartCurrentAction();
}

void UA1TutorialStep::ExecuteFadeAction(const FA1TutorialAction& Action)
{
	TriggerFadeEffect(Action.FadeStart, Action.FadeEnd, Action.FadeDuration, Action.FadeColor);

	//Fade 업무를 하는 동안 다른걸 할 수 있게 조정
	CompleteCurrentAction();
}

void UA1TutorialStep::ExecuteDialogueAction(const FA1TutorialAction& Action)
{
	FA1TutorialMessage Message;
	Message.MessageType = Action.ActionType;
	Message.Content = Action.DialogueData.DialogueText;
	Message.SpeakerName = Action.DialogueData.SpeakerName;
	Message.bShowMissionUI = false;

	// 타이핑 효과 설정
	Message.bUseTypingEffect = Action.DialogueData.bUseTypingEffect;
	Message.TypingSpeed = Action.DialogueData.TypingSpeed;
	Message.TypingSound = Action.DialogueData.TypingSound;

	// AI 오류 상태 설정
	Message.bIsGlitched = Action.DialogueData.bIsGlitched;
	Message.GlitchSound = Action.DialogueData.GlitchSound;

	if ( TutorialManager )
	{
		TutorialManager->BroadcastTutorialMessage(Message);
	}

	BP_OnDialogueChanged(Action.DialogueData);

	// 사용자 입력이 필요한지 확인
	if ( Action.DialogueData.bRequireInput )
	{
		bWaitingForDialogueInput = true;
		// 사용자 입력 대기, AdvanceDialogue() 호출 시 완료
	}
	else
	{
		// 자동 진행
		GetWorld()->GetTimerManager().SetTimer(DialogueTimer,
			[ this ] ()
			{
				CompleteCurrentAction();
			},
			Action.DialogueData.DisplayDuration, false);
	}
}

void UA1TutorialStep::ExecuteMissionAction(const FA1TutorialAction& Action)
{
	FA1TutorialMission Message;
	Message.MissionTitle = Action.MissionData.MissionTitle;
	Message.MissionDescription= Action.MissionData.MissionDescription;

	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
	MessageSubsystem.BroadcastMessage(A1GameplayTags::Message_Tutorial_Mission, Message);
	

	// 미션은 즉시 완료 (실제 완료는 이벤트로 처리)
	CompleteCurrentAction();
}

void UA1TutorialStep::ExecuteWaitAction(const FA1TutorialAction& Action)
{
	bWaitingForEvent = true;

	// Event Listener
	if ( Action.WaitEventTag.IsValid() )
	{
		UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
		ActionEventListenerHandle = MessageSubsystem.RegisterListener<FGameplayEventData>(
			Action.WaitEventTag, this, &UA1TutorialStep::OnGameplayEvent);
	}

	UE_LOG(LogA1Tutorial, Log, TEXT("[Tutorial] Waiting for event: %s"), *Action.WaitEventTag.ToString());
}

void UA1TutorialStep::ExecuteMovementAction(const FA1TutorialAction& Action)
{
	bool bEnable = ( Action.ActionType == ETutorialActionType::EnableMovement );
	SetPlayerMovementEnabled(bEnable);

	// 즉시 완료
	CompleteCurrentAction();
}

void UA1TutorialStep::ExecuteTriggerEventAction(const FA1TutorialAction& Action)
{
	if ( Action.TriggerEventTag.IsValid() )
	{
		FGameplayEventData EventData;
		UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
		MessageSubsystem.BroadcastMessage(Action.TriggerEventTag, EventData);

		UE_LOG(LogA1Tutorial, Log, TEXT("[Tutorial] Triggered event: %s"), *Action.TriggerEventTag.ToString());
	}

	// 즉시 완료
	CompleteCurrentAction();
}

void UA1TutorialStep::ExecuteCutsceneAction(const FA1TutorialAction& Action)
{
	UE_LOG(LogA1Tutorial, Warning, TEXT("[Tutorial] ExecuteCutsceneAction called for: %s"),
		*Action.ActionName.ToString());

	if ( Action.CutsceneSequence.IsValid() )
	{
		ULevelSequence* Sequence = Action.CutsceneSequence.LoadSynchronous();
		if ( Sequence )
		{
			PlayCutscene(Sequence);
			// 컷신 완료는 Blueprint에서 처리하도록 함
			return;
		}
	}

	// 컷신이 없거나 로드 실패 시 즉시 완료
	CompleteCurrentAction();
}

void UA1TutorialStep::ExecuteDelayAction(const FA1TutorialAction& Action)
{
	GetWorld()->GetTimerManager().SetTimer(DelayTimer,
		[ this ] ()
		{
			CompleteCurrentAction();
		},
		Action.DelayDuration, false);
}

void UA1TutorialStep::ExecuteLyingBedAction()
{
	//Actor가 LyraCharacter인지 체크
	ALyraCharacter* LyraCharacter = Cast<ALyraCharacter>(GetPlayerCharacter());
	if ( !LyraCharacter )
		return;

	ALyraPlayerController* LyraPlayerController = LyraCharacter->GetLyraPlayerController();
	if ( !LyraPlayerController )
		return;

	// AbilitySystemComponent 가져오기
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(LyraCharacter);
	if ( !ASC )
		return;

	AActor* Actor = UGameplayStatics::GetActorOfClass(GetWorld(), AA1BedBase::StaticClass());
	if (AA1BedBase* Bed = Cast<AA1BedBase>(Actor) )
	{
		FA1InteractionQuery InteractionQuery;
		InteractionQuery.RequestingAvatar = LyraCharacter;
		InteractionQuery.RequestingController = LyraPlayerController;

		if ( Bed->CanInteraction(InteractionQuery) )
		{
			FGameplayEventData Payload;
			Payload.EventTag = A1GameplayTags::Ability_Interact_Active;
			Payload.Instigator = LyraCharacter;
			Payload.Target = Bed;

			ASC->HandleGameplayEvent(A1GameplayTags::Ability_Interact_Active, &Payload);
		}
	}

	CompleteCurrentAction();
}

void UA1TutorialStep::ExecuteHideHUD(const FA1TutorialAction& Action)
{
	// CMD 오픈에 따른 UI 비활성화
	FA1WidgetActiveMessage Message;
	Message.bActive = Action.bHideHUD;

	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(this);
	MessageSubsystem.BroadcastMessage(A1GameplayTags::Message_HUD_Active, Message);

	CompleteCurrentAction();
}

void UA1TutorialStep::ExecuteSpawnItem(const FA1TutorialAction& Action)
{
	int32 ItemTemplateId = UA1ItemData::Get().FindItemTemplateIDByClass(Action.SpawnItemTemplate);

	UA1ItemInstance* ItemInstance = NewObject<UA1ItemInstance>();
	ItemInstance->Init(ItemTemplateId, EItemRarity::Poor);

	const UA1ItemFragment_Equipable_Attachment* AttachmentFragment = ItemInstance->FindFragmentByClass<UA1ItemFragment_Equipable_Attachment>();
	if ( !AttachmentFragment )
		return;

	const FA1ItemAttachInfo& AttachInfo = AttachmentFragment->ItemAttachInfo;
	if ( AttachInfo.SpawnItemClass )
	{
		AA1EquipmentBase* NewItem = GetWorld()->SpawnActorDeferred<AA1EquipmentBase>(AttachInfo.SpawnItemClass, FTransform::Identity);
		NewItem->Init(ItemInstance->GetItemTemplateID(), EEquipmentSlotType::Count, ItemInstance->GetItemRarity());
		NewItem->SetActorRelativeTransform(Action.SpawnTransform);
		NewItem->SetActorScale3D(FVector::OneVector);
		NewItem->SetPickup(false);
		NewItem->SetActorHiddenInGame(false);
		NewItem->FinishSpawning(FTransform::Identity, true);

		CompleteCurrentAction();
	}
}

void UA1TutorialStep::ExecuteNavigateMinimap(const FA1TutorialAction& Action)
{
	FA1TutorialMessage Message;
	Message.NavigateActorClass = Action.NavigateActorClass;

	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
	MessageSubsystem.BroadcastMessage(Action.NavigateTag, Message);

	CompleteCurrentAction();
}

void UA1TutorialStep::ExecuteHighlightActor(const FA1TutorialAction& Action)
{
	AActor* FindActor = UGameplayStatics::GetActorOfClass(GetWorld(), Action.HighlightActorClass);

	TArray<UPrimitiveComponent*> PrimitiveComponents;
	FindActor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);

	for ( UPrimitiveComponent* Component : PrimitiveComponents )
	{
		Component->SetRenderCustomDepth(true);
		Component->SetCustomDepthStencilValue(250);
	}

	CompleteCurrentAction();
}

void UA1TutorialStep::ExecuteActivatePipeRepair()
{
	AActor* FindActor = UGameplayStatics::GetActorOfClass(GetWorld(), AA1SpaceshipBase::StaticClass());
	if (AA1SpaceshipBase* Spaceship = Cast<AA1SpaceshipBase>(FindActor) )
	{
		Spaceship->BreakPipeRepairBase();
	}
	CompleteCurrentAction();
}

void UA1TutorialStep::ExecuteSpawnChecker(const FA1TutorialAction& Action)
{
	GetWorld()->SpawnActor(Action.CheckerClass, &Action.CheckerSpawnTransform);

	CompleteCurrentAction();
}

void UA1TutorialStep::AdvanceDialogue()
{
	if ( bWaitingForDialogueInput )
	{
		bWaitingForDialogueInput = false;
		CompleteCurrentAction();
	}
}

const FA1TutorialAction& UA1TutorialStep::GetCurrentAction() const
{
	static FA1TutorialAction DefaultAction;
	if ( CurrentActionIndex >= 0 && CurrentActionIndex < StepInfo.Actions.Num() )
	{
		return StepInfo.Actions[ CurrentActionIndex ];
	}
	return DefaultAction;
}

bool UA1TutorialStep::CheckActionCondition(const FA1TutorialAction& Action)
{
	if ( !Action.ConditionTag.IsValid() )
		return true;

	// TODO: 조건 태그에 따른 실제 조건 확인 로직 구현
	// 예: 특정 아이템 보유, 특정 위치 도달 등
	return true;
}

void UA1TutorialStep::CompleteStep()
{
	if ( !bIsActive || bIsCompleted )
		return;

	bIsCompleted = true;
	bIsActive = false;

	UE_LOG(LogA1Tutorial, Log, TEXT("[Tutorial] Completing Step: %s"), *StepInfo.StepName.ToString());

	UnbindFromGameplayEvent();

	// 모든 타이머 정리
	if ( UWorld* World = GetWorld() )
	{
		World->GetTimerManager().ClearTimer(AutoProgressTimer);
		World->GetTimerManager().ClearTimer(DelayTimer);
		World->GetTimerManager().ClearTimer(DialogueTimer);
	}

	// 플레이어 이동 복원
	if ( bPlayerMovementWasDisabled )
	{
		SetPlayerMovementEnabled(true);
	}

	// Navigation 정리
	if ( TutorialManager )
	{
		TutorialManager->ClearNavigationTarget();
	}

	BP_OnStepCompleted();

	// 다음 단계로 진행
	if ( TutorialManager && !StepInfo.NextStepID.IsEmpty() )
	{
		TutorialManager->StartStep(StepInfo.NextStepID);
	}
	else if ( TutorialManager )
	{
		// 마지막 단계 완료
		TutorialManager->OnTutorialCompleted.Broadcast();
	}
}

void UA1TutorialStep::CleanupStep()
{
	UnbindFromGameplayEvent();

	if ( UWorld* World = GetWorld() )
	{
		World->GetTimerManager().ClearTimer(AutoProgressTimer);
		World->GetTimerManager().ClearTimer(DelayTimer);
		World->GetTimerManager().ClearTimer(DialogueTimer);
	}

	if ( bPlayerMovementWasDisabled )
	{
		SetPlayerMovementEnabled(true);
	}

	bIsActive = false;
}

void UA1TutorialStep::OnGameplayEvent(FGameplayTag EventTag, const FGameplayEventData& Payload)
{
	if ( !bIsActive || bIsCompleted )
		return;

	UE_LOG(LogA1Tutorial, Log, TEXT("[Tutorial] Received Event: %s for Step: %s"),
		*EventTag.ToString(), *StepInfo.StepName.ToString());

	// 현재 대기 중인 액션의 이벤트인지 확인
	if ( bWaitingForEvent && CurrentActionIndex < StepInfo.Actions.Num() )
	{
		const FA1TutorialAction& CurrentAction = StepInfo.Actions[ CurrentActionIndex ];
		if ( CurrentAction.WaitEventTag.IsValid() && EventTag.MatchesTag(CurrentAction.WaitEventTag) )
		{
			bWaitingForEvent = false;

			// 액션 이벤트 리스너 해제
			if ( ActionEventListenerHandle.IsValid() )
			{
				UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
				MessageSubsystem.UnregisterListener(ActionEventListenerHandle);
				ActionEventListenerHandle.Unregister();
			}

			CompleteCurrentAction();
		}
	}

	// 단계 완료 조건 확인
	if ( StepInfo.CompletionTag.IsValid() && EventTag.MatchesTag(StepInfo.CompletionTag) )
	{
		CompleteStep();
	}
}

void UA1TutorialStep::BindToGameplayEvent()
{
	if ( !StepInfo.CompletionTag.IsValid() )
		return;

	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
	MessageListenerHandle = MessageSubsystem.RegisterListener<FGameplayEventData>(StepInfo.CompletionTag, this,
		&UA1TutorialStep::OnGameplayEvent);
}

void UA1TutorialStep::UnbindFromGameplayEvent()
{
	if ( MessageListenerHandle.IsValid() )
	{
		UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
		MessageSubsystem.UnregisterListener(MessageListenerHandle);
		MessageListenerHandle.Unregister();
	}

	if ( ActionEventListenerHandle.IsValid() )
	{
		UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
		MessageSubsystem.UnregisterListener(ActionEventListenerHandle);
		ActionEventListenerHandle.Unregister();
	}
}

void UA1TutorialStep::OnCutsceneFinished()
{
	UE_LOG(LogA1Tutorial, Warning, TEXT("=== OnCutsceneFinished CALLED ==="));

	// 중복 호출 방지
	static bool bIsProcessing = false;
	if ( bIsProcessing )
	{
		UE_LOG(LogA1Tutorial, Warning, TEXT("OnCutsceneFinished already processing, ignoring!"));
		return;
	}
	bIsProcessing = true;

	if ( CurrentSequencePlayer )
	{
		UE_LOG(LogA1Tutorial, Warning, TEXT("Cleaning up sequence player"));
		CurrentSequencePlayer->OnFinished.RemoveAll(this);
		CurrentSequencePlayer = nullptr;
	}

	UE_LOG(LogA1Tutorial, Warning, TEXT("Calling CompleteCurrentAction"));
	CompleteCurrentAction();

	bIsProcessing = false;
	UE_LOG(LogA1Tutorial, Warning, TEXT("=== OnCutsceneFinished END ==="));
	RestoreCameraState();

	RotateCharacter();
}

void UA1TutorialStep::RestoreCameraState()
{
	if ( APlayerController* PC = GetWorld()->GetFirstPlayerController() )
	{
		// 원래 뷰 타겟으로 복원
		if ( OriginalViewTarget.IsValid() )
		{
			PC->SetViewTarget(OriginalViewTarget.Get());
		}
		else if ( ALyraCharacter* Player = GetPlayerCharacter() )
		{
			// 원래 뷰 타겟이 없으면 플레이어로 설정
			PC->SetViewTarget(Player);
			Player->bUseControllerRotationPitch = false;
		}

		

		// 카메라 매니저 설정 복원
		if ( PC->PlayerCameraManager )
		{
			PC->PlayerCameraManager->CameraStyle = OriginalCameraMode;

			// 카메라 블렌드로 부드럽게 전환
			FViewTargetTransitionParams TransitionParams;
			TransitionParams.BlendTime = 1.0f; // 1초 블렌드
			TransitionParams.BlendFunction = VTBlend_Cubic;
			PC->SetViewTarget(PC->GetViewTarget(), TransitionParams);
		}

		// 입력 다시 활성화
		PC->EnableInput(PC);
	}
}

void UA1TutorialStep::RotateCharacter()
{
	if (ALyraCharacter* LyraCharacter = GetPlayerCharacter())
	{
		if ( ULyraCameraComponent* CameraComp = LyraCharacter->FindComponentByClass<ULyraCameraComponent>())
		{
			if (ULyraCameraModeStack* CameraModeStack = CameraComp->GetLyraCameraModeStack())
			{
				for ( ULyraCameraMode* Mode : CameraModeStack->GetCameraModeStack() )
				{
					if (ULyraCameraMode_FirstPerson* FirstPersonMode = Cast<ULyraCameraMode_FirstPerson>(Mode) )
					{
						FirstPersonMode->SetCanRotate(false);
						break;
					}
				}
			}
		}
	}
}

void UA1TutorialStep::TriggerFadeEffect(float FromAlpha, float ToAlpha, float Duration, FLinearColor Color)
{
	if ( UWorld* World = GetWorld() )
	{
		if ( AGameModeBase* GameMode = World->GetAuthGameMode() )
		{
			if ( ALyraGameMode* LyraGameMode = Cast<ALyraGameMode>(GameMode) )
			{
				LyraGameMode->TriggerFadeOnAllPlayer(FromAlpha, ToAlpha, Duration, Color);
			}
		}
	}
}

void UA1TutorialStep::SetPlayerMovementEnabled(bool bEnabled)
{
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if ( bEnabled )
		{
			PC->SetIgnoreMoveInput(false);
			PC->SetIgnoreLookInput(false);
			bPlayerMovementWasDisabled = false;
		}
		else
		{
			PC->SetIgnoreMoveInput(true);
			PC->SetIgnoreLookInput(true);
			bPlayerMovementWasDisabled = true;
		}
	}
}

ALyraCharacter* UA1TutorialStep::GetPlayerCharacter()
{
	//TODO eric1306 멀티에서 로컬 플레이어로 찾든가 해서 전환.
	AActor* FindActor = UGameplayStatics::GetActorOfClass(GetWorld(), ALyraCharacter::StaticClass());
	if (ALyraCharacter* LyraCharacter = Cast<ALyraCharacter>(FindActor) )
	{
		return LyraCharacter;
	}

	return nullptr;
}

void UA1TutorialStep::PlayCutscene(ULevelSequence* Sequence)
{
	if ( !Sequence )
	{
		UE_LOG(LogA1Tutorial, Warning, TEXT("Sequence is null, completing action"));
		CompleteCurrentAction();
		return;
	}

	if ( CurrentSequencePlayer )
	{
		// Force stop last sequence player
		CurrentSequencePlayer->Stop();
		CurrentSequencePlayer->OnFinished.RemoveAll(this);
		CurrentSequencePlayer = nullptr;
	}

	ALevelSequenceActor* FoundSequenceActor = nullptr;
	for ( TActorIterator<ALevelSequenceActor> ActorItr(GetWorld()); ActorItr; ++ActorItr )
	{
		ALevelSequenceActor* SequenceActor = *ActorItr;
		if ( SequenceActor && SequenceActor->GetSequence() == Sequence )
		{
			FoundSequenceActor = SequenceActor;
			break;
		}
	}

	if ( !FoundSequenceActor )
	{
		UE_LOG(LogA1Tutorial, Warning, TEXT("No sequence actor found in level!"));
		CompleteCurrentAction();
		return;
	}

	// 시퀀스 플레이어 가져오기
	CurrentSequencePlayer = FoundSequenceActor->GetSequencePlayer();

	if ( !CurrentSequencePlayer )
	{
		UE_LOG(LogA1Tutorial, Warning, TEXT("Failed to get sequence player from actor"));
		CompleteCurrentAction();
		return;
	}

	CurrentSequencePlayer->OnFinished.RemoveAll(this);
	CurrentSequencePlayer->OnFinished.Clear(); //clear all bindings

	CurrentSequencePlayer->OnFinished.AddDynamic(this, &UA1TutorialStep::OnCutsceneFinished);

	CurrentSequencePlayer->Play();
}