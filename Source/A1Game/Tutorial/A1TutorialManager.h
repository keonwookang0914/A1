// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Engine/Texture2D.h"
#include "Sound/SoundBase.h"
#include "A1TutorialStep.h"
#include "Data/A1TutorialData.h"
#include "A1TutorialManager.generated.h"

/**
 * A1TutorialManager(GameInstanceSubsystem)
 * This Class file Include
 * - FA1TutorialMessage
 * - UA1TutorialManager
 */

USTRUCT(BlueprintType)
struct A1GAME_API FA1TutorialMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	ETutorialActionType MessageType;

	UPROPERTY(BlueprintReadWrite)
	FText Content;

	UPROPERTY(BlueprintReadWrite)
	FString SpeakerName;

	UPROPERTY(BlueprintReadWrite)
	bool bShowMissionUI = false;

	UPROPERTY(BlueprintReadWrite)
	FA1TutorialMission MissionData;

	// 타이핑 효과
	UPROPERTY(BlueprintReadWrite)
	bool bUseTypingEffect = true;

	UPROPERTY(BlueprintReadWrite)
	float TypingSpeed = 0.05f;

	UPROPERTY(BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> TypingSound;

	// AI 오류 상태
	UPROPERTY(BlueprintReadWrite)
	bool bIsGlitched = false;

	UPROPERTY(BlueprintReadWrite)
	TSoftObjectPtr<USoundBase> GlitchSound;

	// 네비게이션
	UPROPERTY(BlueprintReadWrite)
	bool bShowNavigation = false;

	UPROPERTY(BlueprintReadWrite)
	FVector NavigationTarget = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite)
	FText NavigationText;

	UPROPERTY(BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> NavigationIcon;

	// 페이드 효과
	UPROPERTY(BlueprintReadWrite)
	bool bTriggerFadeIn = false;

	UPROPERTY(BlueprintReadWrite)
	bool bTriggerFadeOut = false;

	UPROPERTY(BlueprintReadWrite)
	float FadeDuration = 1.0f;

	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<AActor> NavigateActorClass;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTutorialStepChanged, const FA1TutorialStepInfo, StepData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTutorialMessage, const FA1TutorialMessage&, Message);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTutorialCompleted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTutorialStarted);

UCLASS()
class A1GAME_API UA1TutorialManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	UA1TutorialManager();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	void OnGameplayEventReceived(FGameplayTag EventTag, const FGameplayEventData& EventData);

	// 튜토리얼 제어
	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void StartTutorial();

	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void StopTutorial();

	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void PauseTutorial();

	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void ResumeTutorial();

	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void StartStep(const FString& StepID);

	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void CompleteCurrentStep();

	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void SkipToStep(const FString& StepID);

	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void SendGameplayEvent(FGameplayTag EventTag, const FGameplayEventData& EventData);

	// 상태 조회
	UFUNCTION(BlueprintPure, Category = "Tutorial")
	bool IsTutorialActive() const { return bIsTutorialActive; }

	UFUNCTION(BlueprintPure, Category = "Tutorial")
	bool IsTutorialPaused() const { return bIsTutorialPaused; }

	UFUNCTION(BlueprintPure, Category = "Tutorial")
	UA1TutorialStep* GetCurrentStep() const { return CurrentStep; }

	UFUNCTION(BlueprintPure, Category = "Tutorial")
	FString GetCurrentStepID() const;

	UFUNCTION(BlueprintPure, Category = "Tutorial")
	int32 GetCurrentStepIndex() const;

	UFUNCTION(BlueprintPure, Category = "Tutorial")
	int32 GetTotalStepCount() const { return StepDataMap.Num(); }

	UFUNCTION(BlueprintPure, Category = "Tutorial")
	float GetTutorialProgress() const;

	// UI 제어
	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void AdvanceDialogue();

	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void SetNavigationTarget(const FVector& TargetLocation, const FText& TargetName, UTexture2D* TargetIcon = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Tutorial")
	void ClearNavigationTarget();

	// Public methods for TutorialStep access
	void BroadcastTutorialMessage(const FA1TutorialMessage& Message);
	void TransitionToStep(const FString& StepID);

	// 델리게이트
	UPROPERTY(BlueprintAssignable)
	FOnTutorialStarted OnTutorialStarted;

	UPROPERTY(BlueprintAssignable)
	FOnTutorialStepChanged OnStepChanged;

	UPROPERTY(BlueprintAssignable)
	FOnTutorialMessage OnTutorialMessage;

	UPROPERTY(BlueprintAssignable)
	FOnTutorialCompleted OnTutorialCompleted;

protected:
	void OnGameplayEventReceived(FGameplayTag EventTag, const FGameplayEventData* EventData);

	void LoadTutorialSteps();
	UA1TutorialStep* CreateStepInstance(const FA1TutorialStepInfo* StepData);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnTutorialStarted();

	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnTutorialCompleted();

	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnTutorialPaused();

	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnTutorialResumed();

private:
	UPROPERTY(EditDefaultsOnly, Category = "Tutorial", meta = ( AllowPrivateAccess = "true" ))
	TArray<TSoftObjectPtr<UA1TutorialData>> TutorialStepAssets;

	UPROPERTY(EditDefaultsOnly, Category = "Tutorial", meta = ( AllowPrivateAccess = "true" ))
	FString FirstStepID = TEXT("Step_Opening");

	UPROPERTY()
	TMap<FString, FA1TutorialStepInfo> StepDataMap;

	UPROPERTY()
	TArray<FString> StepOrder;

	UPROPERTY()
	TObjectPtr<UA1TutorialStep> CurrentStep;

	UPROPERTY()
	bool bIsTutorialActive = false;

	UPROPERTY()
	bool bIsTutorialPaused = false;

	UPROPERTY()
	FString CurrentStepID;

	UPROPERTY()
	TArray<FString> CompletedSteps;

	UPROPERTY()
	bool bHasActiveNavigation = false;

	UPROPERTY()
	FVector CurrentNavigationTarget = FVector::ZeroVector;

	UPROPERTY()
	FText CurrentNavigationText;

	UPROPERTY()
	TSoftObjectPtr<UTexture2D> CurrentNavigationIcon;

	FGameplayMessageListenerHandle MessageListenerHandle;
};