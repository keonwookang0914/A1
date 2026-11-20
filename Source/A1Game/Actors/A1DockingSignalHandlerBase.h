// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "A1SpaceshipBase.h"
#include "Interaction/A1WorldInteractable.h"
#include "A1DockingSignalHandlerBase.generated.h"

class ALevelSequenceActor;
class ULevelSequencePlayer;
class ULevelSequence;
class UArrowComponent;

UENUM(BlueprintType)
enum class ESignalState : uint8
{
	Pressed,
	Released
};

UCLASS()
class AA1DockingSignalHandlerBase : public AA1WorldInteractable, public IA1SpaceshipComponent
{
	GENERATED_BODY()
public:
	AA1DockingSignalHandlerBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
public:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

public:
	virtual FA1InteractionInfo GetPreInteractionInfo(const FA1InteractionQuery& InteractionQuery) const override;
	virtual void GetMeshComponents(TArray<UMeshComponent*>& OutMeshComponents) const override;

	//IA1SpaceshipComponent
	virtual void RegisterWithSpaceship(class AA1SpaceshipBase* Spaceship) override;
	virtual ESpaceshipComponentType GetComponentType() const override { return ESpaceshipComponentType::DockingSignalHandler; }

public:
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void SetSignalState(ESignalState NewSignalState);

protected:

	void SetupTags();
	UFUNCTION(BlueprintImplementableEvent)
	void OnSignalStateChanged(ESignalState NewSignalState);

	UFUNCTION(BlueprintCallable)
	AA1SpaceshipBase* FindSpaceshipOwner() const;

private:
	UFUNCTION()
	void OnRep_SignalState();

public:
	ESignalState GetSignalState() const { return SignalState; }

	AA1SpaceshipBase* GetOwningSpaceship() const { return OwningSpaceship.Get(); }

protected:
	UPROPERTY(BlueprintReadWrite, ReplicatedUsing = OnRep_SignalState)
	ESignalState SignalState = ESignalState::Released;

	UPROPERTY(EditDefaultsOnly, Category = "Info")
	FA1InteractionInfo PressedInteractionInfo;

	UPROPERTY(EditDefaultsOnly, Category = "Info")
	FA1InteractionInfo ReleasedInteractionInfo;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UArrowComponent> ArrowComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, Transient)
	TWeakObjectPtr<AA1SpaceshipBase> OwningSpaceship;

};
