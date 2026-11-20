// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "A1RaiderController.generated.h"

UCLASS()
class A1GAME_API AA1RaiderController : public AAIController
{
	GENERATED_BODY()
	
public:
	AA1RaiderController();
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

public:
	static const FName HomePosKey;
	static const FName PatrolPosKey;
	static const FName AggroTargetKey;
	static const FName CanAttackKey;

private:
	UPROPERTY()
	class UBehaviorTree* BTAsset;

	UPROPERTY()
	class UBlackboardData* BBAsset;
};
