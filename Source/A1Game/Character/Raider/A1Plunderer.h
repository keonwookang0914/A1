// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "Character/Raider/A1RaiderBase.h"
#include "A1Plunderer.generated.h"

class UA1CharacterAttributeSet;
class AA1EquipmentBase;

UCLASS()
class A1GAME_API AA1Plunderer : public AA1RaiderBase
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AA1Plunderer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void SpawnDropItem() override;
};
