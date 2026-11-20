// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#include "A1Plunderer.h"
#include "A1LogChannels.h"
#include "Actors/A1StorageBase.h"
#include "Actors/A1RepairBase.h"
#include "Actors/A1EquipmentBase.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Controller/Raider/A1RaiderController.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1Plunderer)

// Sets default values
AA1Plunderer::AA1Plunderer()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	warlike = false;
}

// Called when the game starts or when spawned
void AA1Plunderer::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogA1Raider, Log, TEXT("Plunderer: Beginplay Call"));

	AAIController* AIController = Cast<AAIController>(GetController());
	if (AIController && AIController->GetBlackboardComponent())
	{
		UBlackboardComponent* BlackBoard = AIController->GetBlackboardComponent();
		if (BlackBoard)
		{
			BlackBoard->SetValueAsVector(AA1RaiderController::HomePosKey, GetActorLocation());

			AA1StorageBase* Storage = Cast<AA1StorageBase>(UGameplayStatics::GetActorOfClass(GetWorld(), AA1StorageBase::StaticClass()));
			if (Storage)
			{
				
				BlackBoard->SetValueAsBool(AA1RaiderController::CanAttackKey, true);
				BlackBoard->SetValueAsObject(AA1RaiderController::AggroTargetKey, Storage);
			}
		}
	}
}

void AA1Plunderer::SpawnDropItem()
{
	// 아이템 스폰
	int ItemNum = dropItems.Num();
	if (ItemNum > 0)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		FVector ItemSpawnLocation = GetActorLocation();
		ItemSpawnLocation.Z = 0;

		for (int i = 0; i < ItemNum; i++)
		{
			GetWorld()->SpawnActor<AA1EquipmentBase>(dropItems[i], ItemSpawnLocation, FRotator::ZeroRotator, SpawnParameters);
		}	
	}

	DeatState = EA1DeathState::DeathFinished;
}