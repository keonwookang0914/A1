// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraGameMode.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "A1LogChannels.h"
#include "AbilitySystemComponent.h"
#include "System/LyraAssetManager.h"
#include "LyraGameState.h"
#include "Player/LyraPlayerController.h"
#include "Player/LyraPlayerState.h"
#include "Character/LyraCharacter.h"
#include "Character/LyraPawnExtensionComponent.h"
#include "Character/LyraPawnData.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "AbilitySystem/Attributes/A1CharacterAttributeSet.h"
#include "Score/A1ScoreBlueprintFunctionLibrary.h"
#include "Score/A1ScoreManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraGameMode)

ALyraGameMode::ALyraGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	GameStateClass = ALyraGameState::StaticClass();
	PlayerControllerClass = ALyraPlayerController::StaticClass();
	PlayerStateClass = ALyraPlayerState::StaticClass();
	DefaultPawnClass = ALyraCharacter::StaticClass();
}

const ULyraPawnData* ALyraGameMode::GetPawnDataForController(const AController* InController) const
{
	// See if pawn data is already set on the player state
	if (InController != nullptr)
	{
		if (const ALyraPlayerState* LyraPS = InController->GetPlayerState<ALyraPlayerState>())
		{
			if (const ULyraPawnData* PawnData = LyraPS->GetPawnData<ULyraPawnData>())
			{
				return PawnData;
			}
		}
	}

	// If not, fall back to the default pawn data (DefaultPawnData in the asset manager config)
	return ULyraAssetManager::Get().GetDefaultPawnData();
}

void ALyraGameMode::TriggerFadeOnAllPlayer(float From, float To, float Duration, FLinearColor Color)
{
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		if (ALyraPlayerController* PC = Cast<ALyraPlayerController>(Iterator->Get()))
		{
			PC->Client_FadeCamera(From, To, Duration, Color, true);
		}
	}
}

void ALyraGameMode::TriggerFadeOnSpecificPlayer(ALyraPlayerController* TargetPC, float From, float To)
{
	if (TargetPC)
	{
		TargetPC->Client_FadeCamera(From, To, 1.0f, FLinearColor::Black, true);
	}
}

UClass* ALyraGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	if (const ULyraPawnData* PawnData = GetPawnDataForController(InController))
	{
		if (PawnData->PawnClass)
		{
			return PawnData->PawnClass;
		}
	}

	return Super::GetDefaultPawnClassForController_Implementation(InController);
}

APawn* ALyraGameMode::SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform)
{
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Instigator = GetInstigator();
	SpawnInfo.ObjectFlags |= RF_Transient;	// Never save the default player pawns into a map.
	SpawnInfo.bDeferConstruction = true;

	if (UClass* PawnClass = GetDefaultPawnClassForController(NewPlayer))
	{
		if (APawn* SpawnedPawn = GetWorld()->SpawnActor<APawn>(PawnClass, SpawnTransform, SpawnInfo))
		{
			if (ULyraPawnExtensionComponent* PawnExtComp = ULyraPawnExtensionComponent::FindPawnExtensionComponent(SpawnedPawn))
			{
				if (const ULyraPawnData* PawnData = GetPawnDataForController(NewPlayer))
				{
					PawnExtComp->SetPawnData(PawnData);
				}
				else
				{
					UE_LOG(LogA1, Error, TEXT("Game mode was unable to set PawnData on the spawned pawn [%s]."), *GetNameSafe(SpawnedPawn));
				}
			}

			SpawnedPawn->FinishSpawning(SpawnTransform);

			return SpawnedPawn;
		}
		else
		{
			UE_LOG(LogA1, Error, TEXT("Game mode was unable to spawn Pawn of class [%s] at [%s]."), *GetNameSafe(PawnClass), *SpawnTransform.ToHumanReadableString());
		}
	}
	else
	{
		UE_LOG(LogA1, Error, TEXT("Game mode was unable to spawn Pawn due to NULL pawn class."));
	}

	return nullptr;
}

bool ALyraGameMode::PlayerCanRestart_Implementation(APlayerController* Player)
{
	return ControllerCanRestart(Player);
}

bool ALyraGameMode::ControllerCanRestart(AController* Controller)
{
	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{	
		if (!Super::PlayerCanRestart_Implementation(PC))
		{
			return false;
		}
	}
	else
	{
		// Bot version of Super::PlayerCanRestart_Implementation
		if ((Controller == nullptr) || Controller->IsPendingKillPending())
		{
			return false;
		}
	}

	return true;
}

