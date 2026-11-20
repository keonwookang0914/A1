#pragma once

#include "A1Define.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "A1InventoryManagerComponent.generated.h"

class UA1ItemTemplate;
class UA1ItemInstance;
struct FA1ItemRarityProbability;
class UA1EquipmentManagerComponent;
class UA1InventoryManagerComponent;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnInventoryEntryChanged, const FIntPoint&/*ItemSlotPos*/, UA1ItemInstance*, int32/*ItemCount*/);

/*-----------------------------
*      A1InventoryEntry
-----------------------------*/
USTRUCT(BlueprintType)
struct FA1InventoryEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

private:
	UA1ItemInstance* Init(int32 InItemTemplateID, int32 InItemCount, EItemRarity InItemRarity);
	void Init(UA1ItemInstance* InItemInstance, int32 InItemCount);
	UA1ItemInstance* Reset();
	
public:
	UA1ItemInstance* GetItemInstance() const { return ItemInstance; }
	int32 GetItemCount() const { return ItemCount; }
	
private:
	friend struct FA1InventoryList;
	friend class UA1InventoryManagerComponent;
	friend class UA1ItemManagerComponent;
	
	UPROPERTY()
	TObjectPtr<UA1ItemInstance> ItemInstance;

	UPROPERTY()
	int32 ItemCount = 0;
};


/*-----------------------------
*      A1InventoryList
-----------------------------*/
USTRUCT(BlueprintType)
struct FA1InventoryList : public FFastArraySerializer
{
	GENERATED_BODY()

public:
	FA1InventoryList() : InventoryManager(nullptr) { }
	FA1InventoryList(UA1InventoryManagerComponent* InOwnerComponent) : InventoryManager(InOwnerComponent) { }

public:
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);

private:
	void BroadcastChangedMessage(const FIntPoint& ItemSlotPos, UA1ItemInstance* ItemInstance, int32 ItemCount);
	
public:
	const TArray<FA1InventoryEntry>& GetAllEntries() const { return Entries; }
	
private:
	friend class UA1InventoryManagerComponent;
	friend class UA1EquipmentManagerComponent;
	friend class UA1ItemManagerComponent;
	
	UPROPERTY()
	TArray<FA1InventoryEntry> Entries;
	
	UPROPERTY(NotReplicated)
	TObjectPtr<UA1InventoryManagerComponent> InventoryManager;
};

template<>
struct TStructOpsTypeTraits<FA1InventoryList> : public TStructOpsTypeTraitsBase2<FA1InventoryList>
{
	enum
	{
		WithNetDeltaSerializer = true
	};
};


/*-----------------------------
*     A1InventoryManager
-----------------------------*/
UCLASS(BlueprintType, Blueprintable)
class UA1InventoryManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UA1InventoryManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void InitializeComponent() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	virtual void ReadyForReplication() override;
	
public:
	int32 CanMoveOrMergeItem(UA1InventoryManagerComponent* OtherComponent, const FIntPoint& FromItemSlotPos, const FIntPoint& ToItemSlotPos) const;
	int32 CanMoveOrMergeItem(UA1EquipmentManagerComponent* OtherComponent, EEquipmentSlotType FromEquipmentSlotType, const FIntPoint& ToItemSlotPos) const;

	int32 CanMoveOrMergeItem_Quick(UA1InventoryManagerComponent* OtherComponent, const FIntPoint& FromItemSlotPos, TArray<FIntPoint>& OutToItemSlotPoses, TArray<int32>& OutToItemCounts) const;
	int32 CanMoveOrMergeItem_Quick(UA1EquipmentManagerComponent* OtherComponent, EEquipmentSlotType FromEquipmentSlotType, TArray<FIntPoint>& OutToItemSlotPoses, TArray<int32>& OutToItemCounts) const;
	
	int32 CanAddItem(int32 ItemTemplateID, EItemRarity ItemRarity, int32 ItemCount, TArray<FIntPoint>& OutToItemSlotPoses, TArray<int32>& OutToItemCounts) const;
	bool CanRemoveItem(int32 ItemTemplateID, int32 ItemCount, TArray<FIntPoint>& OutToItemSlotPoses, TArray<int32>& OutToItemCounts) const;

public:
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	int32 TryAddItemByRarity(TSubclassOf<UA1ItemTemplate> ItemTemplateClass, EItemRarity ItemRarity, int32 ItemCount);
	
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	int32 TryAddItemByProbability(TSubclassOf<UA1ItemTemplate> ItemTemplateClass, int32 ItemCount, const TArray<FA1ItemRarityProbability>& ItemProbabilities);
	
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	bool TryRemoveItem(int32 ItemTemplateID, int32 ItemCount);

private:
	void AddItem_Unsafe(const FIntPoint& ItemSlotPos, UA1ItemInstance* ItemInstance, int32 ItemCount);
	UA1ItemInstance* RemoveItem_Unsafe(const FIntPoint& ItemSlotPos, int32 ItemCount);
	
private:
	void MarkSlotChecks(TArray<bool>& InSlotChecks, bool bIsUsing, const FIntPoint& ItemSlotPos, const FIntPoint& ItemSlotCount) const;
	void MarkSlotChecks(bool bIsUsing, const FIntPoint& ItemSlotPos, const FIntPoint& ItemSlotCount);

public:
	bool IsEmpty(const TArray<bool>& InSlotChecks, const FIntPoint& ItemSlotPos, const FIntPoint& ItemSlotCount) const;
	bool IsEmpty(const FIntPoint& ItemSlotPos, const FIntPoint& ItemSlotCount) const;
	bool IsAllEmpty();
	
	UA1ItemInstance* GetItemInstance(const FIntPoint& ItemSlotPos) const;
	int32 GetItemCount(const FIntPoint& ItemSlotPos) const;
	
	const TArray<FA1InventoryEntry>& GetAllEntries() const;
	int32 GetTotalCountByID(int32 ItemTemplateID) const;
	FIntPoint GetInventorySlotCount() const { return InventorySlotCount; }
	TArray<bool>& GetSlotChecks() { return SlotChecks; }

public:
	FOnInventoryEntryChanged OnInventoryEntryChanged;
	FIntPoint ClickedIndex = NULL;
	
private:
	friend class UA1EquipmentManagerComponent;
	friend class UA1ItemManagerComponent;
	
	UPROPERTY(Replicated)
	FA1InventoryList InventoryList;
	
	UPROPERTY(Replicated)
	TArray<bool> SlotChecks;
	
	FIntPoint InventorySlotCount = FIntPoint(5, 5);
};
