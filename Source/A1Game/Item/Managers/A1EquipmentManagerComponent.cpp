#include "A1EquipmentManagerComponent.h"

#include "A1EquipManagerComponent.h"
#include "A1InventoryManagerComponent.h"
#include "A1ItemManagerComponent.h"
#include "Character/LyraCharacter.h"
#include "Data/A1ItemData.h"
#include "Engine/ActorChannel.h"
#include "Item/A1ItemInstance.h"
#include "Item/Fragments/A1ItemFragment_Equipable_Armor.h"
#include "Item/Fragments/A1ItemFragment_Equipable_Weapon.h"
#include "Net/UnrealNetwork.h"
#include "Player/LyraPlayerController.h"
#include "Player/LyraPlayerState.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1EquipmentManagerComponent)

void FA1EquipmentEntry::Init(UA1ItemInstance* InItemInstance, int32 InItemCount)
{
	check(InItemInstance && InItemCount > 0);

	UA1EquipManagerComponent* EquipManager = EquipmentManager->GetEquipManager();
	if (EquipManager == nullptr)
		return;

	const UA1ItemFragment_Equipable* EquippableFragment = InItemInstance->FindFragmentByClass<UA1ItemFragment_Equipable>();
	if (EquippableFragment == nullptr)
		return;
	
	if (ItemInstance)
	{
		EquipManager->Unequip(EquipmentSlotType);
	}
	
	ItemInstance = InItemInstance;
	
	const UA1ItemTemplate& ItemTemplate = UA1ItemData::Get().FindItemTemplateByID(ItemInstance->GetItemTemplateID());
	ItemCount = FMath::Clamp(InItemCount, 1, ItemTemplate.MaxStackCount);

	//TEMP Jerry
	EquipmentManager->OnEquipmentEntryChanged.Broadcast(EquipmentSlotType, ItemInstance, ItemCount);

	EquipManager->Equip(EquipmentSlotType, ItemInstance);
	EquipManager->ChangeEquipState(EquipmentSlotType, true);
}

UA1ItemInstance* FA1EquipmentEntry::Reset()
{
	UA1EquipManagerComponent* EquipManager = EquipmentManager->GetEquipManager();
	if (EquipManager == nullptr)
		return nullptr;

	if (ItemInstance)
	{
		EquipManager->Unequip(EquipmentSlotType);
	}
	
	UA1ItemInstance* RemovedItemInstance = ItemInstance;
	ItemInstance = nullptr;
	ItemCount = 0;

	// 해당 장비 벗었다고 전환
	EquipManager->ChangeEquipState(EquipmentSlotType, false);
	EquipManager->CanInteract();

	//TEMP Jerry
	EquipmentManager->OnEquipmentEntryChanged.Broadcast(EquipmentSlotType, ItemInstance, ItemCount);

	return RemovedItemInstance;
}

bool FA1EquipmentList::NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
{
	return FFastArraySerializer::FastArrayDeltaSerialize<FA1EquipmentEntry, FA1EquipmentList>(Entries, DeltaParams,*this);
}

void FA1EquipmentList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (int32 AddedIndex : AddedIndices)
	{
		FA1EquipmentEntry& Entry = Entries[AddedIndex];
		Entry.EquipmentManager = EquipmentManager;
		Entry.EquipmentSlotType = (EEquipmentSlotType)AddedIndex;
		
		if (Entry.GetItemInstance())
		{
			BroadcastChangedMessage((EEquipmentSlotType)AddedIndex, Entry.GetItemInstance(), Entry.GetItemCount());
		}
	}
}

void FA1EquipmentList::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	for (int32 ChangedIndex : ChangedIndices)
	{
		const FA1EquipmentEntry& Entry = Entries[ChangedIndex];
		BroadcastChangedMessage((EEquipmentSlotType)ChangedIndex, Entry.GetItemInstance(), Entry.GetItemCount());
	}
}

void FA1EquipmentList::BroadcastChangedMessage(EEquipmentSlotType EquipmentSlotType, UA1ItemInstance* ItemInstance, int32 ItemCount)
{
	if (EquipmentManager->OnEquipmentEntryChanged.IsBound())
	{
		EquipmentManager->OnEquipmentEntryChanged.Broadcast(EquipmentSlotType, ItemInstance, ItemCount);
	}
}

UA1EquipmentManagerComponent::UA1EquipmentManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, EquipmentList(this)
{
	bWantsInitializeComponent = true;
    SetIsReplicatedByDefault(true);
}

