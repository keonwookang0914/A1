// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "A1TutorialData.generated.h"

class UA1ItemTemplate;
class AA1EquipmentBase;

UENUM(BlueprintType)
enum class ETutorialActionType : uint8
{
	None,
	Fade UMETA(DisplayName = "Fade"),
	HideHUD UMETA(DisplayName = "HideHUD"),
	Dialogue UMETA(DisplayName = "Dialogue"),
	SystemMessage UMETA(DisplayName = "SystemMessage"),
	Mission UMETA(DisplayName = "Mission"),
	WaitForEvent UMETA(DisplayName = "WaitForEvent"),
	WaitForInteraction UMETA(DisplayName = "WaitForInteraction"),
	Cutscene UMETA(DisplayName = "Cutscene"),
	DisableMovement UMETA(DisplayName = "DisableMovement"),
	EnableMovement UMETA(DisplayName = "EnableMovement"),
	TriggerEvent UMETA(DisplayName = "TriggerEvent"),
	Delay UMETA(DisplayName = "Delay"),
	LyingBed UMETA(DisplayName = "LyingBed"),
	SpawnItem UMETA(DisplayName = "SpawnItem"),
	Navigate UMETA(DisplayName = "Navigate"),
	Highlight UMETA(DisplayName = "Highlight"),
	ActivatePipeRepairBase UMETA(DisplayName = "ActivatePipeRepairBase"),
	SpawnChecker UMETA(DisplayName = "SpawnChecker")
};

USTRUCT(BlueprintType)
struct A1GAME_API FA1TutorialDialogue
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speaker")
	FString SpeakerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content", meta = (MultiLine))
	FText DialogueText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	float DisplayDuration = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
	bool bRequireInput = false;

	// Typing Effect
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typing Effect")
	bool bUseTypingEffect = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typing Effect", meta = ( EditCondition = "bUseTypingEffect", EditConditionHides ))
	float TypingSpeed = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typing Effect", meta = ( EditCondition = "bUseTypingEffect", EditConditionHides ))
	TSoftObjectPtr<USoundBase> TypingSound;

	// AI Error State
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Error")
	bool bIsGlitched = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Error", meta = ( EditCondition = "bIsGlitched", EditConditionHides ))
	TSoftObjectPtr<USoundBase> GlitchSound;
};

USTRUCT(BlueprintType)
struct A1GAME_API FA1TutorialMission
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Info")
	FText MissionTitle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Info", meta = (MultiLine))
	FText MissionDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Info")
	FGameplayTag CompletionEventTag;
};

