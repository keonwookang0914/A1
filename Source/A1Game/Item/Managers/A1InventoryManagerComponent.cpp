#include "A1InventoryManagerComponent.h"

#include "A1EquipmentManagerComponent.h"
#include "Data/A1ItemData.h"
#include "Engine/ActorChannel.h"
#include "Item/A1ItemInstance.h"
#include "Item/A1ItemTemplate.h"
#include "Net/UnrealNetwork.h"
#include "Score/A1ScoreBlueprintFunctionLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1InventoryManagerComponent)

/*-----------------------------
*      A1InventoryEntry
-----------------------------*/
UA1ItemInstance* FA1InventoryEntry::Init(int32 InItemTemplateID, int32 InItemCount, EItemRarity InItemRarity)
{
	check(InItemTemplateID > 0 && InItemCount > 0 && InItemRarity != EItemRarity::Count);
	
	UA1ItemInstance* NewItemInstance = NewObject<UA1ItemInstance>();
	NewItemInstance->Init(InItemTemplateID, InItemRarity);
	Init(NewItemInstance, InItemCount);
	
	return NewItemInstance;
}

void FA1InventoryEntry::Init(UA1ItemInstance* InItemInstance, int32 InItemCount)
{
	check(InItemInstance && InItemCount > 0);
	
	ItemInstance = InItemInstance;
	
	const UA1ItemTemplate& ItemTemplate = UA1ItemData::Get().FindItemTemplateByID(ItemInstance->GetItemTemplateID());
	ItemCount = FMath::Clamp(InItemCount, 1, ItemTemplate.MaxStackCount);
}

UA1ItemInstance* FA1InventoryEntry::Reset()
{
	UA1ItemInstance* RemovedItemInstance = ItemInstance;
	ItemInstance = nullptr;
	ItemCount = 0;
	
	return RemovedItemInstance;
}

/*-----------------------------
*      A1InventoryList
-----------------------------*/
bool FA1InventoryList::NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
{
	return FFastArraySerializer::FastArrayDeltaSerialize<FA1InventoryEntry, FA1InventoryList>(Entries, DeltaParams, *this);
}

void FA1InventoryList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	const FIntPoint& InventorySlotCount = InventoryManager->GetInventorySlotCount();

	for (int32 AddedIndex : AddedIndices)
	{
		FA1InventoryEntry& Entry = Entries[AddedIndex];
		if (Entry.ItemInstance)
		{
			const FIntPoint ItemSlotPos = FIntPoint(AddedIndex % InventorySlotCount.X, AddedIndex / InventorySlotCount.X);
			BroadcastChangedMessage(ItemSlotPos, Entry.ItemInstance, Entry.ItemCount);
		}
	}
}

void FA1InventoryList::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	TArray<int32> AddedIndices;
	AddedIndices.Reserve(FinalSize);

	const FIntPoint& InventorySlotCount = InventoryManager->GetInventorySlotCount();
	
	for (int32 ChangedIndex : ChangedIndices)
	{
		FA1InventoryEntry& Entry = Entries[ChangedIndex];
		if (Entry.ItemInstance)
		{
			AddedIndices.Add(ChangedIndex);
		}
		else
		{
			const FIntPoint ItemSlotPos = FIntPoint(ChangedIndex % InventorySlotCount.X, ChangedIndex / InventorySlotCount.X);
			BroadcastChangedMessage(ItemSlotPos, nullptr, 0);
		}
	}

	for (int32 AddedIndex : AddedIndices)
	{
		FA1InventoryEntry& Entry = Entries[AddedIndex];
		const FIntPoint ItemSlotPos = FIntPoint(AddedIndex % InventorySlotCount.X, AddedIndex / InventorySlotCount.X);
		BroadcastChangedMessage(ItemSlotPos, Entry.ItemInstance, Entry.ItemCount);
	}
}

void FA1InventoryList::BroadcastChangedMessage(const FIntPoint& ItemSlotPos, UA1ItemInstance* ItemInstance, int32 ItemCount)
{
	if (InventoryManager->OnInventoryEntryChanged.IsBound())
	{
		InventoryManager->OnInventoryEntryChanged.Broadcast(ItemSlotPos, ItemInstance, ItemCount);
	}
}

