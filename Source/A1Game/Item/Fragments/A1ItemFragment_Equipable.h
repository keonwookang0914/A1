#pragma once

#include "A1Define.h"
#include "GameplayTags.h"
#include "Item/A1ItemTemplate.h"
#include "A1ItemFragment_Equipable.generated.h"

class ULyraAbilitySet;
class UD1ItemInstance;

USTRUCT(BlueprintType)
struct FRarityStat
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleDefaultsOnly)
	EItemRarity Rarity = EItemRarity::Poor;

	UPROPERTY(EditDefaultsOnly)
	int32 Value = 0;
};

USTRUCT(BlueprintType)
struct FRarityStatSet
{
	GENERATED_BODY()

public:
	FRarityStatSet();
	
public:
	UPROPERTY(EditDefaultsOnly, meta=(Categories="SetByCaller"))
	FGameplayTag StatTag;
	
	UPROPERTY(EditDefaultsOnly, EditFixedSize)
	TArray<FRarityStat> RarityStats;
};

USTRUCT(BlueprintType)
struct FRarityStatRange
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleDefaultsOnly)
	EItemRarity Rarity = EItemRarity::Poor;
	
	UPROPERTY(EditDefaultsOnly)
	int32 MinValue = 0;

	UPROPERTY(EditDefaultsOnly)
	int32 MaxValue = 0;
};

USTRUCT(BlueprintType)
struct FRarityStatRangeSet
{
	GENERATED_BODY()

public:
	FRarityStatRangeSet();
	
public:
	UPROPERTY(EditDefaultsOnly, meta=(Categories="SetByCaller"))
	FGameplayTag StatTag;
	
	UPROPERTY(EditDefaultsOnly, EditFixedSize)
	TArray<FRarityStatRange> RarityStatRanges;
};

UCLASS(Abstract, Const)
class UA1ItemFragment_Equipable : public UA1ItemFragment
{
	GENERATED_BODY()
	
public:
	UA1ItemFragment_Equipable(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
protected:
	void AddStatTagStack(UA1ItemInstance* ItemInstance, const TArray<FRarityStatSet>& RarityStatSets) const;
	void AddStatTagStack(UA1ItemInstance* ItemInstance, const TArray<FRarityStatRangeSet>& RarityStatRangeSets) const;
	
public:
	EEquipmentType EquipmentType = EEquipmentType::Count;
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<const ULyraAbilitySet> BaseAbilitySet;
};
