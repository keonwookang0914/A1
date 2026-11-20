// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "UI/Tutorial/A1PlayTutorialVideoWidget.h"

#include "MediaPlayer.h"
#include "MediaSource.h"
#include "Components/Image.h"
#include "Data/A1AssetData.h"

void UA1PlayTutorialVideoWidget::StartMovie()
{
	FSoftObjectPath AssetPath = UA1AssetData::Get().GetAssetPathByName(FName("M_ShortStory"));
	UObject* LoadedObject = AssetPath.TryLoad();
	UMaterial* MI = Cast<UMaterial>(LoadedObject);

	if (MI == nullptr)
	{
		return;
	}

	TutorialVideo->SetBrushFromMaterial(MI);

	FSoftObjectPath MediaPlayerPath = UA1AssetData::Get().GetAssetPathByName(FName("MP_ShortStory"));
	UObject* LoadedMP = MediaPlayerPath.TryLoad();
	UMediaPlayer* MP = Cast<UMediaPlayer>(LoadedMP);
	if (MP == nullptr)
	{
		return;
	}

	FSoftObjectPath MediaSourcePath = UA1AssetData::Get().GetAssetPathByName(FName("ShortStory"));
	UObject* LoadedMS = MediaSourcePath.TryLoad();
	UMediaSource* MS = Cast<UMediaSource>(LoadedMS);

	if (MS == nullptr)
	{
		return;
	}

	bool PlayResult = MP->OpenSource(MS);
	if (PlayResult)
	{
		MP->Rewind();
	}
}