/*-----------------------------
*      A1InventoryManager
-----------------------------*/
UA1InventoryManagerComponent::UA1InventoryManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, InventoryList(this)
{
	bWantsInitializeComponent = true;
	SetIsReplicatedByDefault(true);
}

void UA1InventoryManagerComponent::InitializeComponent()
{
	Super::InitializeComponent();

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		TArray<FA1InventoryEntry>& Entries = InventoryList.Entries;
		Entries.SetNum(InventorySlotCount.X * InventorySlotCount.Y);
	
		for (FA1InventoryEntry& Entry : Entries)
		{
			InventoryList.MarkItemDirty(Entry);
		}

		SlotChecks.SetNumZeroed(InventorySlotCount.X * InventorySlotCount.Y);
	}
}

void UA1InventoryManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, InventoryList);
	DOREPLIFETIME(ThisClass, SlotChecks);
}

bool UA1InventoryManagerComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bWroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	
	for (FA1InventoryEntry& Entry : InventoryList.Entries)
	{
		UA1ItemInstance* ItemInstance = Entry.ItemInstance;
		if (IsValid(ItemInstance))
		{
			bWroteSomething |= Channel->ReplicateSubobject(ItemInstance, *Bunch, *RepFlags);
		}
	}
	
	return bWroteSomething;
}

void UA1InventoryManagerComponent::ReadyForReplication()
{
	Super::ReadyForReplication();
	
	if (IsUsingRegisteredSubObjectList())
	{
		for (const FA1InventoryEntry& Entry : InventoryList.Entries)
		{
			UA1ItemInstance* ItemInstance = Entry.GetItemInstance();
			if (IsValid(ItemInstance))
			{
				AddReplicatedSubObject(ItemInstance);
			}
		}
	}
}

int32 UA1InventoryManagerComponent::CanMoveOrMergeItem(UA1InventoryManagerComponent* OtherComponent, const FIntPoint& FromItemSlotPos, const FIntPoint& ToItemSlotPos) const
{
	if (OtherComponent == nullptr)
		return 0;

	const FIntPoint& FromInventorySlotCount = OtherComponent->GetInventorySlotCount();
	if (FromItemSlotPos.X < 0 || FromItemSlotPos.Y < 0 || FromItemSlotPos.X >= FromInventorySlotCount.X || FromItemSlotPos.Y >= FromInventorySlotCount.Y)
		return 0;
	
	if (ToItemSlotPos.X < 0 || ToItemSlotPos.Y < 0 || ToItemSlotPos.X >= InventorySlotCount.X || ToItemSlotPos.Y >= InventorySlotCount.Y)
		return 0;
	
	const UA1ItemInstance* FromItemInstance = OtherComponent->GetItemInstance(FromItemSlotPos);
	const int32 FromItemCount = OtherComponent->GetItemCount(FromItemSlotPos);
	
	if (this == OtherComponent && FromItemSlotPos == ToItemSlotPos)
		return FromItemCount;
	
	if (FromItemInstance == nullptr || FromItemCount <= 0)
		return 0;
	
	const UA1ItemInstance* ToItemInstance = GetItemInstance(ToItemSlotPos);
	const int32 ToItemCount = GetItemCount(ToItemSlotPos);
	
	const int32 FromTemplateID = FromItemInstance->GetItemTemplateID();
	const UA1ItemTemplate& FromItemTemplate = UA1ItemData::Get().FindItemTemplateByID(FromTemplateID);
	
	if (ToItemInstance)
	{
		const int32 ToTemplateID = ToItemInstance->GetItemTemplateID();
		if (FromTemplateID != ToTemplateID)
			return 0;

		if (FromItemInstance->GetItemRarity() != ToItemInstance->GetItemRarity())
			return 0;
		
		if (FromItemTemplate.MaxStackCount < 2)
			return 0;

		return FMath::Min(FromItemCount + ToItemCount, FromItemTemplate.MaxStackCount) - ToItemCount;
	}
	else
	{
		const FIntPoint& FromItemSlotCount = FromItemTemplate.SlotCount;
		if (ToItemSlotPos.X + FromItemSlotCount.X > InventorySlotCount.X || ToItemSlotPos.Y + FromItemSlotCount.Y > InventorySlotCount.Y)
			return 0;

		if (this == OtherComponent)
		{
			TArray<bool> TempSlotChecks = SlotChecks;
			MarkSlotChecks(TempSlotChecks, false, FromItemSlotPos, FromItemSlotCount);
			
			return IsEmpty(TempSlotChecks, ToItemSlotPos, FromItemSlotCount) ? FromItemCount : 0;
		}
		else
		{
			return IsEmpty(ToItemSlotPos, FromItemSlotCount) ? FromItemCount : 0;
		}
	}
}