void UA1EquipmentManagerComponent::InitializeComponent()
{
	Super::InitializeComponent();
	
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		TArray<FA1EquipmentEntry>& Entries = EquipmentList.Entries;
		Entries.SetNum((int32)EEquipmentSlotType::Count);

		for (int32 i = 0; i < Entries.Num(); i++)
		{
			FA1EquipmentEntry& Entry = Entries[i];
			Entry.EquipmentManager = this;
			Entry.EquipmentSlotType = (EEquipmentSlotType)i;
			EquipmentList.MarkItemDirty(Entry);
		}
	}
}

void UA1EquipmentManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, EquipmentList);
}

bool UA1EquipmentManagerComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bWroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	
	for (FA1EquipmentEntry& Entry : EquipmentList.Entries)
	{
		UA1ItemInstance* ItemInstance = Entry.ItemInstance;
		if (IsValid(ItemInstance))
		{
			bWroteSomething |= Channel->ReplicateSubobject(ItemInstance, *Bunch, *RepFlags);
		}
	}
	
	return bWroteSomething;
}

void UA1EquipmentManagerComponent::ReadyForReplication()
{
	Super::ReadyForReplication();

	if (IsUsingRegisteredSubObjectList())
	{
		for (const FA1EquipmentEntry& Entry : EquipmentList.Entries)
		{
			UA1ItemInstance* ItemInstance = Entry.GetItemInstance();
			if (IsValid(ItemInstance))
			{
				AddReplicatedSubObject(ItemInstance);
			}
		}
	}
}

int32 UA1EquipmentManagerComponent::CanMoveOrMergeEquipment(UA1EquipmentManagerComponent* OtherComponent, EEquipmentSlotType FromEquipmentSlotType, EEquipmentSlotType ToEquipmentSlotType) const
{
	if (OtherComponent == nullptr)
		return 0;

	if (FromEquipmentSlotType == EEquipmentSlotType::Count)
		return 0;

	const UA1ItemInstance* FromItemInstance = OtherComponent->GetItemInstance(FromEquipmentSlotType);
	const int32 FromItemCount = OtherComponent->GetItemCount(FromEquipmentSlotType);

	if (FromItemInstance == nullptr)
		return 0;

	if (this == OtherComponent && FromEquipmentSlotType == ToEquipmentSlotType)
		return FromItemCount;

	return CanAddEquipment(FromItemInstance->GetItemTemplateID(), FromItemInstance->GetItemRarity(), FromItemCount, ToEquipmentSlotType);
}

int32 UA1EquipmentManagerComponent::CanMoveOrMergeEquipment(UA1InventoryManagerComponent* OtherComponent, const FIntPoint& FromItemSlotPos, EEquipmentSlotType ToEquipmentSlotType) const
{
	if (OtherComponent == nullptr)
		return 0;

	const FIntPoint& FromInventorySlotCount = OtherComponent->GetInventorySlotCount();
	if (FromItemSlotPos.X < 0 || FromItemSlotPos.Y < 0 || FromItemSlotPos.X >= FromInventorySlotCount.X || FromItemSlotPos.Y >= FromInventorySlotCount.Y)
		return 0;

	const UA1ItemInstance* FromItemInstance = OtherComponent->GetItemInstance(FromItemSlotPos);
	const int32 FromItemCount = OtherComponent->GetItemCount(FromItemSlotPos);

	if (FromItemInstance == nullptr)
		return 0;

	return CanAddEquipment(FromItemInstance->GetItemTemplateID(), FromItemInstance->GetItemRarity(), FromItemCount, ToEquipmentSlotType);
}
/*
*   CanMoveOrMergeEquipment_Quick
*   빈 자리 있으면 바로 해당 자리에 가능한 지
*	없으면 아이템 개수 증가 가능한 지
*/
int32 UA1EquipmentManagerComponent::CanMoveOrMergeEquipment_Quick(UA1EquipmentManagerComponent* OtherComponent, EEquipmentSlotType FromEquipmentSlotType, EEquipmentSlotType& OutToEquipmentSlotType) const
{
	OutToEquipmentSlotType = EEquipmentSlotType::Count;

	if (OtherComponent == nullptr)
		return 0;

	if (FromEquipmentSlotType == EEquipmentSlotType::Count)
		return 0;

	if (this == OtherComponent)
		return 0;

	const UA1ItemInstance* FromItemInstance = OtherComponent->GetItemInstance(FromEquipmentSlotType);
	const int32 FromItemCount = OtherComponent->GetItemCount(FromEquipmentSlotType);

	if (FromItemInstance == nullptr)
		return 0;

	return CanMoveOrMergeEquipment_Quick(FromItemInstance->GetItemTemplateID(), FromItemInstance->GetItemRarity(), FromItemCount, OutToEquipmentSlotType);
}

