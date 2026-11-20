#pragma once

#include "A1Define.h"
#include "Components/PawnComponent.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "A1EquipmentManagerComponent.generated.h"

class UA1EquipManagerComponent;
class ALyraCharacter;
class ALyraPlayerController;
class UA1ItemInstance;
class UA1ItemTemplate;
class UA1EquipmentManagerComponent;
class UA1InventoryManagerComponent;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnEquipmentEntryChanged, EEquipmentSlotType, UA1ItemInstance*, int32 ItemCount);

USTRUCT(BlueprintType)
struct FA1EquipmentEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()
	
private:
	void Init(UA1ItemInstance* InItemInstance, int32 InItemCount);
	UA1ItemInstance* Reset();
	
public:
	UA1ItemInstance* GetItemInstance() const { return ItemInstance; }
	int32 GetItemCount() const { return ItemCount; }
	
private:
	friend struct FA1EquipmentList;
	friend class UA1EquipmentManagerComponent;
	friend class UA1ItemManagerComponent;

	UPROPERTY()
	TObjectPtr<UA1ItemInstance> ItemInstance;

	UPROPERTY()
	int32 ItemCount = 0;

private:
	UPROPERTY(NotReplicated)
	EEquipmentSlotType EquipmentSlotType = EEquipmentSlotType::Count;
	
	UPROPERTY(NotReplicated)
	TObjectPtr<UA1EquipmentManagerComponent> EquipmentManager;
};

USTRUCT(BlueprintType)
struct FA1EquipmentList : public FFastArraySerializer
{
	GENERATED_BODY()

public:
	FA1EquipmentList() : EquipmentManager(nullptr) { }
	FA1EquipmentList(UA1EquipmentManagerComponent* InOwnerComponent) : EquipmentManager(InOwnerComponent) { }
	
public:
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
	
private:
	void BroadcastChangedMessage(EEquipmentSlotType EquipmentSlotType, UA1ItemInstance* ItemInstance, int32 ItemCount);
	
public:
	const TArray<FA1EquipmentEntry>& GetAllEntries() const { return Entries; }
	
private:
	friend class UA1EquipmentManagerComponent;
	friend class UA1InventoryManagerComponent;
	friend class UA1ItemManagerComponent;

	UPROPERTY()
	TArray<FA1EquipmentEntry> Entries;
	
	UPROPERTY(NotReplicated)
	TObjectPtr<UA1EquipmentManagerComponent> EquipmentManager;
};

template<>
struct TStructOpsTypeTraits<FA1EquipmentList> : public TStructOpsTypeTraitsBase2<FA1EquipmentList>
{
	enum
	{
		WithNetDeltaSerializer = true
	};
};

UCLASS(BlueprintType)
class UA1EquipmentManagerComponent : public UPawnComponent
{
	GENERATED_BODY()
	
public:
	UA1EquipmentManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void InitializeComponent() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	virtual void ReadyForReplication() override;
	
public:
	
	int32 CanMoveOrMergeEquipment(UA1EquipmentManagerComponent* OtherComponent, EEquipmentSlotType FromEquipmentSlotType, EEquipmentSlotType ToEquipmentSlotType) const;
	int32 CanMoveOrMergeEquipment(UA1InventoryManagerComponent* OtherComponent, const FIntPoint& FromItemSlotPos, EEquipmentSlotType ToEquipmentSlotType) const;
	
	int32 CanMoveOrMergeEquipment_Quick(UA1EquipmentManagerComponent* OtherComponent, EEquipmentSlotType FromEquipmentSlotType, EEquipmentSlotType& OutToEquipmentSlotType) const;
	int32 CanMoveOrMergeEquipment_Quick(UA1InventoryManagerComponent* OtherComponent, const FIntPoint& FromItemSlotPos, EEquipmentSlotType& OutToEquipmentSlotType) const;
	int32 CanMoveOrMergeEquipment_Quick(int32 FromItemTemplateID, EItemRarity FromItemRarity, int32 FromItemCount, EEquipmentSlotType& OutToEquipmentSlotType) const;

	bool CanSwapEquipment(UA1EquipmentManagerComponent* OtherComponent, EEquipmentSlotType FromEquipmentSlotType, EEquipmentSlotType ToEquipmentSlotType) const;
	bool CanSwapEquipment(UA1InventoryManagerComponent* OtherComponent, const FIntPoint& FromItemSlotPos, EEquipmentSlotType ToEquipmentSlotType, FIntPoint& OutToItemSlotPos);
	
	bool CanSwapEquipment_Quick(UA1EquipmentManagerComponent* OtherComponent, EEquipmentSlotType FromEquipmentSlotType, EEquipmentSlotType& OutToEquipmentSlotType) const;
	bool CanSwapEquipment_Quick(UA1InventoryManagerComponent* OtherComponent, const FIntPoint& FromItemSlotPos, EEquipmentSlotType& OutToEquipmentSlotType, FIntPoint& OutToItemSlotPos);
	
	
	int32 CanAddEquipment(int32 ItemTemplateID, EItemRarity ItemRarity, int32 ItemCount, EEquipmentSlotType ToEquipmentSlotType) const;
	
public:
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void AddUnarmedEquipments(TSubclassOf<UA1ItemTemplate> LeftHandClass, TSubclassOf<UA1ItemTemplate> RightHandClass);
	
public:
	void AddEquipment_Unsafe(EEquipmentSlotType EquipmentSlotType, UA1ItemInstance* ItemInstance, int32 ItemCount);
	UA1ItemInstance* RemoveEquipment_Unsafe(EEquipmentSlotType EquipmentSlotType, int32 ItemCount);
	void SetEquipment(EEquipmentSlotType EquipmentSlotType, TSubclassOf<UA1ItemTemplate> ItemTemplateClass, EItemRarity ItemRarity, int32 ItemCount);
	
public:
	const UA1ItemInstance* FindPairItemInstance(const UA1ItemInstance* InBaseItemInstance, EEquipmentSlotType& OutEquipmentSlotType) const;
	static bool IsSameEquipState(EEquipmentSlotType EquipmentSlotType, EEquipState WeaponEquipState);

public:
	ALyraCharacter* GetCharacter() const;
	ALyraPlayerController* GetPlayerController() const;
	UA1EquipManagerComponent* GetEquipManager() const;
	
	UA1ItemInstance* GetItemInstance(EEquipmentSlotType EquipmentSlotType) const;
	int32 GetItemCount(EEquipmentSlotType EquipmentSlotType) const;
	
	const TArray<FA1EquipmentEntry>& GetAllEntries() const;
	void GetAllWeaponItemInstances(TArray<UA1ItemInstance*>& OutItemInstances) const;

public:
	FOnEquipmentEntryChanged OnEquipmentEntryChanged;
	
private:
	friend class UD1ItemManagerComponent;
	friend class ULyraCheatManager;
	
	UPROPERTY(Replicated)
	FA1EquipmentList EquipmentList;
};