USTRUCT(BlueprintType)
struct A1GAME_API FA1TutorialAction
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action", meta = ( DisplayPriority = "1" ))
	ETutorialActionType ActionType = ETutorialActionType::None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action", meta = ( DisplayPriority = "2" ))
	FText ActionName;

	//Fade
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fade Settings",
		meta = ( EditCondition = "ActionType == ETutorialActionType::Fade", EditConditionHides ))
	float FadeStart = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fade Settings",
		meta = ( EditCondition = "ActionType == ETutorialActionType::Fade", EditConditionHides ))
	float FadeEnd = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fade Settings",
		meta = ( EditCondition = "ActionType == ETutorialActionType::Fade", EditConditionHides))
	float FadeDuration = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Fade Settings",
		meta = ( EditCondition = "ActionType == ETutorialActionType::Fade", EditConditionHides ))
	FLinearColor FadeColor = FLinearColor::Black;

	//Hide HUD
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HUD Settings",
		meta = ( EditCondition = "ActionType == ETutorialActionType::HideHUD", EditConditionHides ))
	bool bHideHUD;

	//Dialogue
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dialogue Settings",
		meta = ( EditCondition = "ActionType == ETutorialActionType::Dialogue || ActionType == ETutorialActionType::SystemMessage", EditConditionHides ))
	FA1TutorialDialogue DialogueData;

	//Mission
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mission Settings",
		meta = ( EditCondition = "ActionType == ETutorialActionType::Mission", EditConditionHides ))
	FA1TutorialMission MissionData;

	//Wait Event
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Event Settings",
		meta = ( EditCondition = "ActionType == ETutorialActionType::WaitForEvent || ActionType == ETutorialActionType::WaitForInteraction", EditConditionHides ))
	FGameplayTag WaitEventTag;

	//Event Trigger
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Event Settings",
		meta = ( EditCondition = "ActionType == ETutorialActionType::TriggerEvent", EditConditionHides ))
	FGameplayTag TriggerEventTag;

	//CutScene
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cutscene Settings",
		meta = ( EditCondition = "ActionType == ETutorialActionType::Cutscene", EditConditionHides ))
	TSoftObjectPtr<class ULevelSequence> CutsceneSequence;

	//Delay
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Delay Settings",
		meta = ( EditCondition = "ActionType == ETutorialActionType::Delay", EditConditionHides ))
	float DelayDuration = 1.0f;

	//SpawnItem
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawn Item Template Setting",
		meta = ( EditCondition = "ActionType == ETutorialActionType::SpawnItem", EditConditionHides ))
	TSubclassOf<UA1ItemTemplate> SpawnItemTemplate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawn Transform Setting",
		meta = ( EditCondition = "ActionType == ETutorialActionType::SpawnItem", EditConditionHides ))
	FTransform SpawnTransform;

	//Navigate In Minimap
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Navigate Tag Setting",
		meta = ( EditCondition = "ActionType == ETutorialActionType::Navigate", EditConditionHides ))
	FGameplayTag NavigateTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Navigate Actor Class Setting",
		meta = ( EditCondition = "ActionType == ETutorialActionType::Navigate", EditConditionHides ))
	TSubclassOf<AActor> NavigateActorClass;

	//Highlight;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Highlight Actor Class Setting",
		meta = ( EditCondition = "ActionType == ETutorialActionType::Highlight", EditConditionHides ))
	TSubclassOf<AActor> HighlightActorClass;

	//Checker
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Checker Actor Class Setting",
		meta = ( EditCondition = "ActionType == ETutorialActionType::SpawnChecker", EditConditionHides ))
	TSubclassOf<AActor> CheckerClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Checker Spawn Transform Setting",
		meta = ( EditCondition = "ActionType == ETutorialActionType::SpawnChecker", EditConditionHides ))
	FTransform CheckerSpawnTransform;
	

	//Common
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Flow Control",
		meta = ( EditCondition = "ActionType != ETutorialActionType::None && ActionType != ETutorialActionType::DisableMovement && ActionType != ETutorialActionType::EnableMovement", EditConditionHides ))
	bool bAutoProgress = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Flow Control",
		meta = ( EditCondition = "bAutoProgress && ActionType != ETutorialActionType::None && ActionType != ETutorialActionType::DisableMovement && ActionType != ETutorialActionType::EnableMovement", EditConditionHides ))
	float AutoProgressDelay = 0.0f;

	// === 조건부 실행 ===
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Condition",
		meta = ( EditCondition = "ActionType != ETutorialActionType::None", EditConditionHides ))
	FGameplayTag ConditionTag;

	FA1TutorialAction()
	{
		ActionType = ETutorialActionType::None;
		ActionName = FText::FromString(TEXT("New Action"));
		FadeDuration = 1.0f;
		FadeColor = FLinearColor::Black;
		DelayDuration = 1.0f;
		bAutoProgress = true;
		AutoProgressDelay = 0.0f;
	}
};

USTRUCT(BlueprintType)
struct FA1TutorialStepInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Step Info", meta = ( DisplayPriority = "1" ))
	FString StepID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Step Info", meta = ( DisplayPriority = "2" ))
	FText StepName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Step Info", meta = ( DisplayPriority = "3" ))
	FText StepDescription;

	//Action Array
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Actions", meta = ( TitleProperty = "ActionName" ))
	TArray<FA1TutorialAction> Actions;

	//Control Flow
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Flow")
	FString NextStepID;

	//Start Condition
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Conditions")
	FGameplayTag StartConditionTag;

	//Complete Condition
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Conditions")
	FGameplayTag CompletionTag;

	FA1TutorialStepInfo()
	{
		StepID = TEXT("");
		StepName = FText::FromString(TEXT("New Step"));
		StepDescription = FText::FromString(TEXT(""));
		NextStepID = TEXT("");
	}
};

/**
 * Tutorial Data Asset
 */
UCLASS(BlueprintType, Const, meta = ( DisplayName = "A1 Tutorial Data" ))
class A1GAME_API UA1TutorialData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	static const UA1TutorialData& Get();

#if WITH_EDITOR
	virtual void PreSave(FObjectPreSaveContext ObjectSaveContext) override;
#endif //WITH_EDITOR

	UFUNCTION(BlueprintPure, Category = "Tutorial")
	FA1TutorialStepInfo GetTutorialStepInfoByStepName(FString InStepID);

	UFUNCTION(BlueprintPure, Category = "Tutorial")
	TArray<FA1TutorialStepInfo> GetAllTutorialStepInfos() const { return TutorialStepInfos; }

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tutorial Steps", meta = ( TitleProperty = "StepName" ))
	TArray<FA1TutorialStepInfo> TutorialStepInfos;
};