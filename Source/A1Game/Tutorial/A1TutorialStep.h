// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Data/A1TutorialData.h"
#include "Engine/Texture2D.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Sound/SoundBase.h"
#include "A1TutorialStep.generated.h"

class UA1TutorialData;
class UA1TutorialManager;
class ALyraCharacter;
class ULevelSequence;

UCLASS(BlueprintType)
class A1GAME_API UA1TutorialStep : public UObject
{
	GENERATED_BODY()

public:
	UA1TutorialStep();

	UFUNCTION(BlueprintCallable)
	virtual void Initialize(UA1TutorialManager* InManager, const FA1TutorialStepInfo& InStepInfo);

	UFUNCTION(BlueprintCallable)
	virtual void StartStep();

	UFUNCTION(BlueprintCallable)
	virtual void CompleteStep();

	UFUNCTION(BlueprintCallable)
	virtual void CleanupStep();

	// 액션 관련 함수들
	UFUNCTION(BlueprintCallable)
	virtual void StartCurrentAction();

	UFUNCTION(BlueprintCallable)
	virtual void CompleteCurrentAction();

	UFUNCTION(BlueprintCallable)
	virtual void AdvanceToNextAction();

	// 특정 액션 타입 처리 함수들
	UFUNCTION(BlueprintCallable)
	virtual void ExecuteFadeAction(const FA1TutorialAction& Action);

	UFUNCTION(BlueprintCallable)
	virtual void ExecuteDialogueAction(const FA1TutorialAction& Action);

	UFUNCTION(BlueprintCallable)
	virtual void ExecuteMissionAction(const FA1TutorialAction& Action);

	UFUNCTION(BlueprintCallable)
	virtual void ExecuteWaitAction(const FA1TutorialAction& Action);

	UFUNCTION(BlueprintCallable)
	virtual void ExecuteMovementAction(const FA1TutorialAction& Action);

	UFUNCTION(BlueprintCallable)
	virtual void ExecuteTriggerEventAction(const FA1TutorialAction& Action);

	UFUNCTION(BlueprintCallable)
	virtual void ExecuteCutsceneAction(const FA1TutorialAction& Action);

	UFUNCTION(BlueprintCallable)
	virtual void ExecuteDelayAction(const FA1TutorialAction& Action);
	UFUNCTION(BlueprintCallable)
	virtual void ExecuteLyingBedAction();

	UFUNCTION(BlueprintCallable)
	virtual void ExecuteHideHUD(const FA1TutorialAction& Action);

	UFUNCTION(BlueprintCallable)
	virtual void ExecuteSpawnItem(const FA1TutorialAction& Action);

	UFUNCTION(BlueprintCallable)
	virtual void ExecuteNavigateMinimap(const FA1TutorialAction& Action);

	UFUNCTION(BlueprintCallable)
	virtual void ExecuteHighlightActor(const FA1TutorialAction& Action);

	UFUNCTION(BlueprintCallable)
	virtual void ExecuteActivatePipeRepair();

	UFUNCTION(BlueprintCallable)
	virtual void ExecuteSpawnChecker(const FA1TutorialAction& Action);

	// 대화 진행 (사용자 입력용)
	UFUNCTION(BlueprintCallable)
	virtual void AdvanceDialogue();

	// 상태 확인
	UFUNCTION(BlueprintPure)
	bool IsStepCompleted() const { return bIsCompleted; }

	UFUNCTION(BlueprintPure)
	bool IsStepActive() const { return bIsActive; }

	UFUNCTION(BlueprintPure)
	const FA1TutorialStepInfo& GetStepData() const { return StepInfo; }

	UFUNCTION(BlueprintPure)
	int32 GetCurrentActionIndex() const { return CurrentActionIndex; }

	UFUNCTION(BlueprintPure)
	int32 GetTotalActionCount() const { return StepInfo.Actions.Num(); }

	UFUNCTION(BlueprintPure)
	const FA1TutorialAction& GetCurrentAction() const;

	// Public method for TutorialManager access
	virtual void OnGameplayEvent(FGameplayTag EventTag, const FGameplayEventData& Payload);

protected:
	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnStepStarted();

	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnStepCompleted();

	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnActionStarted(const FA1TutorialAction& Action, int32 ActionIndex);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnActionCompleted(const FA1TutorialAction& Action, int32 ActionIndex);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnDialogueChanged(const FA1TutorialDialogue& Dialogue);

	// 유틸리티 함수들
	UFUNCTION(BlueprintCallable)
	void TriggerFadeEffect(float FromAlpha, float ToAlpha, float Duration, FLinearColor Color = FLinearColor::Black);

	UFUNCTION(BlueprintCallable)
	void SetPlayerMovementEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure)
	ALyraCharacter* GetPlayerCharacter();

	UFUNCTION(BlueprintCallable)
	void PlayCutscene(ULevelSequence* Sequence);

	// 조건 확인
	UFUNCTION(BlueprintPure)
	bool CheckActionCondition(const FA1TutorialAction& Action);

	void BindToGameplayEvent();
	void UnbindFromGameplayEvent();

	UFUNCTION()
	void OnCutsceneFinished();

	void RestoreCameraState();

private:
	void RotateCharacter();

protected:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UA1TutorialManager> TutorialManager;

	UPROPERTY(BlueprintReadOnly)
	FA1TutorialStepInfo StepInfo;

	UPROPERTY(BlueprintReadOnly)
	bool bIsCompleted = false;

	UPROPERTY(BlueprintReadOnly)
	bool bIsActive = false;

	// 현재 실행 중인 액션 인덱스
	UPROPERTY(BlueprintReadOnly)
	int32 CurrentActionIndex = 0;

	// 현재 액션의 상태
	UPROPERTY(BlueprintReadOnly)
	bool bCurrentActionCompleted = false;

	// 대화 액션에서 사용 (현재 대화 액션 내에서의 진행 상태)
	UPROPERTY(BlueprintReadOnly)
	bool bWaitingForDialogueInput = false;

	// 이벤트 대기 액션에서 사용
	UPROPERTY(BlueprintReadOnly)
	bool bWaitingForEvent = false;

	// 플레이어 이동 상태 저장
	UPROPERTY(BlueprintReadOnly)
	bool bPlayerMovementWasDisabled = false;

	UPROPERTY()
	class ULevelSequencePlayer* CurrentSequencePlayer;
	// 카메라 상태 복원을 위한 변수들
	UPROPERTY()
	TWeakObjectPtr<AActor> OriginalViewTarget;

	UPROPERTY()
	FName OriginalCameraMode;

	// 타이머들
	FTimerHandle AutoProgressTimer;
	FTimerHandle DelayTimer;
	FTimerHandle DialogueTimer;

	// 이벤트 리스너
	FGameplayMessageListenerHandle MessageListenerHandle;
	FGameplayMessageListenerHandle ActionEventListenerHandle;
};