int32 UA1EquipmentManagerComponent::CanMoveOrMergeEquipment_Quick(UA1InventoryManagerComponent* OtherComponent, const FIntPoint& FromItemSlotPos, EEquipmentSlotType& OutToEquipmentSlotType) const
{
	OutToEquipmentSlotType = EEquipmentSlotType::Count;

	if (OtherComponent == nullptr)
		return 0;

	const FIntPoint& FromInventorySlotCount = OtherComponent->GetInventorySlotCount();
	if (FromItemSlotPos.X < 0 || FromItemSlotPos.Y < 0 || FromItemSlotPos.X >= FromInventorySlotCount.X || FromItemSlotPos.Y >= FromInventorySlotCount.Y)
		return 0;

	const UA1ItemInstance* FromItemInstance = OtherComponent->GetItemInstance(FromItemSlotPos);
	const int32 FromItemCount = OtherComponent->GetItemCount(FromItemSlotPos);

	if (FromItemInstance == nullptr)
		return 0;

	return CanMoveOrMergeEquipment_Quick(FromItemInstance->GetItemTemplateID(), FromItemInstance->GetItemRarity(), FromItemCount, OutToEquipmentSlotType);
}

int32 UA1EquipmentManagerComponent::CanMoveOrMergeEquipment_Quick(int32 FromItemTemplateID, EItemRarity FromItemRarity, int32 FromItemCount, EEquipmentSlotType& OutToEquipmentSlotType) const
{
	OutToEquipmentSlotType = EEquipmentSlotType::Count;

	if (FromItemTemplateID <= 0 || FromItemRarity == EItemRarity::Count || FromItemCount <= 0)
		return 0;

	const UA1ItemTemplate& FromItemTemplate = UA1ItemData::Get().FindItemTemplateByID(FromItemTemplateID);
	const UA1ItemFragment_Equipable* FromEquippableFragment = FromItemTemplate.FindFragmentByClass<UA1ItemFragment_Equipable>();
	if (FromEquippableFragment == nullptr)
		return 0;

	const UA1ItemFragment_Equipable_Attachment* FromItemFragment = Cast<UA1ItemFragment_Equipable_Attachment>(FromEquippableFragment);
	EEquipmentSlotType ToEquipmentSlotType = FromItemFragment->ItemHandType;

	UA1ItemInstance* ToItemInstance = GetItemInstance(ToEquipmentSlotType);

	if((ToItemInstance != nullptr))
	{
		const int32 ToItemCount = GetItemCount(ToEquipmentSlotType);
		const UA1ItemTemplate& ToItemTemplate = UA1ItemData::Get().FindItemTemplateByID(ToItemInstance->GetItemTemplateID());

		if (!(ToItemTemplate.MaxStackCount > 1 && ToItemCount < ToItemTemplate.MaxStackCount && ToItemInstance->GetItemRarity() == FromItemRarity && ToItemInstance->GetItemTemplateID() == FromItemTemplateID))
			return 0;
	}
	
	int32 MovableCount = CanAddEquipment(FromItemTemplateID, FromItemRarity, FromItemCount, ToEquipmentSlotType);
	if (MovableCount > 0)
	{
		OutToEquipmentSlotType = ToEquipmentSlotType;
		return MovableCount;
	}
	
	return 0;
}