int32 UA1InventoryManagerComponent::CanMoveOrMergeItem(UA1EquipmentManagerComponent* OtherComponent, EEquipmentSlotType FromEquipmentSlotType, const FIntPoint& ToItemSlotPos) const
{
	if (OtherComponent == nullptr)
		return 0;

	if (FromEquipmentSlotType == EEquipmentSlotType::Count)
		return 0;
	
	if (ToItemSlotPos.X < 0 || ToItemSlotPos.Y < 0 || ToItemSlotPos.X >= InventorySlotCount.X || ToItemSlotPos.Y >= InventorySlotCount.Y)
		return 0;
	
	const UA1ItemInstance* FromItemInstance = OtherComponent->GetItemInstance(FromEquipmentSlotType);
	const int32 FromItemCount = OtherComponent->GetItemCount(FromEquipmentSlotType);

	if (FromItemInstance == nullptr || FromItemCount <= 0)
		return 0;
	
	const UA1ItemInstance* ToItemInstance = GetItemInstance(ToItemSlotPos);
	const int32 ToItemCount = GetItemCount(ToItemSlotPos);
	
	const int32 FromTemplateID = FromItemInstance->GetItemTemplateID();
	const UA1ItemTemplate& FromItemTemplate = UA1ItemData::Get().FindItemTemplateByID(FromTemplateID);
	
	if (ToItemInstance)
	{
		const int32 ToTemplateID = ToItemInstance->GetItemTemplateID();
		if (FromTemplateID != ToTemplateID)
			return 0;

		if (FromItemInstance->GetItemRarity() != ToItemInstance->GetItemRarity())
			return 0;
		
		if (FromItemTemplate.MaxStackCount < 2)
			return 0;

		return FMath::Min(FromItemCount + ToItemCount, FromItemTemplate.MaxStackCount) - ToItemCount;
	}
	else
	{
		const FIntPoint& FromItemSlotCount = FromItemTemplate.SlotCount;
		if (ToItemSlotPos.X + FromItemSlotCount.X > InventorySlotCount.X || ToItemSlotPos.Y + FromItemSlotCount.Y > InventorySlotCount.Y)
			return 0;

		return IsEmpty(ToItemSlotPos, FromItemSlotCount) ? FromItemCount : 0;
	}
}

int32 UA1InventoryManagerComponent::CanMoveOrMergeItem_Quick(UA1InventoryManagerComponent* OtherComponent, const FIntPoint& FromItemSlotPos, TArray<FIntPoint>& OutToItemSlotPoses, TArray<int32>& OutToItemCounts) const
{
	OutToItemSlotPoses.Reset();
	OutToItemCounts.Reset();
	
	if (OtherComponent == nullptr || this == OtherComponent)
		return 0;

	const FIntPoint& FromInventorySlotCount = OtherComponent->GetInventorySlotCount();
	if (FromItemSlotPos.X < 0 || FromItemSlotPos.Y < 0 || FromItemSlotPos.X >= FromInventorySlotCount.X || FromItemSlotPos.Y >= FromInventorySlotCount.Y)
		return 0;
	
	const UA1ItemInstance* FromItemInstance = OtherComponent->GetItemInstance(FromItemSlotPos);
	const int32 FromItemCount = OtherComponent->GetItemCount(FromItemSlotPos);

	if (FromItemInstance == nullptr)
		return 0;
	
	return CanAddItem(FromItemInstance->GetItemTemplateID(), FromItemInstance->GetItemRarity(), FromItemCount, OutToItemSlotPoses, OutToItemCounts);
}

