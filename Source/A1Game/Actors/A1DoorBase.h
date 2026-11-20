// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "A1SpaceshipBase.h"
#include "Interaction/A1WorldInteractable.h"
#include "A1DoorBase.generated.h"

class UArrowComponent;

UENUM(BlueprintType)
enum class EDoorState : uint8
{
	Open,
	Close
};

UCLASS()
class AA1DoorBase : public AA1WorldInteractable, public IA1SpaceshipComponent
{
	GENERATED_BODY()
public:
	AA1DoorBase(const FObjectInitializer& objectInitializer = FObjectInitializer::Get());

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
public:
	virtual FA1InteractionInfo GetPreInteractionInfo(const FA1InteractionQuery& InteractionQuery) const override;
	virtual void GetMeshComponents(TArray<UMeshComponent*>& OutMeshComponents) const override;

	//IA1SpaceshipComponent
	virtual void RegisterWithSpaceship(AA1SpaceshipBase* Spaceship) override;
	virtual ESpaceshipComponentType GetComponentType() const override { return ESpaceshipComponentType::Door; }

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void SetDoorState(EDoorState NewDoorState);

protected:

	void SetupTags();

	UFUNCTION(BlueprintImplementableEvent)
	void OnDoorStateChanged(EDoorState NewDoorState);

	UFUNCTION(BlueprintCallable)
	AA1SpaceshipBase* FindSpaceshipOwner() const;
private:
	UFUNCTION()
	void OnRep_DoorState();
public:
	EDoorState GetDoorState() const { return DoorState; }

	FORCEINLINE	AA1SpaceshipBase* GetOwningSpaceship() const { return OwningSpaceship.IsValid() ? OwningSpaceship.Get() : nullptr; }

protected:
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_DoorState)
	EDoorState DoorState = EDoorState::Close;

	UPROPERTY(EditDefaultsOnly, Category = "Info")
	FA1InteractionInfo OpenedInteractionInfo;

	UPROPERTY(EditDefaultsOnly, Category = "Info")
	FA1InteractionInfo ClosedInteractionInfo;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UArrowComponent> ArrowComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, Transient)
	TWeakObjectPtr<AA1SpaceshipBase> OwningSpaceship;
};