bool UA1EquipmentManagerComponent::CanSwapEquipment(UA1EquipmentManagerComponent* OtherComponent, EEquipmentSlotType FromEquipmentSlotType, EEquipmentSlotType ToEquipmentSlotType) const
{
	if (OtherComponent == nullptr || FromEquipmentSlotType == EEquipmentSlotType::Count)
		return false;

	const TArray<FA1EquipmentEntry>& FromEntries = OtherComponent->GetAllEntries();
	const FA1EquipmentEntry& FromEntry = FromEntries[(int32)FromEquipmentSlotType];
	UA1ItemInstance* FromItemInstance = FromEntry.GetItemInstance();

	if (FromItemInstance == nullptr)
		return false;

	const UA1ItemFragment_Equipable* FromEquippableFragment = FromItemInstance->FindFragmentByClass<UA1ItemFragment_Equipable>();
	if (FromEquippableFragment == nullptr)
		return false;

	if (this == OtherComponent && FromEquipmentSlotType == ToEquipmentSlotType)
		return true;

	const UA1ItemInstance* ToItemInstance = GetItemInstance(ToEquipmentSlotType);
	if (ToItemInstance == nullptr)
		return false;

	//if (FromEquippableFragment->EquipmentType == EEquipmentType::Weapon)
	//{
	//	const UA1ItemFragment_Equipable_Weapon* FromWeaponFragment = Cast<UA1ItemFragment_Equipable_Weapon>(FromEquippableFragment);
	//	if (FromWeaponFragment == nullptr)
	//		return false;
	//
	//	EWeaponHandType FromWeaponHandType = FromWeaponFragment->WeaponHandType;
	//	if (IsSameWeaponHandType(ToEquipmentSlotType, FromWeaponHandType) == false)
	//		return false;
	//
	//	return true;
	//}
	//else if (FromEquippableFragment->EquipmentType == EEquipmentType::Armor)
	//{
	//	if (FromEquipmentSlotType != ToEquipmentSlotType)
	//		return false;
	//
	//	return true;
	//}
	//else if (FromEquippableFragment->EquipmentType == EEquipmentType::Utility)
	//{
	//	if (IsUtilitySlot(ToEquipmentSlotType) == false)
	//		return false;
	//
	//	return true;
	//}

	return false;
}

bool UA1EquipmentManagerComponent::CanSwapEquipment(UA1InventoryManagerComponent* OtherComponent, const FIntPoint& FromItemSlotPos, EEquipmentSlotType ToEquipmentSlotType, FIntPoint& OutToItemSlotPos)
{
	if (OtherComponent == nullptr)
		return false;

	const FIntPoint& FromInventorySlotCount = OtherComponent->GetInventorySlotCount();
	if (FromItemSlotPos.X < 0 || FromItemSlotPos.Y < 0 || FromItemSlotPos.X >= FromInventorySlotCount.X || FromItemSlotPos.Y >= FromInventorySlotCount.Y)
		return false;

	if (ToEquipmentSlotType == EEquipmentSlotType::Count)
		return false;

	UA1ItemInstance* FromItemInstance = OtherComponent->GetItemInstance(FromItemSlotPos);
	if (FromItemInstance == nullptr)
		return false;

	const UA1ItemInstance* ToItemInstance = GetItemInstance(ToEquipmentSlotType);
	if (ToItemInstance == nullptr)
		return false;

	const UA1ItemFragment_Equipable* FromEquippableFragment = FromItemInstance->FindFragmentByClass<UA1ItemFragment_Equipable>();
	if (FromEquippableFragment == nullptr)
		return false;

	//if (FromEquippableFragment->EquipmentType == EEquipmentType::Weapon)
	//{
	//	const UA1ItemFragment_Equipable_Weapon* FromWeaponFragment = Cast<UA1ItemFragment_Equipable_Weapon>(FromEquippableFragment);
	//	if (FromWeaponFragment == nullptr)
	//		return false;
	//
	//	EWeaponHandType FromWeaponHandType = FromWeaponFragment->WeaponHandType;
	//	if (IsSameWeaponHandType(ToEquipmentSlotType, FromWeaponHandType) == false)
	//		return false;
	//}
	//else if (FromEquippableFragment->EquipmentType == EEquipmentType::Utility)
	//{
	//	if (IsUtilitySlot(ToEquipmentSlotType) == false)
	//		return false;
	//}

	const UA1ItemTemplate& FromItemTemplate = UA1ItemData::Get().FindItemTemplateByID(FromItemInstance->GetItemTemplateID());
	const FIntPoint& FromSlotCount = FromItemTemplate.SlotCount;

	const UA1ItemTemplate& ToItemTemplate = UA1ItemData::Get().FindItemTemplateByID(ToItemInstance->GetItemTemplateID());
	const FIntPoint& ToSlotCount = ToItemTemplate.SlotCount;

	TArray<bool> TempSlotChecks = OtherComponent->SlotChecks;
	OtherComponent->MarkSlotChecks(TempSlotChecks, false, FromItemSlotPos, FromSlotCount);

	if (OtherComponent->IsEmpty(TempSlotChecks, FromItemSlotPos, ToSlotCount))
	{
		OutToItemSlotPos = FromItemSlotPos;
		return true;
	}
	else
	{
		const FIntPoint StartSlotPos = FIntPoint::ZeroValue;
		const FIntPoint EndSlotPos = FromInventorySlotCount - ToSlotCount;

		for (int32 y = StartSlotPos.Y; y <= EndSlotPos.Y; y++)
		{
			for (int32 x = StartSlotPos.X; x <= EndSlotPos.X; x++)
			{
				int32 Index = y * FromInventorySlotCount.X + x;
				if (TempSlotChecks.IsValidIndex(Index) == false || TempSlotChecks[Index])
					continue;

				FIntPoint ToItemSlotPos = FIntPoint(x, y);
				if (OtherComponent->IsEmpty(TempSlotChecks, ToItemSlotPos, ToSlotCount))
				{
					OutToItemSlotPos = ToItemSlotPos;
					return true;
				}
			}
		}
	}

	return false;
}

