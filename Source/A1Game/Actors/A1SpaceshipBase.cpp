// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "Actors/A1SpaceshipBase.h"

#include "A1BedBase.h"
#include "A1DayNightManager.h"
#include "A1DoorBase.h"
#include "A1FuelBase.h"
#include "A1DockingSignalHandlerBase.h"
#include "A1RepairBase.h"
#include "A1ShipOutputBase.h"
#include "A1SignalDetectionBase.h"
#include "A1StorageBase.h"
#include "GameModes/LyraGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Maps/A1RandomMapGenerator.h"
#include "Net/UnrealNetwork.h"
#include "Score/A1ScoreBlueprintFunctionLibrary.h"
#include "Score/A1ScoreManager.h"
#include "Tutorial/A1TutorialManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1SpaceshipBase)

AA1SpaceshipBase::AA1SpaceshipBase()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void AA1SpaceshipBase::BeginPlay()
{
	Super::BeginPlay();

	if ( UA1ScoreManager::Get()->GetDoTutorial())
	{
		if ( UGameInstance* GameInstance = GetGameInstance() )
		{
			if ( UA1TutorialManager* TutorialManager = GameInstance->GetSubsystem<UA1TutorialManager>() )
			{
				TutorialManager->StartTutorial();
			}
		}
	}
	 
	UA1ScoreBlueprintFunctionLibrary::StartNewGame();

	if (HasAuthority())
	{
		FindComponentsByTags();

		if (!DockingSignalHandler || !CacheDoor || !FuelSystem || !ShipOutput || Beds.IsEmpty()/*|| Storages.IsEmpty()*/)
		{
			FindSpaceshipComponents();
		}
	}


	GetWorldTimerManager().SetTimer(FuelConsumeTimer, this, &AA1SpaceshipBase::ConsumeDefaultFuel, 1.f, true);

	UA1ScoreManager::Get()->OnGameEnded.AddDynamic(this, &AA1SpaceshipBase::OnStopFuelConsume);

	SpawnOneRepairBaseByTutoMode();

	FindAllRepairBases();
}

void AA1SpaceshipBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	if (HasAuthority() && !bGameEndHandled)
	{
		if (CurrentFuelAmount <= 0.0f)
		{
			HandleGameOver();
		}

		if (bMeetRescueShip)
		{
			HandleRescue();
		}
	}
}

void AA1SpaceshipBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AA1SpaceshipBase, CurrentFuelAmount);
	DOREPLIFETIME(AA1SpaceshipBase, bIsExternalMapActive);
	DOREPLIFETIME(AA1SpaceshipBase, GameEndState);
	DOREPLIFETIME(AA1SpaceshipBase, bMeetRescueShip);

}

void AA1SpaceshipBase::RegisterDoor(AA1DoorBase* Door)
{
	if (Door && HasAuthority())
	{
		CacheDoor = Door;
	}
}

void AA1SpaceshipBase::RegisterDockingSignalHandler(AA1DockingSignalHandlerBase* Signal)
{
	if (Signal && HasAuthority() && !DockingSignalHandler)
	{
		DockingSignalHandler = Signal;
	}
}

void AA1SpaceshipBase::RegisterBed(AA1BedBase* Bed)
{
	if (Bed && HasAuthority())
	{
		Beds.AddUnique(Bed);
	}
}

void AA1SpaceshipBase::RegisterFuelSystem(AA1FuelBase* Fuel)
{
	if (Fuel && HasAuthority() && !FuelSystem)
	{
		FuelSystem = Fuel;
	}
}

void AA1SpaceshipBase::RegisterStorage(AA1StorageBase* Storage)
{
	if (Storage && HasAuthority())
	{
		Storages.AddUnique(Storage);
	}
}

void AA1SpaceshipBase::RegisterShipOutput(AA1ShipOutputBase* Output)
{
	if (Output && HasAuthority() && !ShipOutput)
	{
		ShipOutput = Output;
	}
}

void AA1SpaceshipBase::RegisterSignalDetection(AA1SignalDetectionBase* Output)
{
	if (Output && HasAuthority() && !SignalDetection)
	{
		SignalDetection = Output;
	}
}

void AA1SpaceshipBase::HandleGameOver()
{
	if (!HasAuthority() || bGameEndHandled)
		return;

	bGameEndHandled = true;
	GameEndState = EGameEndState::GameOver;

	OnGameEndEvent.Broadcast(GameEndState);

	GetWorldTimerManager().ClearTimer(FuelConsumeTimer);

	
	if (ALyraGameMode* GameMode = Cast<ALyraGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GameMode->HandleGameEnd(this, false);
	}
}

