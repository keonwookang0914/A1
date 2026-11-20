// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "Interaction/A1WorldInteractable.h"
#include "A1ChestBase.generated.h"

class UA1ItemTemplate;
class UArrowComponent;

UENUM(BlueprintType)
enum class EChestState : uint8
{
	Close,
	Open
};

/**
 *
 */
UCLASS()
class AA1ChestBase : public AA1WorldInteractable
{
	GENERATED_BODY()
public:
	AA1ChestBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
public:
	virtual FA1InteractionInfo GetPreInteractionInfo(const FA1InteractionQuery& InteractionQuery) const override;
	virtual void GetMeshComponents(TArray<UMeshComponent*>& OutMeshComponents) const override;

	UFUNCTION(BlueprintImplementableEvent)
	void OnChestStateChanged(EChestState InChestState);

	UFUNCTION()
	void OnRep_ChestState();

public:
	UFUNCTION(BlueprintCallable)
	FORCEINLINE	void SetChestState(EChestState InChestState) { ChestState = InChestState; }

	UFUNCTION(BlueprintCallable)
	EChestState GetChestState() const { return ChestState; }

	UFUNCTION(BlueprintCallable)
	void SpawnItems();


	UFUNCTION(BlueprintCallable)
	void SpawnItem(int32 idx);



protected:
	UPROPERTY(EditDefaultsOnly, Category = "Bed|Info")
	FA1InteractionInfo OpenInteractionInfo;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UArrowComponent> ArrowComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> ItemLocation;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_ChestState)
	EChestState ChestState = EChestState::Close;

	//Item Section
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TObjectPtr<USceneComponent>> ItemLocations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSubclassOf<UA1ItemTemplate>> CachedItemTemplates;
};