void ALyraGameMode::HandleGameEnd(AActor* GameEndInstigator, bool bIsRescued)
{
	if (bIsHandlingGameEnd)
	{
		return;
	}

	bIsHandlingGameEnd = true;

	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		if (ALyraPlayerController* PC = Cast<ALyraPlayerController>(Iterator->Get()))
		{
			// Fade Effect
			FLinearColor FadeColor = bIsRescued ? FLinearColor::Blue : FLinearColor::Red;
			PC->Client_FadeCamera(0.0f, 1.0f, 1.0f, FadeColor, true);

			PC->DisableInput(PC);

			//Temp eric1306 Fuel zero -> Game Over
			if (ALyraCharacter* LyraCharacter = Cast<ALyraCharacter>(PC->GetPawn()))
			{
				if (LyraCharacter->GetDeathState() == EA1DeathState::NotDead)
				{
					if (UAbilitySystemComponent* ASC = LyraCharacter->GetAbilitySystemComponent())
					{
						if (const UA1CharacterAttributeSet* Attribute = ASC->GetSet<UA1CharacterAttributeSet>())
						{
							Attribute->OnOutOfHealth.Broadcast(LyraCharacter, Attribute->GetHealth(), 0.f);
						}
					}
				}
			}
		}
	}

	if (bIsRescued)
	{
		OnShowRescuedUI();
	}
	else
	{
		OnShowGameOverUI();
	}
	FTimerHandle RestartTimerHandle;
	GetWorldTimerManager().SetTimer(RestartTimerHandle, [this]()
		{
			//TODO eric1306 : fix hard coding
			UA1ScoreBlueprintFunctionLibrary::EndGame(EGameEndReason::Death);
			
			//UGameplayStatics::OpenLevel(GetWorld(), FName(*GetWorld()->GetName()), true);
		}, GameRestartDelay, false);
}

bool ALyraGameMode::AreAllPlayersDead()
{
	int32 TotalPlayers = 0;
	int32 DeadPlayersCount = DeadPlayers.Num();

	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PC = Iterator->Get();
		if (PC && !PC->IsInState(NAME_Spectating))
		{
			TotalPlayers++;
		}
	}

	if (TotalPlayers == 0)
	{
		return false;
	}

	return DeadPlayersCount >= TotalPlayers;
}

void ALyraGameMode::OnPlayerDied(AController* Controller)
{
	if (!Controller)
	{
		return;
	}

	if (!DeadPlayers.Contains(Controller))
	{
		DeadPlayers.Add(Controller);

		if (AreAllPlayersDead())
		{
			UE_LOG(LogTemp, Warning, TEXT("All players are dead! Triggering game over."));
			HandleGameEnd(Controller, false);
		}
	}
}

void ALyraGameMode::GenericPlayerInitialization(AController* NewPlayer)
{
	Super::GenericPlayerInitialization(NewPlayer);

	OnGameModePlayerInitialized.Broadcast(this, NewPlayer);
}

void ALyraGameMode::RequestPlayerRestartNextFrame(AController* Controller, bool bForceReset)
{
	if (bForceReset && (Controller != nullptr))
	{
		Controller->Reset();
	}

	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		GetWorldTimerManager().SetTimerForNextTick(PC, &APlayerController::ServerRestartPlayer_Implementation);
	}
}

void ALyraGameMode::FailedToRestartPlayer(AController* NewPlayer)
{
	Super::FailedToRestartPlayer(NewPlayer);

	// If we tried to spawn a pawn and it failed, lets try again *note* check if there's actually a pawn class
	// before we try this forever.
	if (UClass* PawnClass = GetDefaultPawnClassForController(NewPlayer))
	{
		if (APlayerController* NewPC = Cast<APlayerController>(NewPlayer))
		{
			// If it's a player don't loop forever, maybe something changed and they can no longer restart if so stop trying.
			if (PlayerCanRestart(NewPC))
			{
				RequestPlayerRestartNextFrame(NewPlayer, false);			
			}
			else
			{
				UE_LOG(LogA1, Verbose, TEXT("FailedToRestartPlayer(%s) and PlayerCanRestart returned false, so we're not going to try again."), *GetPathNameSafe(NewPlayer));
			}
		}
		else
		{
			RequestPlayerRestartNextFrame(NewPlayer, false);
		}
	}
	else
	{
		UE_LOG(LogA1, Verbose, TEXT("FailedToRestartPlayer(%s) but there's no pawn class so giving up."), *GetPathNameSafe(NewPlayer));
	}
}
