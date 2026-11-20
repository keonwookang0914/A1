// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Character/A1CreatureBase.h"
#include "InputActionValue.h"
#include "A1PlayerCharacter.generated.h"

class ULyraCameraComponent;

UCLASS()
class A1GAME_API AA1PlayerCharacter : public AA1CreatureBase
{
	GENERATED_BODY()

public:
	AA1PlayerCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void PossessedBy(AController* NewController) override;
	virtual void BeginPlay() override;

// Component
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lyra|Character", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ULyraCameraComponent> CameraComponent;
};