void AA1SpaceshipBase::HandleRescue()
{
	if (!HasAuthority() || bGameEndHandled)
		return;

	bGameEndHandled = true;

	GameEndState = EGameEndState::Rescued;

	OnGameEndEvent.Broadcast(GameEndState);

	GetWorldTimerManager().ClearTimer(FuelConsumeTimer);

	if (ALyraGameMode* GameMode = Cast<ALyraGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GameMode->HandleGameEnd(this, true);
	}
}

bool AA1SpaceshipBase::IsRescued() const
{
	return GameEndState == EGameEndState::Rescued;
}

void AA1SpaceshipBase::SetMeetRescueShip(bool bMeetRescue)
{
	if (HasAuthority() && !bGameEndHandled)
	{
		bMeetRescueShip = bMeetRescue;

		if (bMeetRescueShip)
		{
			HandleRescue();
		}
	}
}

void AA1SpaceshipBase::CheckTwoDaysAgo(int32 NewDay)
{
	if (NewDay - CurrentDay >= 2)
	{
		BreakFoam();
		CurrentDay = NewDay;
	}
}

void AA1SpaceshipBase::BreakFoam()
{
	//Select RepairBase Random and change state
	int32 idx = FMath::RandRange(0, CachedNonBrokenRepairs.Num() - 1);

	CachedNonBrokenRepairs[ idx ]->SetCurrentState(RepairState::Break);
	UE_LOG(LogA1, Log, TEXT("[AA1Spaceship] %s Changed to break!"), *CachedNonBrokenRepairs[ idx ]->GetName());
	CachedNonBrokenRepairs[ idx ]->ActivateCheckOverlap();

	//Add Fuel Consume Amount after 5 seconds

	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, [ this ] ()
		{
			CurrentFuelConsumeAmount += 1.f;
		}, 5.f, false);

	CachedNonBrokenRepairs.RemoveAt(idx);

	//Game Over if all repair base activate
	if ( CachedNonBrokenRepairs.Num() == 0 )
	{
		HandleGameOver();
	}
}

void AA1SpaceshipBase::FindSpaceshipComponents()
{
	if (!HasAuthority())
		return;

	// 이미 할당된 참조가 있으면 재할당하지 않음
	if (!DockingSignalHandler)
	{
		TArray<AActor*> FoundDockingSignalHandlers;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AA1DockingSignalHandlerBase::StaticClass(), FoundDockingSignalHandlers);
		if (FoundDockingSignalHandlers.Num() > 0)
		{
			DockingSignalHandler = Cast<AA1DockingSignalHandlerBase>(FoundDockingSignalHandlers[0]);
		}
	}

	// Doors 가 비어있는 경우에만 찾기
	if (!CacheDoor)
	{
		TArray<AActor*> FoundDoor;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AA1DoorBase::StaticClass(), FoundDoor);
		if (FoundDoor.Num() > 0)
		{
			CacheDoor = Cast<AA1DoorBase>(FoundDoor[0]);
		}

	}

	if (!FuelSystem)
	{
		TArray<AActor*> FoundFuelSystems;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AA1FuelBase::StaticClass(), FoundFuelSystems);
		if (FoundFuelSystems.Num() > 0)
		{
			FuelSystem = Cast<AA1FuelBase>(FoundFuelSystems[0]);
		}
	}

	if (!ShipOutput)
	{
		TArray<AActor*> FoundShipOutputs;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AA1ShipOutputBase::StaticClass(), FoundShipOutputs);
		if (FoundShipOutputs.Num() > 0)
		{
			ShipOutput = Cast<AA1ShipOutputBase>(FoundShipOutputs[0]);
		}
	}

	if (Beds.IsEmpty())
	{
		TArray<AActor*> FoundBeds;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AA1BedBase::StaticClass(), FoundBeds);
		for (AActor* Actor : FoundBeds)
		{
			if (AA1BedBase* Bed = Cast<AA1BedBase>(Actor))
			{
				Beds.AddUnique(Bed);
			}
		}
	}

	if (Storages.IsEmpty())
	{
		TArray<AActor*> FoundStorages;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AA1StorageBase::StaticClass(), FoundStorages);
		for (AActor* Actor : FoundStorages)
		{
			if (AA1StorageBase* Storage = Cast<AA1StorageBase>(Actor))
			{
				Storages.AddUnique(Storage);
			}
		}
	}

	if (!SignalDetection)
	{
		TArray<AActor*> FoundStorages;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AA1SignalDetectionBase::StaticClass(), FoundStorages);
		for (AActor* Actor : FoundStorages)
		{
			if (AA1SignalDetectionBase* FoundSignalDetection = Cast<AA1SignalDetectionBase>(Actor))
			{
				SignalDetection = Cast<AA1SignalDetectionBase>(FoundSignalDetection);
			}
		}
	}
}

