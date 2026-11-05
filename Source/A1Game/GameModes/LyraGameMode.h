// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/GameModeBase.h"

#include "LyraGameMode.generated.h"

class ALyraPlayerController;
class AActor;
class AController;
class AGameModeBase;
class APawn;
class APlayerController;
class UClass;
class ULyraPawnData;
class UObject;
struct FFrame;

/**
 * Post login event, triggered when a player or bot joins the game as well as after seamless and non seamless travel
 *
 * This is called after the player has finished initialization
 */
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnLyraGameModePlayerInitialized, AGameModeBase* /*GameMode*/, AController* /*NewPlayer*/);

/**
 * ALyraGameMode
 *
 *	The base game mode class used by this project.
 */
UCLASS(Config = Game, Meta = (ShortTooltip = "The base game mode class used by this project."))
class ALyraGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:

	ALyraGameMode(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "Lyra|Pawn")
	const ULyraPawnData* GetPawnDataForController(const AController* InController) const;

	//~AGameModeBase interface
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
	virtual APawn* SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform) override;
	virtual bool PlayerCanRestart_Implementation(APlayerController* Player) override;
	virtual void GenericPlayerInitialization(AController* NewPlayer) override;
	virtual void FailedToRestartPlayer(AController* NewPlayer) override;
	//~End of AGameModeBase interface

	// Restart (respawn) the specified player or bot next frame
	// - If bForceReset is true, the controller will be reset this frame (abandoning the currently possessed pawn, if any)
	UFUNCTION(BlueprintCallable)
	void RequestPlayerRestartNextFrame(AController* Controller, bool bForceReset = false);

	// Agnostic version of PlayerCanRestart that can be used for both player bots and players
	virtual bool ControllerCanRestart(AController* Controller);

	// Delegate called on player initialization, described above 
	FOnLyraGameModePlayerInitialized OnGameModePlayerInitialized;

	/***********************
	 * temp eric1306
	 ***********************/

	 // HandleGameOver -> HandleGameEnd
	UFUNCTION(BlueprintCallable, Category = "Lyra|GameMode")
	void HandleGameEnd(AActor* GameEndInstigator, bool bIsRescued);

	UFUNCTION(BlueprintImplementableEvent, Category = "Lyra|GameMode")
	void OnShowGameOverUI();

	UFUNCTION(BlueprintImplementableEvent, Category = "Lyra|GameMode")
	void OnShowRescuedUI();

	UFUNCTION(BlueprintCallable, Category = "Lyra|GameMode")
	bool AreAllPlayersDead();

	UFUNCTION(BlueprintCallable, Category = "Lyra|GameMode")
	void OnPlayerDied(AController* Controller);

	/***********************
	 * temp code
	 ***********************/

	//Custom Function
public:

	UFUNCTION(BlueprintCallable)
	void TriggerFadeOnAllPlayer(float From, float To, float Duration, FLinearColor Color);

	UFUNCTION(BlueprintCallable)
	void TriggerFadeOnSpecificPlayer(ALyraPlayerController* TargetPC, float From, float To);

	/***********************
	 * temp eric1306
	 ***********************/
private:
	
	UPROPERTY(EditDefaultsOnly, Category = "Lyra|GameEnd")
	float GameRestartDelay = 5.0f;

	bool bIsHandlingGameEnd = false;

	UPROPERTY()
	TArray<TObjectPtr<AController>> DeadPlayers;

	/***********************
	 * temp code
	 ***********************/
};