int32 UA1InventoryManagerComponent::CanMoveOrMergeItem_Quick(UA1EquipmentManagerComponent* OtherComponent, EEquipmentSlotType FromEquipmentSlotType, TArray<FIntPoint>& OutToItemSlotPoses, TArray<int32>& OutToItemCounts) const
{
	OutToItemSlotPoses.Reset();
	OutToItemCounts.Reset();
	
	if (OtherComponent == nullptr)
		return 0;

	if (FromEquipmentSlotType == EEquipmentSlotType::Count)
		return 0;
	
	const UA1ItemInstance* FromItemInstance = OtherComponent->GetItemInstance(FromEquipmentSlotType);
	const int32 FromItemCount = OtherComponent->GetItemCount(FromEquipmentSlotType);

	if (FromItemInstance == nullptr)
		return 0;

	return CanAddItem(FromItemInstance->GetItemTemplateID(), FromItemInstance->GetItemRarity(), FromItemCount, OutToItemSlotPoses, OutToItemCounts);
}

int32 UA1InventoryManagerComponent::CanAddItem(int32 ItemTemplateID, EItemRarity ItemRarity, int32 ItemCount, TArray<FIntPoint>& OutToItemSlotPoses, TArray<int32>& OutToItemCounts) const
{
	OutToItemSlotPoses.Reset();
	OutToItemCounts.Reset();
	
	if (ItemTemplateID <= 0 || ItemRarity == EItemRarity::Count || ItemCount <= 0)
		return 0;
	
	const UA1ItemTemplate& ItemTemplate = UA1ItemData::Get().FindItemTemplateByID(ItemTemplateID);
	int32 LeftItemCount = ItemCount;
	
	if (ItemTemplate.MaxStackCount > 1)
	{
		const TArray<FA1InventoryEntry>& ToEntries = GetAllEntries();
		
		for (int32 i = 0; i < ToEntries.Num(); i++)
		{
			const FA1InventoryEntry& ToEntry = ToEntries[i];
			const UA1ItemInstance* ToItemInstance = ToEntry.GetItemInstance();
			const int32 ToItemCount = ToEntry.GetItemCount();
			
			if (ToItemInstance == nullptr)
				continue;

			if (ToItemInstance->GetItemTemplateID() != ItemTemplateID)
				continue;

			if (ToItemInstance->GetItemRarity() != ItemRarity)
				continue;
			
			if (int32 AddCount = FMath::Min(ToItemCount + LeftItemCount, ItemTemplate.MaxStackCount) - ToItemCount)
			{
				OutToItemSlotPoses.Emplace(i % InventorySlotCount.X, i / InventorySlotCount.X);
				OutToItemCounts.Emplace(AddCount);
				LeftItemCount -= AddCount;

				if (LeftItemCount == 0)
					return ItemCount;
			}
		}
	}
	
	const FIntPoint& ItemSlotCount = ItemTemplate.SlotCount;
	TArray<bool> TempSlotChecks = SlotChecks;
	
	const FIntPoint StartSlotPos = FIntPoint::ZeroValue;
	const FIntPoint EndSlotPos = InventorySlotCount - ItemSlotCount;
	
	for (int32 y = StartSlotPos.Y; y <= EndSlotPos.Y; y++)
	{
		for (int32 x = StartSlotPos.X; x <= EndSlotPos.X; x++)
		{
			int32 Index = y * InventorySlotCount.X + x;
			if (TempSlotChecks.IsValidIndex(Index) == false || TempSlotChecks[Index])
				continue;

			FIntPoint ItemSlotPos = FIntPoint(x, y);
			if (IsEmpty(TempSlotChecks, ItemSlotPos, ItemSlotCount))
			{
				MarkSlotChecks(TempSlotChecks, true, ItemSlotPos, ItemSlotCount);
				
				int32 AddCount = FMath::Min(LeftItemCount, ItemTemplate.MaxStackCount);
				OutToItemSlotPoses.Emplace(ItemSlotPos);
				OutToItemCounts.Emplace(AddCount);
				
				LeftItemCount -= AddCount;
				
				if (LeftItemCount == 0)
					return ItemCount;
			}
		}
	}
	
	return ItemCount - LeftItemCount;
}

