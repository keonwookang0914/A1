// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "A1RaiderController.h"
#include "A1LogChannels.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Raider/A1RaiderBase.h"
#include "Data/A1RaiderData.h"
#include "System/LyraAssetManager.h"

const FName AA1RaiderController::HomePosKey(TEXT("HomePos"));
const FName AA1RaiderController::PatrolPosKey(TEXT("PatrolPos"));
const FName AA1RaiderController::AggroTargetKey(TEXT("AggroTarget"));
const FName AA1RaiderController::CanAttackKey(TEXT("CanAttack"));


AA1RaiderController::AA1RaiderController()
{
}

void AA1RaiderController::BeginPlay()
{
	Super::BeginPlay();

}

void AA1RaiderController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	const UA1RaiderData& RaiderData = ULyraAssetManager::Get().GetRaiderData();
	if (AA1RaiderBase* PossesedPawn = Cast<AA1RaiderBase>(InPawn))
	{
		const FA1RaiderBaseSet& RaiderBase = RaiderData.GetRaiderDataSet(PossesedPawn->RaiderType);
		
		if (RaiderBase.BBAsset != nullptr && RaiderBase.BTAsset != nullptr)
		{
			BBAsset = RaiderBase.BBAsset;
			BTAsset = RaiderBase.BTAsset;
		}
	}

	UBlackboardComponent* BlackboardComp = Blackboard;
	if (UseBlackboard(BBAsset, BlackboardComp))
	{
		BlackboardComp->SetValueAsVector(HomePosKey, InPawn->GetActorLocation());
		if (!RunBehaviorTree(BTAsset))
		{
			UE_LOG(LogA1Raider, Warning, TEXT("AIController couldn't run behavior tree!"));
		}
	}
}

void AA1RaiderController::OnUnPossess()
{
	Super::OnUnPossess();
}
