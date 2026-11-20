// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#include "Data/A1RaiderData.h"
#include "System/LyraAssetManager.h"
#include "UObject/ObjectSaveContext.h"

const UA1RaiderData& UA1RaiderData::Get()
{
	return ULyraAssetManager::Get().GetRaiderData();
}

#if WITH_EDITOR
void UA1RaiderData::PreSave(FObjectPreSaveContext ObjectSaveContext)
{
	Super::PreSave(ObjectSaveContext);

	RaiderDataMap.KeySort([](const ERaiderType A, const ERaiderType B)
		{
			return (A < B);
		});
}
#endif // WITH_EDITOR

const FA1RaiderBaseSet& UA1RaiderData::GetRaiderDataSet(ERaiderType RaiderType) const
{
	if (RaiderDataMap.Contains(RaiderType) == false)
	{
		static FA1RaiderBaseSet EmptyEntry;
		return EmptyEntry;
	}

	return RaiderDataMap.FindChecked(RaiderType);
}
