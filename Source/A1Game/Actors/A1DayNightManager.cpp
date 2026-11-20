// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "Actors/A1DayNightManager.h"

#include "A1BedBase.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Score/A1ScoreBlueprintFunctionLibrary.h"
#include "Score/A1ScoreManager.h"

AA1DayNightManager::AA1DayNightManager()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	/*
	 * - 20 minutes = 1 day
	 * - 10 minue day/night
	 */
	DayDurationMinutes = 1.f; //eric1306->OriginalValue : 20.f
	DayPhaseDurationMinutes = 0.5f; //eric1306->OriginalValue : 10.f

	CurrentPhase = EDayPhase::Day;
	CurrentDay = 1;
	DayProgress = 0.f;
	ElapsedTime = 0.f;

	//minute -> second
	PhaseChangeDuration = DayPhaseDurationMinutes * 60.f;

	//Init
	LastUpdatedHour = -1;
	LastUpdatedMinute = -1;
}

void AA1DayNightManager::BeginPlay()
{
	Super::BeginPlay();

	// Init Singleton only server
	if (HasAuthority())
	{
		//enroll all bed in server
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AA1BedBase::StaticClass(), OUT FoundActors);
		for (auto Actor : FoundActors)
		{
			if (AA1BedBase* BedActor = Cast<AA1BedBase>(Actor))
			{
				OccupiedBeds.Add(BedActor);
			}
		}
		UE_LOG(LogTemp, Log, TEXT("Find %d Beds!"), OccupiedBeds.Num());
	}
	UA1ScoreManager::Get()->OnGameEnded.AddDynamic(this, &AA1DayNightManager::OnStopUpdateTime);
}

void AA1DayNightManager::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AA1DayNightManager, CurrentPhase);
	DOREPLIFETIME(AA1DayNightManager, CurrentDay);
	DOREPLIFETIME(AA1DayNightManager, DayProgress);
}

void AA1DayNightManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!HasAuthority())
		return;

	if (!bIsActive)
		return;

	UpdateTime(DeltaTime);

	if (!SkipNightflag && AreAllPlayersSleeping()) 
	{
		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle, [this]()
			{
				TrySkipNight();
			}, 10.f, false); //TODO eric1306 Original Value : 10.f(10 second)
		SkipNightflag = true;
	}
}

void AA1DayNightManager::TrySkipNight()
{
	if (!HasAuthority()) {
		return;
	}

	if (AreAllPlayersSleeping()) {
		if (CurrentPhase == EDayPhase::Day)
		{
			ElapsedTime = DayDurationMinutes * 30.f;
			DayProgress = 0.5f;
			CurrentPhase = EDayPhase::Night;
		}
		else
		{
			ElapsedTime = 0.0f;
			DayProgress = 0.0f;
			CurrentDay++;
			UA1ScoreBlueprintFunctionLibrary::AddDaySurvived();
			CurrentPhase = EDayPhase::Day;
		}

		OnDayChanged.Broadcast(CurrentDay);
		OnDayPhaseChanged.Broadcast(CurrentPhase, CurrentDay);

		WakeAllPlayers();
	}
}

void AA1DayNightManager::SetPlayerSleeping(AActor* Player, bool bIsSleeping)
{
	if (!HasAuthority() || Player == nullptr) {
		return;
	}
	if (SleepingPlayers.Find(Player))
	{
		SleepingPlayers[Player] = bIsSleeping;
	}
	else
	{
		SleepingPlayers.Add(Player, bIsSleeping);
	}
}

void AA1DayNightManager::GetGameTime(int32& OutHours, int32& OutMinutes) const
{
	constexpr float TotalDayMinutes = 24.0f * 60.0f;
	const float ElapsedMinutes = DayProgress * TotalDayMinutes;

	OutHours = FMath::FloorToInt(ElapsedMinutes / 60.0f);
	OutMinutes = FMath::FloorToInt(FMath::Fmod(ElapsedMinutes, 60.0f));
}

FString AA1DayNightManager::GetTimeString() const
{
	int32 Hours, Minutes;
	GetGameTime(Hours, Minutes);

	return FString::Printf(TEXT("%02d:%02d"), Hours, Minutes);

}

void AA1DayNightManager::WakeAllPlayers()
{
	if (!HasAuthority()) {
		return;
	}

	for (auto& Pair : SleepingPlayers) {
		Pair.Value = false;
	}

	for (AA1BedBase* Bed : OccupiedBeds)
	{
		if (Bed && Bed->GetBedState() == EBedState::Occupied)
		{
			//Bed->Multicast_Wakeup();
			//Temp Code eric1306 for standalone
			Bed->WakeUpOccupyingCharacter();
		}
	}

	OnWakeUpPlayers.Broadcast();

	SkipNightflag = false;
}

void AA1DayNightManager::OnStopUpdateTime(const FA1ScoreData& FinalScore)
{
	bIsActive = false;
}

void AA1DayNightManager::UpdateTime(float DeltaTime)
{
	//Add Time
	ElapsedTime += DeltaTime;
	DayProgress = ElapsedTime / (DayDurationMinutes * 60.f);

	if (DayProgress >= 1.f)
	{
		ElapsedTime = 0.f;
		DayProgress = 0.f;
		CurrentDay++;
		UA1ScoreBlueprintFunctionLibrary::AddDaySurvived();
		OnDayChanged.Broadcast(CurrentDay);
	}

	if (CurrentPhase == EDayPhase::Day && ElapsedTime >= PhaseChangeDuration)
	{
		ChangePhase(EDayPhase::Night);
	}
	else if (CurrentPhase == EDayPhase::Night && ElapsedTime == 0.f) {
		ChangePhase(EDayPhase::Day);
	}

	int32 CurrentHour, CurrentMinute;
	GetGameTime(CurrentHour, CurrentMinute);

	if (CurrentHour != LastUpdatedHour || CurrentMinute != LastUpdatedMinute)
	{
		LastUpdatedHour = CurrentHour;
		LastUpdatedMinute = CurrentMinute;
		OnTimeChanged.Broadcast(CurrentHour, CurrentMinute);
	}
}

void AA1DayNightManager::ChangePhase(EDayPhase NewPhase)
{
	if (CurrentPhase != NewPhase) {
		CurrentPhase = NewPhase;
		OnDayPhaseChanged.Broadcast(CurrentPhase, CurrentDay);
	}
}

bool AA1DayNightManager::AreAllPlayersSleeping() const
{
	if (SleepingPlayers.Num() == 0 || SleepingPlayers.Num() < GetActivePlayerCount(GetWorld())) {
		return false;
	}
	//UE_LOG(LogTemp, Log, TEXT("Connected Clients: %d"), GetActivePlayerCount(GetWorld()));
	for (const auto& Pair : SleepingPlayers) {
		if (!Pair.Value) {
			return false;
		}
	}

	return true;
}

void AA1DayNightManager::OnRep_CurrentPhase()
{
	OnDayPhaseChanged.Broadcast(CurrentPhase, CurrentDay);
}

void AA1DayNightManager::OnRep_CurrentDay()
{
	OnDayChanged.Broadcast(CurrentDay);
}

int32 AA1DayNightManager::GetActivePlayerCount(const UObject* WorldContextObject) const
{
	if (WorldContextObject == nullptr)
		return 0;

	UWorld* World = WorldContextObject->GetWorld();
	if (!World || World->GetNetMode() == NM_Client)
	{
		return 0;
	}

	AGameStateBase* GameState = World->GetGameState<AGameStateBase>();
	if (!GameState)
	{
		return 0;
	}

	return GameState->PlayerArray.Num();
}


