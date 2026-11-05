// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/WorldSettings.h"
#include "LyraWorldSettings.generated.h"

/**
 * The default world settings object
 */
UCLASS()
class A1GAME_API ALyraWorldSettings : public AWorldSettings
{
	GENERATED_BODY()

public:

	ALyraWorldSettings(const FObjectInitializer& ObjectInitializer);
};
