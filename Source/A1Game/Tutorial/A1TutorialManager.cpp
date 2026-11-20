// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#include "Tutorial/A1TutorialManager.h"

#include "A1GameplayTags.h"
#include "A1LogChannels.h"
#include "Data/A1TutorialData.h"
#include "Tutorial/A1TutorialStep.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1TutorialManager)

UA1TutorialManager::UA1TutorialManager()
{
	bIsTutorialActive = false;
	bIsTutorialPaused = false;
	bHasActiveNavigation = false;
	CurrentNavigationTarget = FVector::ZeroVector;
}

void UA1TutorialManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	LoadTutorialSteps();

	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetGameInstance());
	MessageListenerHandle = MessageSubsystem.RegisterListener(FGameplayTag(), this,
			&UA1TutorialManager::OnGameplayEventReceived);
	

	UE_LOG(LogA1Tutorial, Log, TEXT("[TutorialManager] Initialized with %d steps"), StepDataMap.Num());
}

void UA1TutorialManager::Deinitialize()
{
	StopTutorial();

	if ( MessageListenerHandle.IsValid() )
	{
		UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetGameInstance());
		MessageSubsystem.UnregisterListener(MessageListenerHandle);
		MessageListenerHandle.Unregister();
	}

	Super::Deinitialize();
}

void UA1TutorialManager::StartTutorial()
{
	if ( bIsTutorialActive )
		return;

	UE_LOG(LogA1Tutorial, Log, TEXT("[TutorialManager] Starting Tutorial"));

	bIsTutorialActive = true;
	bIsTutorialPaused = false;
	CompletedSteps.Empty();

	OnTutorialStarted.Broadcast();
	BP_OnTutorialStarted();

	// 첫 번째 단계 시작
	if ( !FirstStepID.IsEmpty() && StepDataMap.Contains(FirstStepID) )
	{
		StartStep(FirstStepID);
	}
	else if ( !StepDataMap.IsEmpty() )
	{
		// FirstStepID가 없으면 첫 번째 단계 사용
		auto It = StepDataMap.CreateConstIterator();
		StartStep(It.Key());
	}
}

void UA1TutorialManager::StopTutorial()
{
	if ( !bIsTutorialActive )
		return;

	UE_LOG(LogA1Tutorial, Log, TEXT("[TutorialManager] Stopping Tutorial"));

	if ( CurrentStep )
	{
		CurrentStep->CleanupStep();
		CurrentStep = nullptr;
	}

	bIsTutorialActive = false;
	bIsTutorialPaused = false;
	CurrentStepID.Empty();
	ClearNavigationTarget();
}

void UA1TutorialManager::PauseTutorial()
{
	if ( !bIsTutorialActive || bIsTutorialPaused )
		return;

	UE_LOG(LogA1Tutorial, Log, TEXT("[TutorialManager] Pausing Tutorial"));

	bIsTutorialPaused = true;
	BP_OnTutorialPaused();
}

void UA1TutorialManager::ResumeTutorial()
{
	if ( !bIsTutorialActive || !bIsTutorialPaused )
		return;

	UE_LOG(LogA1Tutorial, Log, TEXT("[TutorialManager] Resuming Tutorial"));

	bIsTutorialPaused = false;
	BP_OnTutorialResumed();
}

void UA1TutorialManager::StartStep(const FString& StepID)
{
	if ( !bIsTutorialActive || bIsTutorialPaused || StepID.IsEmpty() )
		return;

	FA1TutorialStepInfo* StepInfo = StepDataMap.Find(StepID);
	if ( !StepInfo)
	{
		UE_LOG(LogA1Tutorial, Warning, TEXT("[TutorialManager] Tutorial Step not found: %s"), *StepID);
		return;
	}

	UE_LOG(LogA1Tutorial, Log, TEXT("[TutorialManager] Starting Step: %s"), *StepID);

	// 현재 단계 정리
	if ( CurrentStep )
	{
		CurrentStep->CleanupStep();
	}

	// 새 단계 생성 및 시작
	CurrentStep = CreateStepInstance(StepInfo);
	if ( CurrentStep )
	{
		CurrentStepID = StepID;
		CurrentStep->StartStep();
		OnStepChanged.Broadcast(*StepInfo);
	}
}

void UA1TutorialManager::CompleteCurrentStep()
{
	if ( CurrentStep && !CurrentStep->IsStepCompleted() )
	{
		// 완료된 단계 기록
		if ( !CurrentStepID.IsEmpty() )
		{
			CompletedSteps.AddUnique(CurrentStepID);
		}

		CurrentStep->CompleteStep();
	}
}

