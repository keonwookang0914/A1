#include "A1ItemFragment_Equipable.h"

#include "Item/A1ItemInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1ItemFragment_Equipable)

FRarityStatSet::FRarityStatSet()
{
	RarityStats.SetNum((int32)EItemRarity::Count);
	for (int32 i = 0; i < RarityStats.Num(); i++)
	{
		RarityStats[i].Rarity = (EItemRarity)i;
	}
}

FRarityStatRangeSet::FRarityStatRangeSet()
{
	RarityStatRanges.SetNum((int32)EItemRarity::Count);
	for (int32 i = 0; i < RarityStatRanges.Num(); i++)
	{
		RarityStatRanges[i].Rarity = (EItemRarity)i;
	}
}

UA1ItemFragment_Equipable::UA1ItemFragment_Equipable(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
    
}

void UA1ItemFragment_Equipable::AddStatTagStack(UA1ItemInstance* ItemInstance, const TArray<FRarityStatSet>& RarityStatSets) const
{
	if (ItemInstance == nullptr)
		return;

	for (const FRarityStatSet& RarityStatSet : RarityStatSets)
	{
		const FGameplayTag& StatTag = RarityStatSet.StatTag;
		const FRarityStat& Stat = RarityStatSet.RarityStats[(int32)ItemInstance->GetItemRarity()];
		ItemInstance->AddStatTagStack(StatTag, Stat.Value);
	}
}

void UA1ItemFragment_Equipable::AddStatTagStack(UA1ItemInstance* ItemInstance, const TArray<FRarityStatRangeSet>& RarityStatRangeSets) const
{
	if (ItemInstance == nullptr)
		return;
	
	for (const FRarityStatRangeSet& RarityStatRangeSet : RarityStatRangeSets)
	{
		const FGameplayTag& StatTag = RarityStatRangeSet.StatTag;
		const FRarityStatRange& StatRange = RarityStatRangeSet.RarityStatRanges[(int32)ItemInstance->GetItemRarity()];
		const int32 StatValue = FMath::RandRange(StatRange.MinValue, StatRange.MaxValue);
		ItemInstance->AddStatTagStack(StatTag, StatValue);
	}
}