bool UA1EquipmentManagerComponent::CanSwapEquipment_Quick(UA1EquipmentManagerComponent* OtherComponent, EEquipmentSlotType FromEquipmentSlotType, EEquipmentSlotType& OutToEquipmentSlotType) const
{
	OutToEquipmentSlotType = EEquipmentSlotType::Count;

	if (OtherComponent == nullptr)
		return false;

	if (FromEquipmentSlotType == EEquipmentSlotType::Count)
		return false;

	UA1ItemInstance* FromItemInstance = OtherComponent->GetItemInstance(FromEquipmentSlotType);
	if (FromItemInstance == nullptr)
		return false;

	const UA1ItemFragment_Equipable* FromEquippableFragment = FromItemInstance->FindFragmentByClass<UA1ItemFragment_Equipable>();
	if (FromEquippableFragment == nullptr)
		return false;

	if (FromEquippableFragment->EquipmentType == EEquipmentType::Weapon)
	{
		const UA1ItemFragment_Equipable_Weapon* FromWeaponFragment = Cast<UA1ItemFragment_Equipable_Weapon>(FromEquippableFragment);
		if (FromWeaponFragment == nullptr)
			return false;

		if (FindPairItemInstance(FromItemInstance, OutToEquipmentSlotType))
			return true;
	}
	else if (FromEquippableFragment->EquipmentType == EEquipmentType::Utility)
	{
		// TODO Jerry
	}

	return false;
}

bool UA1EquipmentManagerComponent::CanSwapEquipment_Quick(UA1InventoryManagerComponent* OtherComponent, const FIntPoint& FromItemSlotPos, EEquipmentSlotType& OutToEquipmentSlotType, FIntPoint& OutToItemSlotPos)
{
	OutToEquipmentSlotType = EEquipmentSlotType::Count;

	if (OtherComponent == nullptr)
		return false;

	const FIntPoint& FromInventorySlotCount = OtherComponent->GetInventorySlotCount();
	if (FromItemSlotPos.X < 0 || FromItemSlotPos.Y < 0 || FromItemSlotPos.X >= FromInventorySlotCount.X || FromItemSlotPos.Y >= FromInventorySlotCount.Y)
		return false;

	UA1ItemInstance* FromItemInstance = OtherComponent->GetItemInstance(FromItemSlotPos);
	if (FromItemInstance == nullptr)
		return false;

	const UA1ItemFragment_Equipable* FromEquippableFragment = FromItemInstance->FindFragmentByClass<UA1ItemFragment_Equipable>();
	if (FromEquippableFragment == nullptr)
		return false;

	const UA1ItemInstance* ToItemInstance = nullptr;

	if (FromEquippableFragment->EquipmentType == EEquipmentType::Weapon)
	{
		const UA1ItemFragment_Equipable_Weapon* FromWeaponFragment = Cast<UA1ItemFragment_Equipable_Weapon>(FromEquippableFragment);
		if (FromWeaponFragment == nullptr)
			return false;

		ToItemInstance = FindPairItemInstance(FromItemInstance, OutToEquipmentSlotType);
	}
	else if (FromEquippableFragment->EquipmentType == EEquipmentType::Utility)
	{
		// TODO Jerry
	}

	if (ToItemInstance == nullptr)
		return false;

	const UA1ItemTemplate& FromItemTemplate = UA1ItemData::Get().FindItemTemplateByID(FromItemInstance->GetItemTemplateID());
	const FIntPoint& FromSlotCount = FromItemTemplate.SlotCount;

	const UA1ItemTemplate& ToItemTemplate = UA1ItemData::Get().FindItemTemplateByID(ToItemInstance->GetItemTemplateID());
	const FIntPoint& ToSlotCount = ToItemTemplate.SlotCount;

	TArray<bool> TempSlotChecks = OtherComponent->SlotChecks;
	OtherComponent->MarkSlotChecks(TempSlotChecks, false, FromItemSlotPos, FromSlotCount);

	if (OtherComponent->IsEmpty(TempSlotChecks, FromItemSlotPos, ToSlotCount))
	{
		OutToItemSlotPos = FromItemSlotPos;
		return true;
	}
	else
	{
		const FIntPoint StartSlotPos = FIntPoint::ZeroValue;
		const FIntPoint EndSlotPos = FromInventorySlotCount - ToSlotCount;

		for (int32 y = StartSlotPos.Y; y <= EndSlotPos.Y; y++)
		{
			for (int32 x = StartSlotPos.X; x <= EndSlotPos.X; x++)
			{
				int32 Index = y * FromInventorySlotCount.X + x;
				if (TempSlotChecks.IsValidIndex(Index) == false || TempSlotChecks[Index])
					continue;

				FIntPoint ToItemSlotPos = FIntPoint(x, y);
				if (OtherComponent->IsEmpty(TempSlotChecks, ToItemSlotPos, ToSlotCount))
				{
					OutToItemSlotPos = ToItemSlotPos;
					return true;
				}
			}
		}
	}

	return false;
}

