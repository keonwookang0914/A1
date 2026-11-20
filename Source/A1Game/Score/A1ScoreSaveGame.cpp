// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "Score/A1ScoreSaveGame.h"

#include "A1ScoreData.h"

UA1ScoreSaveGame::UA1ScoreSaveGame()
{
	HighestScore = 0;
}

void UA1ScoreSaveGame::AddScore(const FA1ScoreData& NewScore)
{
	AllScores.Add(NewScore);

	if (NewScore.TotalScore > HighestScore)
	{
		HighestScore = NewScore.TotalScore;
	}

	//Sort
	AllScores.Sort([](const FA1ScoreData& A, const FA1ScoreData& B)
		{
			return A.TotalScore > B.TotalScore;
		});

	if (AllScores.Num() > 30)
	{
		AllScores.SetNum(30);
	}
}

void UA1ScoreSaveGame::ClearAllScore()
{
	AllScores.Empty();
	HighestScore = 0;
}
