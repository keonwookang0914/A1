#pragma once

#include "GameplayTagContainer.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "A1GameplayTagStack.generated.h"

struct FA1GameplayTagStackContainer;
struct FNetDeltaSerializeInfo;

USTRUCT(BlueprintType)
struct FA1GameplayTagStack : public FFastArraySerializerItem
{
	GENERATED_BODY()

public:
	FA1GameplayTagStack() { }

	FA1GameplayTagStack(FGameplayTag InTag, int32 InStackCount)
		: Tag(InTag)
		, StackCount(InStackCount) { }

public:
	const FGameplayTag& GetStackTag() const { return Tag; }
	int32 GetStackCount() const { return StackCount; }
	
	FString GetDebugString() const;
	
private:
	friend FA1GameplayTagStackContainer;

	UPROPERTY()
	FGameplayTag Tag;

	UPROPERTY()
	int32 StackCount = 0;
};

USTRUCT(BlueprintType)
struct FA1GameplayTagStackContainer : public FFastArraySerializer
{
	GENERATED_BODY()

public:
	FA1GameplayTagStackContainer() { }

public:
	void AddStack(FGameplayTag Tag, int32 StackCount);
	void RemoveStack(FGameplayTag Tag);

public:
	const TArray<FA1GameplayTagStack>& GetStacks() const { return Stacks; }
	
	int32 GetStackCount(FGameplayTag Tag) const { return TagToCountMap.FindRef(Tag); }
	bool ContainsTag(FGameplayTag Tag) const { return TagToCountMap.Contains(Tag); }
	
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FA1GameplayTagStack, FA1GameplayTagStackContainer>(Stacks, DeltaParms, *this);
	}

private:
	UPROPERTY()
	TArray<FA1GameplayTagStack> Stacks;

	UPROPERTY(NotReplicated)
	TMap<FGameplayTag, int32> TagToCountMap;
};

template<>
struct TStructOpsTypeTraits<FA1GameplayTagStackContainer> : public TStructOpsTypeTraitsBase2<FA1GameplayTagStackContainer>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};