void AA1SpaceshipBase::FindComponentsByTags()
{
	if (!HasAuthority())
		return;

	TArray<AActor*> TaggedActors;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), SpaceshipComponentTag, TaggedActors);
	for (AActor* Actor : TaggedActors)
	{
		if (!IsValid(Actor))
			continue;
		if (Actor->ActorHasTag(DoorTag))
		{
			CacheDoor = Cast<AA1DoorBase>(Actor);;
		}
		else if (Actor->ActorHasTag(DockingSignalHandlerTag) && !DockingSignalHandler)
		{
			DockingSignalHandler = Cast<AA1DockingSignalHandlerBase>(Actor);
		}
		else if (Actor->ActorHasTag(BedTag))
		{
			AA1BedBase* Bed = Cast<AA1BedBase>(Actor);
			if (Bed)
			{
				Beds.AddUnique(Bed);
			}
		}
		else if (Actor->ActorHasTag(FuelSystemTag) && !FuelSystem)
		{
			FuelSystem = Cast<AA1FuelBase>(Actor);
		}
		else if (Actor->ActorHasTag(StorageTag))
		{
			AA1StorageBase* Storage = Cast<AA1StorageBase>(Actor);
			if (Storage)
			{
				Storages.AddUnique(Storage);
			}
		}
		else if (Actor->ActorHasTag(ShipOutputTag) && !ShipOutput)
		{
			ShipOutput = Cast<AA1ShipOutputBase>(Actor);
		}
	}
}

void AA1SpaceshipBase::SpawnOneRepairBaseByTutoMode()
{
	
	const FVector SpawnLocation = FVector(-6026.f, -600.f, 170.f);
	const FVector SpawnLocation2 = FVector(-6129.f, -600.f, 170.f);
	FActorSpawnParameters Params;

	if (UA1ScoreManager::Get()->GetDoTutorial())
	{
		AActor* Actor = GetWorld()->SpawnActor(PipeRepairBase, &SpawnLocation, &FRotator::ZeroRotator);
		if (AA1RepairBase* RepairBase = Cast<AA1RepairBase>(Actor) )
		{
			RepairBase->SetCurrentState(RepairState::NotBroken);
			UE_LOG(LogA1, Log, TEXT("RepairBase Name: %s"), *RepairBase->GetName());
			RepairBase->SetActorHiddenInGame(true);

			//Caching Specific Actor
			SpecificRepairBase = RepairBase;

			UE_LOG(LogA1, Log, TEXT("RepairBase Name: %s"), *SpecificRepairBase->GetName());
		}
	}
	else
	{
		AActor* Actor = GetWorld()->SpawnActor(DefaultRepairBaseClass, &SpawnLocation2, &FRotator::ZeroRotator);
		if ( AA1RepairBase* RepairBase = Cast<AA1RepairBase>(Actor) )
		{
			RepairBase->SetCurrentState(RepairState::NotBroken);
		}
	}
}

void AA1SpaceshipBase::BreakPipeRepairBase()
{
	SpecificRepairBase->SetActorHiddenInGame(false);
	SpecificRepairBase->SetCurrentState(RepairState::Break);
	SpecificRepairBase->ActivateCheckOverlap();
	CachedNonBrokenRepairs.Remove(SpecificRepairBase);

}

void AA1SpaceshipBase::OnRep_CurrentFuel()
{
	OnFuelChanged.Broadcast(CurrentFuelAmount);
}

void AA1SpaceshipBase::OnRep_GameEndState()
{
	// 클라이언트에서 게임 종료 이벤트 발생
	OnGameEndEvent.Broadcast(GameEndState);
}

void AA1SpaceshipBase::FindAllRepairBases()
{
	//TODO eric1306 -> Tutorial인지 아닌지에 따라 캐싱 객체 수정하게
	TArray<AActor*> Results;
	//AActor* 객체 -> A1RepairBase*로 변환
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AA1RepairBase::StaticClass(), OUT Results);
	for (auto Result : Results)
	{
		if (AA1RepairBase* Repair = Cast<AA1RepairBase>(Result))
		{
			CachedRepairs.Add(Repair);
		}
	}

	//Change State
	for (auto Repair : CachedRepairs)
	{
		Repair->SetCurrentState(RepairState::NotBroken);
		CachedNonBrokenRepairs.Add(Repair);
	}

	//튜토리얼이 아니라면 랜덤으로 10개의 객체 부수고 시작.
	if ( !UA1ScoreManager::Get()->GetDoTutorial() )
	{
		int32 iter = 10;
		while ( iter-- )
		{
			BreakFoam();
		}
	}
	AActor* Actor = UGameplayStatics::GetActorOfClass(GetWorld(), AA1DayNightManager::StaticClass());
	if (Actor)
	{
		if (AA1DayNightManager* DayNight = Cast<AA1DayNightManager>(Actor))
		{
			DayNight->OnDayChanged.AddDynamic(this, &AA1SpaceshipBase::CheckTwoDaysAgo);
		}
	}
	

	UE_LOG(LogA1, Log, TEXT("Find %d Repair Objects"), CachedRepairs.Num());
}

