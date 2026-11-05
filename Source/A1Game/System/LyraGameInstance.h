// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/GameInstance.h"

#include "LyraGameInstance.generated.h"

class ALyraPlayerController;
class UObject;

UCLASS(Config = Game)
class A1GAME_API ULyraGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:

	ULyraGameInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	ALyraPlayerController* GetPrimaryPlayerController() const;

protected:

	virtual void Init() override;
};
