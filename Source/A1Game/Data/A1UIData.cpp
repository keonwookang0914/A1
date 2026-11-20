// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#include "A1UIData.h"
#include "System/LyraAssetManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1UIData)

const UA1UIData& UA1UIData::Get()
{
	return ULyraAssetManager::Get().GetUIData();
}

UTexture2D* UA1UIData::GetEntryRarityTexture(EItemRarity ItemRarity) const
{
	const int32 ItemRarityIndex = (int32)ItemRarity;
	if (ItemRarityIndex < 0 || ItemRarityIndex >= (int32)EItemRarity::Count)
		return nullptr;

	return RarityInfoEntries[ItemRarityIndex].EntryTexture;
}

UTexture2D* UA1UIData::GetHoverRarityTexture(EItemRarity ItemRarity) const
{
	const int32 ItemRarityIndex = (int32)ItemRarity;
	if (ItemRarityIndex < 0 || ItemRarityIndex >= (int32)EItemRarity::Count)
		return nullptr;

	return RarityInfoEntries[ItemRarityIndex].HoverTexture;
}

FColor UA1UIData::GetRarityColor(EItemRarity ItemRarity) const
{
	const int32 ItemRarityIndex = (int32)ItemRarity;
	if (ItemRarityIndex < 0 || ItemRarityIndex >= (int32)EItemRarity::Count)
		return FColor::White;

	return RarityInfoEntries[ItemRarityIndex].Color;
}

const FA1UIInfo& UA1UIData::GetTagUIInfo(FGameplayTag Tag) const
{
	const FA1UIInfo* UIInfo = TagUIInfos.Find(Tag);
	if (UIInfo == nullptr)
	{
		static FA1UIInfo EmptyInfo;
		return EmptyInfo;
	}

	return *UIInfo;
}