void AA1SpaceshipBase::SetIsExternamMapActive(bool InExternalMapActive)
{
	bIsExternalMapActive = InExternalMapActive;
	UE_LOG(LogTemp, Log, TEXT("Change ExternalMapActive : %s"), bIsExternalMapActive ? TEXT("True") :  TEXT("False"));
}

void AA1SpaceshipBase::AddFuel(float AmountToAdd)
{
	if (!HasAuthority()/*Only Server*/)
		return;
	if (AmountToAdd <= 0.f)
		return;

	const float PreviousFuel = CurrentFuelAmount;
	CurrentFuelAmount = FMath::Min(CurrentFuelAmount + AmountToAdd, MaxFuelAmount);
	UA1ScoreBlueprintFunctionLibrary::SetRemainingFuel(CurrentFuelAmount);

	if (PreviousFuel != CurrentFuelAmount)
	{
		OnRep_CurrentFuel();
	}

}

void AA1SpaceshipBase::ConsumeFuel(float AmountToConsume)
{
	if (!HasAuthority()/*Only Server*/)
		return;

	if (AmountToConsume <= 0.f)
		return;

	if (FuelSystem == nullptr)
	{
		GetWorldTimerManager().ClearTimer(FuelConsumeTimer);
		return;
	}

	const float PreviousFuel = CurrentFuelAmount;
	CurrentFuelAmount = FMath::Max(CurrentFuelAmount - AmountToConsume, 0.0f);
	UA1ScoreBlueprintFunctionLibrary::SetRemainingFuel(CurrentFuelAmount);

	if (PreviousFuel != CurrentFuelAmount)
	{
		OnRep_CurrentFuel();
	}
}

void AA1SpaceshipBase::ConsumeDefaultFuel()
{

	if (!HasAuthority()/*Only Server*/)
		return;

	if (CurrentFuelConsumeAmount <= 0.f)
		return;

	if (FuelSystem == nullptr)
	{
		GetWorldTimerManager().ClearTimer(FuelConsumeTimer);
		return;
	}

	const float PreviousFuel = CurrentFuelAmount;
	CurrentFuelAmount = FMath::Max(CurrentFuelAmount - CurrentFuelConsumeAmount, 0.0f);
	UA1ScoreBlueprintFunctionLibrary::SetRemainingFuel(CurrentFuelAmount);

	if (PreviousFuel != CurrentFuelAmount)
	{
		OnRep_CurrentFuel();
	}
}

void AA1SpaceshipBase::ActivateExternalMap()
{
	if (!HasAuthority()/*Only Server*/)
		return;

	bIsExternalMapActive = true;

}

void AA1SpaceshipBase::DeactivateExternalMap()
{
	if (!HasAuthority()/*Only Server*/)
		return;

	bIsExternalMapActive = false;

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AA1RandomMapGenerator::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		AA1RandomMapGenerator* TargetActor = Cast<AA1RandomMapGenerator>(Actor);
		if (TargetActor && TargetActor->GetbDungeonGenerateComplete())
		{
			TargetActor->Server_ResetMap();
			DockingSignalHandler->SetSignalState(ESignalState::Released);
			break;
		}
	}
	OnDeactivateMap.Broadcast();
}

bool AA1SpaceshipBase::IsGameOver() const
{
	return GameEndState == EGameEndState::GameOver;
}

bool AA1SpaceshipBase::HasEnoughFuelToSurvive() const
{
	constexpr float MinimumFuelToSurvive = 100.0f;
	return CurrentFuelAmount >= MinimumFuelToSurvive;
}

void AA1SpaceshipBase::OnStopFuelConsume(const FA1ScoreData& FinalScore)
{
	GetWorldTimerManager().ClearTimer(FuelConsumeTimer);

	for (auto it : CachedRepairs)
	{
		if (it->CurrentState == RepairState::NotBroken)
		{
			continue;
		}
		else if (it->CurrentState == RepairState::Break)
		{
			UA1ScoreManager::Get()->SetTotalRepair(UA1ScoreManager::Get()->GetTotalRepair() + 1);
		}
		else //Complete
		{
			UA1ScoreManager::Get()->SetTotalRepair(UA1ScoreManager::Get()->GetTotalRepair() + 1);
			UA1ScoreManager::Get()->SetCompleteRepair(UA1ScoreManager::Get()->GetCompleteRepair() + 1);
		}
	}
}