void UA1TutorialManager::SkipToStep(const FString& StepID)
{
	if ( !bIsTutorialActive )
		return;

	UE_LOG(LogA1Tutorial, Log, TEXT("[TutorialManager] Skipping to Step: %s"), *StepID);
	StartStep(StepID);
}

void UA1TutorialManager::SendGameplayEvent(FGameplayTag EventTag, const FGameplayEventData& EventData)
{
	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetGameInstance());
	
	MessageSubsystem.BroadcastMessage(EventTag, EventData);
	UE_LOG(LogA1Tutorial, Log, TEXT("[TutorialManager] Sent Gameplay Event: %s"), *EventTag.ToString());
}

FString UA1TutorialManager::GetCurrentStepID() const
{
	return CurrentStepID;
}

int32 UA1TutorialManager::GetCurrentStepIndex() const
{
	if ( CurrentStepID.IsEmpty() )
		return -1;

	return StepOrder.Find(CurrentStepID);
}

float UA1TutorialManager::GetTutorialProgress() const
{
	if ( StepOrder.Num() == 0 )
		return 0.0f;

	int32 CurrentIndex = GetCurrentStepIndex();
	if ( CurrentIndex < 0 )
		return 0.0f;

	return static_cast< float >(CurrentIndex) / static_cast< float >(StepOrder.Num());
}

void UA1TutorialManager::AdvanceDialogue()
{
	if ( CurrentStep )
	{
		CurrentStep->AdvanceDialogue();
	}
}

void UA1TutorialManager::SetNavigationTarget(const FVector& TargetLocation, const FText& TargetName,
	UTexture2D* TargetIcon)
{
	bHasActiveNavigation = true;
	CurrentNavigationTarget = TargetLocation;
	CurrentNavigationText = TargetName;
	CurrentNavigationIcon = TargetIcon;

	UE_LOG(LogA1Tutorial, Log, TEXT("[TutorialManager] Navigation target set: %s at %s"),
		*TargetName.ToString(), *TargetLocation.ToString());
}

void UA1TutorialManager::ClearNavigationTarget()
{
	bHasActiveNavigation = false;
	CurrentNavigationTarget = FVector::ZeroVector;
	CurrentNavigationText = FText::GetEmpty();
	CurrentNavigationIcon = nullptr;

	UE_LOG(LogA1Tutorial, Log, TEXT("[TutorialManager] Navigation target cleared"));
}

void UA1TutorialManager::BroadcastTutorialMessage(const FA1TutorialMessage& Message)
{
	//TODO eric1306 -> bool 값에 따라 다른 messsage 보내게 구현
	UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
	MessageSubsystem.BroadcastMessage(A1GameplayTags::Message_Tutorial_Dialogue, Message);
}

void UA1TutorialManager::TransitionToStep(const FString& StepID)
{
	StartStep(StepID);
}

void UA1TutorialManager::OnGameplayEventReceived(FGameplayTag EventTag, const FGameplayEventData& EventData)
{
	if ( !bIsTutorialActive || bIsTutorialPaused || !CurrentStep )
		return;

	CurrentStep->OnGameplayEvent(EventTag, EventData);
}

void UA1TutorialManager::LoadTutorialSteps()
{
	StepDataMap.Empty();
	StepOrder.Empty();

	//Tutorial Data를 로드
	TArray<FA1TutorialStepInfo> TutorialStepInfos = UA1TutorialData::Get().TutorialStepInfos;

	for (FA1TutorialStepInfo TutorialStepInfo : TutorialStepInfos)
	{
		StepDataMap.Add(TutorialStepInfo.StepID, TutorialStepInfo);
		StepOrder.Add(TutorialStepInfo.StepID);

		UE_LOG(LogA1Tutorial, Log, TEXT("[TutorialManager] Loaded Step: %s - %s"),
			*TutorialStepInfo.StepID, *TutorialStepInfo.StepName.ToString());
	}

	UE_LOG(LogA1Tutorial, Log, TEXT("[TutorialManager] Loaded %d tutorial steps"), StepDataMap.Num());
}

UA1TutorialStep* UA1TutorialManager::CreateStepInstance(const FA1TutorialStepInfo* StepData)
{
	if ( !StepData )
		return nullptr;

	UA1TutorialStep* NewStep = NewObject<UA1TutorialStep>(this);
	NewStep->Initialize(this, *StepData);
	return NewStep;
}