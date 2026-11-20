#pragma once

#include "Components/ControllerComponent.h"
#include "A1ItemManagerComponent.generated.h"

class UA1ItemInstance;
class AA1EquipmentBase;
class AA1PickupableItemBase;
class UA1EquipmentManagerComponent;
class UA1InventoryManagerComponent;

UCLASS()
class UA1ItemManagerComponent : public UControllerComponent
{
	GENERATED_BODY()
	
public:
	UA1ItemManagerComponent(const FObjectInitializer& ObjectInitializer);
	
public:
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_InventoryToEquipment(UA1InventoryManagerComponent* FromInventoryManager, const FIntPoint& FromItemSlotPos, UA1EquipmentManagerComponent* ToEquipmentManager, EEquipmentSlotType ToEquipmentSlotType);
	
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_EquipmentToInventory(UA1EquipmentManagerComponent* FromEquipmentManager, EEquipmentSlotType FromEquipmentSlotType, UA1InventoryManagerComponent* ToInventoryManager, const FIntPoint& ToItemSlotPos);
	
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_InventoryToInventory(UA1InventoryManagerComponent* FromInventoryManager, const FIntPoint& FromItemSlotPos, UA1InventoryManagerComponent* ToInventoryManager, const FIntPoint& ToItemSlotPos);
	
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_EquipmentToEquipment(UA1EquipmentManagerComponent* FromEquipmentManager, EEquipmentSlotType FromEquipmentSlotType, UA1EquipmentManagerComponent* ToEquipmentManager, EEquipmentSlotType ToEquipmentSlotType);
	
public:
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_QuickFromInventory(UA1InventoryManagerComponent* FromInventoryManager, const FIntPoint& FromItemSlotPos);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_QuickFromEquipment(UA1EquipmentManagerComponent* FromEquipmentManager, EEquipmentSlotType FromEquipmentSlotType);
	
public:
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_DropItemFromInventory(UA1InventoryManagerComponent* FromInventoryManager, const FIntPoint& FromItemSlotPos);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_DropItemFromEquipment(UA1EquipmentManagerComponent* FromEquipmentManager, EEquipmentSlotType FromEquipmentSlotType);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_DropItem(bool bActivateWidget);

public:
	bool TryPickItem(AA1EquipmentBase* PickupableItemActor);
	bool TryDropItem(UA1ItemInstance* FromItemInstance, int32 FromItemCount);

	/*UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	bool TryDropItem(UA1ItemInstance* FromItemInstance, int32 FromItemCount);*/
	
public:
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void AddAllowedComponent(UActorComponent* ActorComponent);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void RemoveAllowedComponent(UActorComponent* ActorComponent);
	
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, BlueprintPure)
	bool IsAllowedComponent(UActorComponent* ActorComponent) const;

protected:
	UFUNCTION(BlueprintCallable, BlueprintPure)
	UA1InventoryManagerComponent* GetMyInventoryManager() const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	UA1EquipmentManagerComponent* GetMyEquipmentManager() const;
	
private:
	UPROPERTY()
	TArray<TWeakObjectPtr<UActorComponent>> AllowedComponents;
};
