// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "A1SpaceshipBase.h"
#include "Interaction/A1WorldInteractable.h"
#include "A1ShipOutputBase.generated.h"

class UArrowComponent;
class AA1SpaceshipBase;

UENUM(BlueprintType)
enum class EOutputState : uint8
{
    Idle,
    Ready,
    Active
};

/**
 * Dock out when docked
 */
UCLASS()
class AA1ShipOutputBase : public AA1WorldInteractable, public IA1SpaceshipComponent
{
    GENERATED_BODY()
public:
    AA1ShipOutputBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    virtual void BeginPlay() override;

protected:
    virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

public:
    virtual FA1InteractionInfo GetPreInteractionInfo(const FA1InteractionQuery& InteractionQuery) const override;
    virtual void GetMeshComponents(TArray<UMeshComponent*>& OutMeshComponents) const override;

    //IA1SpaceshipComponent
    virtual void RegisterWithSpaceship(class AA1SpaceshipBase* Spaceship) override;
    virtual ESpaceshipComponentType GetComponentType() const override { return ESpaceshipComponentType::ShipOutput; }

    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
    void SetOutputState(EOutputState NewOutputState);

    UFUNCTION(BlueprintCallable)
    void DeactivateExternalMap();

    void PlayLaunchSound();

    

protected:
    void SetupTags();
    UFUNCTION(BlueprintImplementableEvent)
    void OnOutputStateChanged(EOutputState NewOutputState);

private:
    UFUNCTION()
    void OnRep_OutputState();

    UFUNCTION(BlueprintCallable)
    AA1SpaceshipBase* FindSpaceshipOwner() const;

protected:
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_OutputState)
    EOutputState OutputState = EOutputState::Idle;

    UPROPERTY(EditDefaultsOnly, Category = "Info")
    FA1InteractionInfo IdleInteractionInfo;

    UPROPERTY(EditDefaultsOnly, Category = "Info")
    FA1InteractionInfo ReadyInteractionInfo;

    UPROPERTY(EditDefaultsOnly, Category = "Info")
    FA1InteractionInfo ActiveInteractionInfo;

    UPROPERTY(EditDefaultsOnly, Category = "Output")
    float FuelCostToDeactivateMap = 500.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UArrowComponent> ArrowComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UStaticMeshComponent> MeshComponent;

    UPROPERTY(VisibleAnywhere, Transient)
    TWeakObjectPtr<AA1SpaceshipBase> OwningSpaceship;

    UPROPERTY(EditDefaultsOnly, Category = "Info")
    TObjectPtr<USoundBase> LaunchSound;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UAudioComponent> AudioComp;
};