bool UA1InventoryManagerComponent::CanRemoveItem(int32 ItemTemplateID, int32 ItemCount, TArray<FIntPoint>& OutToItemSlotPoses, TArray<int32>& OutToItemCounts) const
{
	OutToItemSlotPoses.Reset();
	OutToItemCounts.Reset();

	if (ItemTemplateID <= 0 || ItemCount <= 0)
		return false;
	
	TArray<FIntPoint> ToItemSlotPoses;
	TArray<int32> ToItemCounts;

	const TArray<FA1InventoryEntry>& Entries = GetAllEntries();
	
	for (int32 i = Entries.Num() - 1; i >= 0; i--)
	{
		const FA1InventoryEntry& ToEntry = Entries[i];
		if (ToEntry.ItemInstance == nullptr)
			continue;

		if (ToEntry.ItemInstance->GetItemTemplateID() != ItemTemplateID)
			continue;
		
		if (ToEntry.ItemCount < ItemCount)
		{
			ToItemSlotPoses.Emplace(i % InventorySlotCount.X, i / InventorySlotCount.X);
			ToItemCounts.Emplace(ToEntry.ItemCount);
			
			ItemCount -= ToEntry.ItemCount;
		}
		else
		{
			ToItemSlotPoses.Emplace(i % InventorySlotCount.X, i / InventorySlotCount.X);
			ToItemCounts.Emplace(ItemCount);

			OutToItemSlotPoses = ToItemSlotPoses;
			OutToItemCounts = ToItemCounts;
			return true;
		}
	}
	
	return false;
}

int32 UA1InventoryManagerComponent::TryAddItemByRarity(TSubclassOf<UA1ItemTemplate> ItemTemplateClass, EItemRarity ItemRarity, int32 ItemCount)
{
	check(GetOwner()->HasAuthority());

	if (ItemTemplateClass == nullptr || ItemRarity == EItemRarity::Count || ItemCount <= 0)
		return 0;
	
	int32 ItemTemplateID = UA1ItemData::Get().FindItemTemplateIDByClass(ItemTemplateClass);
	const UA1ItemTemplate& ItemTemplate = UA1ItemData::Get().FindItemTemplateByID(ItemTemplateID);
	
	TArray<FIntPoint> ToItemSlotPoses;
	TArray<int32> ToItemCounts;

	int32 AddableItemCount = CanAddItem(ItemTemplateID, ItemRarity, ItemCount, ToItemSlotPoses, ToItemCounts);
	if (AddableItemCount > 0)
	{
		TArray<UA1ItemInstance*> AddedItemInstances;
		
		for (int32 i = 0; i < ToItemSlotPoses.Num(); i++)
		{
			const FIntPoint& ToItemSlotPos = ToItemSlotPoses[i];
			const int32 ToItemCount = ToItemCounts[i];

			const int32 ToIndex = ToItemSlotPos.Y * InventorySlotCount.X + ToItemSlotPos.X;
			FA1InventoryEntry& ToEntry = InventoryList.Entries[ToIndex];

			if (ToEntry.ItemInstance)
			{
				ToEntry.ItemCount += ToItemCount;
				InventoryList.MarkItemDirty(ToEntry);
			}
			else
			{
				AddedItemInstances.Add(ToEntry.Init(ItemTemplateID, ToItemCount, ItemRarity));
				MarkSlotChecks(true, ToItemSlotPos, ItemTemplate.SlotCount);
				InventoryList.MarkItemDirty(ToEntry);
			}
		}

		if (IsUsingRegisteredSubObjectList() && IsReadyForReplication())
		{
			for (UA1ItemInstance* AddedItemInstance : AddedItemInstances)
			{
				if (AddedItemInstance)
				{
					AddReplicatedSubObject(AddedItemInstance);
				}
			}
		}
		return AddableItemCount;
	}

	return 0;
}

int32 UA1InventoryManagerComponent::TryAddItemByProbability(TSubclassOf<UA1ItemTemplate> ItemTemplateClass, int32 ItemCount, const TArray<FA1ItemRarityProbability>& ItemProbabilities)
{
	check(GetOwner()->HasAuthority());

	return TryAddItemByRarity(ItemTemplateClass, UA1ItemInstance::DetermineItemRarity(ItemProbabilities), ItemCount);
}

