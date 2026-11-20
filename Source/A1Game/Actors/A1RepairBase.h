// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "Interaction/A1WorldInteractable.h"
#include "A1RepairBase.generated.h"

class AA1SpaceshipBase;
class ALyraCharacter;
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRepairStateChanged);

class UArrowComponent;

UENUM(BlueprintType)
enum class RepairState : uint8
{
	NotBroken,
	Break,
	Foamed,
	Complete
};

UCLASS()
class AA1RepairBase : public AA1WorldInteractable
{
	GENERATED_BODY()

public:
	AA1RepairBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UFUNCTION(BlueprintImplementableEvent)
	void ActivateCheckOverlap();

	virtual FA1InteractionInfo GetPreInteractionInfo(const FA1InteractionQuery& InteractionQuery) const override;
	virtual void GetMeshComponents(TArray<UMeshComponent*>& OutMeshComponents) const override;

	FORCEINLINE RepairState GetCurrentState() const { return CurrentState; }
	UFUNCTION(BlueprintCallable)
	void SetCurrentState(RepairState InState);

	UFUNCTION(BlueprintCallable)
	void OnRepairChanged();

	UFUNCTION(BlueprintImplementableEvent)
	void SetSpriteNotBroken();
	UFUNCTION(BlueprintImplementableEvent)
	void SetSpriteBreak();
	UFUNCTION(BlueprintImplementableEvent)
	void SetSpriteFoamed();
	UFUNCTION(BlueprintImplementableEvent)
	void SetSpriteComplete();

protected:
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsHoldingRepairKit(ALyraCharacter* Character) const;

	void FindOwningSpaceship();

public:
	UPROPERTY(BlueprintAssignable)
	FOnRepairStateChanged OnRepairStateChanged;
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Info")
	FA1InteractionInfo InteractionInfo;

	UPROPERTY(EditDefaultsOnly, Category = "Info")
	FA1InteractionInfo NoRepairKitInteractionInfo;

	UPROPERTY(EditDefaultsOnly, Category = "Info")
	FA1InteractionInfo AlreadyRepairedInteractionInfo;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UArrowComponent> ArrowComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY()
	TObjectPtr<AA1SpaceshipBase> CachedSpaceship;

public:
	UPROPERTY(VisibleAnywhere)
	RepairState CurrentState = RepairState::Break;

private:
	bool bIsFoamed = false;
};