int32 UA1EquipmentManagerComponent::CanAddEquipment(int32 ItemTemplateID, EItemRarity ItemRarity, int32 ItemCount, EEquipmentSlotType ToEquipmentSlotType) const
{
	if (ItemTemplateID <= 0 || ItemRarity == EItemRarity::Count || ItemCount <= 0)
		return 0;
	
	if (ToEquipmentSlotType == EEquipmentSlotType::Count)
		return 0;
	
	const UA1ItemTemplate& ItemTemplate = UA1ItemData::Get().FindItemTemplateByID(ItemTemplateID);
	const UA1ItemFragment_Equipable* FromEquippableFragment = ItemTemplate.FindFragmentByClass<UA1ItemFragment_Equipable>();
	if (FromEquippableFragment == nullptr)
		return 0;
	
	const UA1ItemInstance* ToItemInstance = GetItemInstance(ToEquipmentSlotType);
	const int32 ToItemCount = GetItemCount(ToEquipmentSlotType);

	if (ToItemInstance)
	{
		const int32 ToTemplateID = ToItemInstance->GetItemTemplateID();
		if (ItemTemplateID != ToTemplateID)
			return 0;

		if (ItemRarity != ToItemInstance->GetItemRarity())
			return 0;
		
		if (ItemTemplate.MaxStackCount < 2)
			return 0;

		return FMath::Min(ItemCount + ToItemCount, ItemTemplate.MaxStackCount) - ToItemCount;
	}
	else
	{
		const UA1ItemFragment_Equipable_Attachment* FromItemFragment = Cast<UA1ItemFragment_Equipable_Attachment>(FromEquippableFragment);
		EEquipmentSlotType FromEquipSlotType = FromItemFragment->ItemHandType;

		if(FromEquipSlotType == EEquipmentSlotType::LeftHand || FromEquipSlotType == EEquipmentSlotType::RightHand)
		{
			return ((GetItemInstance(EEquipmentSlotType::TwoHand) == nullptr) && (FromEquipSlotType == ToEquipmentSlotType)) ? ItemCount : 0;
		}
		if (FromEquipSlotType == EEquipmentSlotType::TwoHand)
		{
			return (GetItemInstance(EEquipmentSlotType::LeftHand) == nullptr && GetItemInstance(EEquipmentSlotType::RightHand) == nullptr) ? ItemCount : 0;
		}
	}

	return 0;
}

void UA1EquipmentManagerComponent::AddUnarmedEquipments(TSubclassOf<UA1ItemTemplate> LeftHandClass, TSubclassOf<UA1ItemTemplate> RightHandClass)
{
	check(HasAuthority());

	SetEquipment(EEquipmentSlotType::LeftHand, LeftHandClass, EItemRarity::Poor, 1);
	SetEquipment(EEquipmentSlotType::RightHand, RightHandClass, EItemRarity::Poor, 1);
}

void UA1EquipmentManagerComponent::AddEquipment_Unsafe(EEquipmentSlotType EquipmentSlotType, UA1ItemInstance* ItemInstance, int32 ItemCount)
{
	check(HasAuthority());

	if (EquipmentSlotType == EEquipmentSlotType::Count || ItemCount <= 0)
		return;
	
	FA1EquipmentEntry& Entry = EquipmentList.Entries[(int32)EquipmentSlotType];
	
	if (Entry.GetItemInstance())
	{
		Entry.ItemCount += ItemCount;
		EquipmentList.MarkItemDirty(Entry);
	}
	else
	{
		if (ItemInstance == nullptr)
			return;
		
		Entry.Init(ItemInstance, ItemCount);
		
		if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && ItemInstance)
		{
			AddReplicatedSubObject(ItemInstance);
		}
		
		EquipmentList.MarkItemDirty(Entry);
	}
}

