// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "A1ScoreSaveGame.generated.h"

struct FA1ScoreData;
/**
 * 
 */
UCLASS(BlueprintType)
class A1GAME_API UA1ScoreSaveGame : public USaveGame
{
	GENERATED_BODY()
public:
	UA1ScoreSaveGame();

	UFUNCTION(BlueprintCallable, Category = "Score")
	void AddScore(const FA1ScoreData& NewScore);

	UFUNCTION(BlueprintCallable, Category = "Score")
	void ClearAllScore();

	FORCEINLINE TArray<FA1ScoreData> GetAllScores() const { return AllScores; }
	FORCEINLINE int32 GetHighestScore() const { return HighestScore; }

protected:
	UPROPERTY(BlueprintReadWrite, Category = "Score")
	TArray<FA1ScoreData> AllScores;

	UPROPERTY(BlueprintReadWrite, Category = "Score")
	int32 HighestScore;

};
