// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "Interaction/A1WorldInteractable.h"
#include "A1StorageEntryBase.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemEntryStateChanged, AA1EquipmentBase*, CachedItem);

enum class EEquipmentSlotType : uint8;
enum class EItemRarity : uint8;
class UArrowComponent;
class AA1EquipmentBase;
class UBoxComponent;

UENUM()
enum class EItemEntryState
{
	None,
	Exist
};
/**
 * 
 */
UCLASS()
class AA1StorageEntryBase : public AA1WorldInteractable
{
	GENERATED_BODY()
public:
	AA1StorageEntryBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
public:
	virtual FA1InteractionInfo GetPreInteractionInfo(const FA1InteractionQuery& InteractionQuery) const override;

	void SetItemTransform(int32 ItemTemplateID, EItemRarity ItemRarity, EEquipmentSlotType SlotToCheck);

	UFUNCTION()
	void SetItemInput();

	UFUNCTION()
	void SetItemOutput();

	void SpawnItem(int32 TemplateID);

	FORCEINLINE EItemEntryState GetItemEntryState() const { return ItemState; }
	FORCEINLINE void SetItemEntryState(EItemEntryState InItemState) { ItemState = InItemState; }
	FORCEINLINE AA1EquipmentBase* GetItem() { return CachedItem; }

	FOnItemEntryStateChanged OnItemEntryStateChanged;
protected:
	//필요한 기능
	/*
	 * 1. 아이템을 지정된 위치로 가져오는 기능
	 * 2. 아이템이 TryInteract로 빠지면 변수 해제하는 기능 -> overlap으로 구현
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UArrowComponent> ArrowComponent;

	UPROPERTY(VisibleAnywhere, Category = "Entry", Replicated)
	TObjectPtr<UBoxComponent> ItemTrigger;

	UPROPERTY(EditDefaultsOnly, Category = "Entry|Info")
	FA1InteractionInfo ItemInputInfo;

	UPROPERTY(EditDefaultsOnly, Category = "Entry|Info")
	FA1InteractionInfo ItemOutputInfo;

	UPROPERTY(VisibleAnywhere, Category = "Entry", Replicated)
	TObjectPtr<AA1EquipmentBase> CachedItem;

	UPROPERTY(VisibleAnywhere, Category = "Entry")
	EItemEntryState ItemState = EItemEntryState::None;
};