UA1ItemInstance* UA1EquipmentManagerComponent::RemoveEquipment_Unsafe(EEquipmentSlotType EquipmentSlotType, int32 ItemCount)
{
	check(HasAuthority());

	FA1EquipmentEntry& Entry = EquipmentList.Entries[(int32)EquipmentSlotType];
	UA1ItemInstance* ItemInstance = Entry.GetItemInstance();

	Entry.ItemCount -= ItemCount;
	if (Entry.GetItemCount() <= 0)
	{
		UA1ItemInstance* RemovedItemInstance = Entry.Reset();
		if (IsUsingRegisteredSubObjectList() && RemovedItemInstance)
		{
			RemoveReplicatedSubObject(RemovedItemInstance);
		}	
	}

	EquipmentList.MarkItemDirty(Entry);
	return ItemInstance;
}

void UA1EquipmentManagerComponent::SetEquipment(EEquipmentSlotType EquipmentSlotType, TSubclassOf<UA1ItemTemplate> ItemTemplateClass, EItemRarity ItemRarity, int32 ItemCount)
{
	check(HasAuthority());

	if (EquipmentSlotType == EEquipmentSlotType::Count || ItemTemplateClass == nullptr || ItemRarity == EItemRarity::Count || ItemCount <= 0)
		return;

	const int32 ItemTemplateID = UA1ItemData::Get().FindItemTemplateIDByClass(ItemTemplateClass);
	const UA1ItemTemplate& ItemTemplate = UA1ItemData::Get().FindItemTemplateByID(ItemTemplateID);

	ItemCount = FMath::Clamp(ItemCount, 1, ItemTemplate.MaxStackCount);

	const UA1ItemFragment_Equipable* EquippableFragment = ItemTemplate.FindFragmentByClass<UA1ItemFragment_Equipable>();
	if (EquippableFragment == nullptr)
		return;

	FA1EquipmentEntry& Entry = EquipmentList.Entries[(int32)EquipmentSlotType];
	Entry.Reset();

	if (EquippableFragment->EquipmentType == EEquipmentType::Weapon)
	{
		const UA1ItemFragment_Equipable_Weapon* WeaponFragment = Cast<UA1ItemFragment_Equipable_Weapon>(EquippableFragment);
		EEquipmentSlotType ItemHandType = WeaponFragment->ItemHandType;

		if (ItemHandType == EEquipmentSlotType::LeftHand || ItemHandType == EEquipmentSlotType::RightHand)
		{
			RemoveEquipment_Unsafe(EEquipmentSlotType::TwoHand, 1);
		}
		else if (ItemHandType == EEquipmentSlotType::TwoHand)
		{
			RemoveEquipment_Unsafe(EEquipmentSlotType::LeftHand, 1);
			RemoveEquipment_Unsafe(EEquipmentSlotType::RightHand, 1);
		}
	}

	UA1ItemInstance* AddedItemInstance = NewObject<UA1ItemInstance>();
	AddedItemInstance->Init(ItemTemplateID, ItemRarity);
	Entry.Init(AddedItemInstance, ItemCount);

	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && AddedItemInstance)
	{
		AddReplicatedSubObject(AddedItemInstance);
	}

	EquipmentList.MarkItemDirty(Entry);
}

bool UA1EquipmentManagerComponent::IsSameEquipState(EEquipmentSlotType EquipmentSlotType, EEquipState WeaponEquipState)
{
	return (((EquipmentSlotType == EEquipmentSlotType::LeftHand || EquipmentSlotType == EEquipmentSlotType::RightHand || EquipmentSlotType == EEquipmentSlotType::TwoHand) && WeaponEquipState != EEquipState::Unarmed));
}

