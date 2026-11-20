// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "A1SpaceshipBase.h"
#include "Interaction/A1WorldInteractable.h"
#include "A1StorageBase.generated.h"

class AA1StorageEntryBase;
class AA1EquipmentBase;
class UBoxComponent;
class UA1StorageComponent;
class UArrowComponent;
class AA1RaiderBase;

/**
 * Storage Interactable Object
 * - Can Store Item Instance
 */
UCLASS()
class AA1StorageBase : public AActor, public IA1SpaceshipComponent
{
	GENERATED_BODY()

public:
	AA1StorageBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

public:
	//IA1SpaceshipComponent
	virtual void RegisterWithSpaceship(AA1SpaceshipBase* Spaceship) override;
	virtual ESpaceshipComponentType GetComponentType() const override { return ESpaceshipComponentType::Storage; }

public:
	bool RemoveItem(AA1RaiderBase* InstigatorActor);
	bool EntryIsEmpty();

	void SpawnDefaultItems();

protected:
	void SetupTags();

	UFUNCTION(BlueprintCallable)
	AA1SpaceshipBase* FindSpaceshipOwner() const;

	FORCEINLINE	AA1SpaceshipBase* GetOwningSpaceship() const { return OwningSpaceship.IsValid() ? OwningSpaceship.Get() : nullptr; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UArrowComponent> ArrowComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, Transient)
	TWeakObjectPtr<AA1SpaceshipBase> OwningSpaceship;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AA1StorageEntryBase> EntryClass;

	UPROPERTY(VisibleAnywhere, Replicated)
	TArray<TObjectPtr<AA1StorageEntryBase>> StorageEntries;

	
	int32 StorageWidthNum;
	int32 StorageHeightNum;
	FVector SpawnStartLocation;	
};
