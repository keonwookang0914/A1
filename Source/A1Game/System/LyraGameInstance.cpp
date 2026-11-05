// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraGameInstance.h"

#include "Components/GameFrameworkComponentManager.h"
#include "A1GameplayTags.h"
#include "Player/LyraPlayerController.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraGameInstance)

ULyraGameInstance::ULyraGameInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void ULyraGameInstance::Init()
{
	Super::Init();

	// Register our custom init states
	UGameFrameworkComponentManager* ComponentManager = GetSubsystem<UGameFrameworkComponentManager>(this);

	if (ensure(ComponentManager))
	{
		ComponentManager->RegisterInitState(A1GameplayTags::InitState_Spawned, false, FGameplayTag());
		ComponentManager->RegisterInitState(A1GameplayTags::InitState_DataAvailable, false, A1GameplayTags::InitState_Spawned);
		ComponentManager->RegisterInitState(A1GameplayTags::InitState_DataInitialized, false, A1GameplayTags::InitState_DataAvailable);
		ComponentManager->RegisterInitState(A1GameplayTags::InitState_GameplayReady, false, A1GameplayTags::InitState_DataInitialized);
	}
}

ALyraPlayerController* ULyraGameInstance::GetPrimaryPlayerController() const
{
	return Cast<ALyraPlayerController>(Super::GetPrimaryPlayerController(false));
}