const UA1ItemInstance* UA1EquipmentManagerComponent::FindPairItemInstance(const UA1ItemInstance* InBaseItemInstance, EEquipmentSlotType& OutEquipmentSlotType) const
{
	if (InBaseItemInstance == nullptr)
		return nullptr;

	const UA1ItemFragment_Equipable* BaseEquippableFragment = InBaseItemInstance->FindFragmentByClass<UA1ItemFragment_Equipable>();
	if (BaseEquippableFragment == nullptr)
		return nullptr;
	
	UA1ItemInstance* SelectedItemInstance = nullptr;

	if (BaseEquippableFragment->EquipmentType == EEquipmentType::Weapon)
	{
		const UA1ItemFragment_Equipable_Weapon* BaseWeaponFragment = Cast<UA1ItemFragment_Equipable_Weapon>(BaseEquippableFragment);
		
		const TArray<FA1EquipmentEntry>& Entries = EquipmentList.GetAllEntries();
		for (int32 i = 0; i < (int32)EEquipmentSlotType::Count; i++)
		{
			const FA1EquipmentEntry& Entry = Entries[i];
			if (UA1ItemInstance* ItemInstance = Entry.ItemInstance)
			{
				if (const UA1ItemFragment_Equipable_Weapon* EquippedWeaponFragment = ItemInstance->FindFragmentByClass<UA1ItemFragment_Equipable_Weapon>())
				{
					if ((BaseWeaponFragment->WeaponType == EquippedWeaponFragment->WeaponType) && (BaseWeaponFragment->ItemHandType == EquippedWeaponFragment->ItemHandType))
					{
						if (InBaseItemInstance == ItemInstance)
						{
							SelectedItemInstance = nullptr;
							break;
						}

						if (SelectedItemInstance == nullptr)
						{
							SelectedItemInstance = ItemInstance;
							OutEquipmentSlotType = (EEquipmentSlotType)i;
						}
					}
				}
			}
		}
	}
	else if (BaseEquippableFragment->EquipmentType == EEquipmentType::Armor)
	{
		/*const UA1ItemFragment_Equipable_Armor* BaseArmorFragment = Cast<UA1ItemFragment_Equipable_Armor>(BaseEquippableFragment);
		OutEquipmentSlotType = UA1EquipManagerComponent::ConvertToEquipmentSlotType(BaseArmorFragment->ArmorType);
		SelectedItemInstance = GetItemInstance(OutEquipmentSlotType);*/
	}

	if (InBaseItemInstance == SelectedItemInstance)
	{
		SelectedItemInstance = nullptr;
	}

	return SelectedItemInstance;
}

ALyraCharacter* UA1EquipmentManagerComponent::GetCharacter() const
{
	return Cast<ALyraCharacter>(GetOwner());
}

ALyraPlayerController* UA1EquipmentManagerComponent::GetPlayerController() const
{
	if (ALyraCharacter* LyraCharacter = GetCharacter())
	{
		return LyraCharacter->GetLyraPlayerController();
	}
	return nullptr;
}

UA1EquipManagerComponent* UA1EquipmentManagerComponent::GetEquipManager() const
{
	UA1EquipManagerComponent* EquipManager = nullptr;
	if (ALyraCharacter* Character = GetCharacter())
	{
		EquipManager = Character->FindComponentByClass<UA1EquipManagerComponent>();
	}
	return EquipManager;
}

UA1ItemInstance* UA1EquipmentManagerComponent::GetItemInstance(EEquipmentSlotType EquipmentSlotType) const
{
	if (EquipmentSlotType == EEquipmentSlotType::Count)
		return nullptr;
	
	const TArray<FA1EquipmentEntry>& Entries = EquipmentList.GetAllEntries();
	const FA1EquipmentEntry& Entry = Entries[(int32)EquipmentSlotType];
	
	return Entry.GetItemInstance();
}

int32 UA1EquipmentManagerComponent::GetItemCount(EEquipmentSlotType EquipmentSlotType) const
{
	if (EquipmentSlotType == EEquipmentSlotType::Count)
		return 0;

	const TArray<FA1EquipmentEntry>& Entries = EquipmentList.GetAllEntries();
	const FA1EquipmentEntry& Entry = Entries[(int32)EquipmentSlotType];
	
	return Entry.GetItemCount();
}

const TArray<FA1EquipmentEntry>& UA1EquipmentManagerComponent::GetAllEntries() const
{
	return EquipmentList.GetAllEntries();
}

void UA1EquipmentManagerComponent::GetAllWeaponItemInstances(TArray<UA1ItemInstance*>& OutItemInstances) const
{
	OutItemInstances.Reset();
	
	const TArray<FA1EquipmentEntry>& Entries = EquipmentList.GetAllEntries();

	for (int32 i = 0; i < (int32)EEquipmentSlotType::Count; i++)
	{
		const FA1EquipmentEntry& Entry = Entries[i];
		if (Entry.ItemInstance)
		{
			OutItemInstances.Add(Entry.ItemInstance);
		}
	}
}