bool UA1InventoryManagerComponent::TryRemoveItem(int32 ItemTemplateID, int32 ItemCount)
{
	check(GetOwner()->HasAuthority());

	if (ItemTemplateID <= 0 || ItemCount <= 0)
		return false;
	
	const UA1ItemTemplate& ItemTemplate = UA1ItemData::Get().FindItemTemplateByID(ItemTemplateID);

	TArray<FIntPoint> ToItemSlotPoses;
	TArray<int32> ToItemCounts;
	
	if (CanRemoveItem(ItemTemplateID, ItemCount, ToItemSlotPoses, ToItemCounts))
	{
		TArray<UA1ItemInstance*> RemovedItemInstances;

		for (int32 i = 0; i < ToItemSlotPoses.Num(); i++)
		{
			const FIntPoint& ToItemSlotPos = ToItemSlotPoses[i];
			const int32 ToItemCount = ToItemCounts[i];

			const int32 ToIndex = ToItemSlotPos.Y * InventorySlotCount.X + ToItemSlotPos.X;
			FA1InventoryEntry& ToEntry = InventoryList.Entries[ToIndex];

			ToEntry.ItemCount -= ToItemCount;
			
			if (ToEntry.ItemCount <= 0)
			{
				MarkSlotChecks(false, ToItemSlotPos, ItemTemplate.SlotCount);
				RemovedItemInstances.Add(ToEntry.Reset());
			}

			InventoryList.MarkItemDirty(ToEntry);
		}

		if (IsUsingRegisteredSubObjectList())
		{
			for (UA1ItemInstance* RemovedItemInstance : RemovedItemInstances)
			{
				if (RemovedItemInstance)
				{
					RemoveReplicatedSubObject(RemovedItemInstance);
				}
			}
		}
		return true;
	}

	return false;
}

void UA1InventoryManagerComponent::AddItem_Unsafe(const FIntPoint& ItemSlotPos, UA1ItemInstance* ItemInstance, int32 ItemCount)
{
	check(GetOwner()->HasAuthority());
	
	const int32 Index = ItemSlotPos.Y * InventorySlotCount.X + ItemSlotPos.X;
	FA1InventoryEntry& Entry = InventoryList.Entries[Index];
	
	if (Entry.GetItemInstance())
	{
		Entry.ItemCount += ItemCount;
		InventoryList.MarkItemDirty(Entry);
	}
	else
	{
		if (ItemInstance == nullptr)
			return;
		
		const UA1ItemTemplate& ItemTemplate = UA1ItemData::Get().FindItemTemplateByID(ItemInstance->GetItemTemplateID());
		
		Entry.Init(ItemInstance, ItemCount);
		
		if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && ItemInstance)
		{
			AddReplicatedSubObject(ItemInstance);
		}

		MarkSlotChecks(true, ItemSlotPos, ItemTemplate.SlotCount);
		InventoryList.MarkItemDirty(Entry);

		//TEMP
		InventoryList.BroadcastChangedMessage(ItemSlotPos, ItemInstance, ItemCount);
	}
	UA1ScoreBlueprintFunctionLibrary::AddInventoryItems();
}

UA1ItemInstance* UA1InventoryManagerComponent::RemoveItem_Unsafe(const FIntPoint& ItemSlotPos, int32 ItemCount)
{
	check(GetOwner()->HasAuthority());
	
	const int32 Index = ItemSlotPos.Y * InventorySlotCount.X + ItemSlotPos.X;
	FA1InventoryEntry& Entry = InventoryList.Entries[Index];
	UA1ItemInstance* ItemInstance = Entry.GetItemInstance();
	
	Entry.ItemCount -= ItemCount;
	if (Entry.GetItemCount() <= 0)
	{
		const UA1ItemTemplate& ItemTemplate = UA1ItemData::Get().FindItemTemplateByID(ItemInstance->GetItemTemplateID());
		MarkSlotChecks(false, ItemSlotPos, ItemTemplate.SlotCount);
		
		UA1ItemInstance* RemovedItemInstance = Entry.Reset();
		if (IsUsingRegisteredSubObjectList() && RemovedItemInstance)
		{
			RemoveReplicatedSubObject(RemovedItemInstance);
		}
	}

	UA1ScoreBlueprintFunctionLibrary::AddInventoryItems(-1);

	InventoryList.MarkItemDirty(Entry);

	//TEMP
	InventoryList.BroadcastChangedMessage(ItemSlotPos, nullptr, Entry.GetItemCount());
	return ItemInstance;
}

void UA1InventoryManagerComponent::MarkSlotChecks(TArray<bool>& InSlotChecks, bool bIsUsing, const FIntPoint& ItemSlotPos, const FIntPoint& ItemSlotCount) const
{
	if (ItemSlotPos.X < 0 || ItemSlotPos.Y < 0)
		return;

	if (ItemSlotPos.X + ItemSlotCount.X > InventorySlotCount.X || ItemSlotPos.Y + ItemSlotCount.Y > InventorySlotCount.Y)
		return;

	const FIntPoint StartSlotPos = ItemSlotPos;
	const FIntPoint EndSlotPos = ItemSlotPos + ItemSlotCount;

	for (int32 y = StartSlotPos.Y; y < EndSlotPos.Y; y++)
	{
		for (int32 x = StartSlotPos.X; x < EndSlotPos.X; x++)
		{
			int32 Index = y * InventorySlotCount.X + x;
			if (InSlotChecks.IsValidIndex(Index))
			{
				InSlotChecks[Index] = bIsUsing;
			}
		}
	}
}

void UA1InventoryManagerComponent::MarkSlotChecks(bool bIsUsing, const FIntPoint& ItemSlotPos, const FIntPoint& ItemSlotCount)
{
	MarkSlotChecks(SlotChecks, bIsUsing, ItemSlotPos, ItemSlotCount);
}

bool UA1InventoryManagerComponent::IsEmpty(const TArray<bool>& InSlotChecks, const FIntPoint& ItemSlotPos, const FIntPoint& ItemSlotCount) const
{
	if (ItemSlotPos.X < 0 || ItemSlotPos.Y < 0)
		return false;

	if (ItemSlotPos.X + ItemSlotCount.X > InventorySlotCount.X || ItemSlotPos.Y + ItemSlotCount.Y > InventorySlotCount.Y)
		return false;

	const FIntPoint StartSlotPos = ItemSlotPos;
	const FIntPoint EndSlotPos = ItemSlotPos + ItemSlotCount;

	for (int32 y = StartSlotPos.Y; y < EndSlotPos.Y; y++)
	{
		for (int32 x = StartSlotPos.X; x < EndSlotPos.X; x++)
		{
			int32 Index = y * InventorySlotCount.X + x;
			if (InSlotChecks.IsValidIndex(Index) == false || InSlotChecks[Index])
				return false;
		}
	}
	return true;
}

bool UA1InventoryManagerComponent::IsEmpty(const FIntPoint& ItemSlotPos, const FIntPoint& ItemSlotCount) const
{
	return IsEmpty(SlotChecks, ItemSlotPos, ItemSlotCount);
}

bool UA1InventoryManagerComponent::IsAllEmpty()
{
	for (FA1InventoryEntry& Entry : InventoryList.Entries)
	{
		if (Entry.GetItemInstance())
			return false;
	}
	return true;
}

UA1ItemInstance* UA1InventoryManagerComponent::GetItemInstance(const FIntPoint& ItemSlotPos) const
{
	if (ItemSlotPos.X < 0 || ItemSlotPos.Y < 0 || ItemSlotPos.X >= InventorySlotCount.X || ItemSlotPos.Y >= InventorySlotCount.Y)
		return nullptr;
	
	const TArray<FA1InventoryEntry>& Entries = InventoryList.GetAllEntries();
	const int32 EntryIndex = ItemSlotPos.Y * InventorySlotCount.X + ItemSlotPos.X;
	const FA1InventoryEntry& Entry = Entries[EntryIndex];
	
	return Entry.GetItemInstance();
}

int32 UA1InventoryManagerComponent::GetItemCount(const FIntPoint& ItemSlotPos) const
{
	if (ItemSlotPos.X < 0 || ItemSlotPos.Y < 0 || ItemSlotPos.X >= InventorySlotCount.X || ItemSlotPos.Y >= InventorySlotCount.Y)
		return 0;
	
	const TArray<FA1InventoryEntry>& Entries = InventoryList.GetAllEntries();
	const int32 EntryIndex = ItemSlotPos.Y * InventorySlotCount.X + ItemSlotPos.X;
	const FA1InventoryEntry& Entry = Entries[EntryIndex];
	
	return Entry.GetItemCount();
}

const TArray<FA1InventoryEntry>& UA1InventoryManagerComponent::GetAllEntries() const
{
	return InventoryList.GetAllEntries();
}

int32 UA1InventoryManagerComponent::GetTotalCountByID(int32 ItemTemplateID) const
{
	int32 TotalCount = 0;
	
	for (const FA1InventoryEntry& Entry : GetAllEntries())
	{
		if (UA1ItemInstance* ItemInstance = Entry.ItemInstance)
		{
			if (ItemInstance->GetItemTemplateID() == ItemTemplateID)
			{
				TotalCount += Entry.ItemCount;
			}
		}
	}
	
	return TotalCount;
}
