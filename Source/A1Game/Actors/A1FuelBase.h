// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "A1SpaceshipBase.h"
#include "Interaction/A1WorldInteractable.h"
#include "A1FuelBase.generated.h"

class AA1FuelDisplayUI;
class UArrowComponent;
class UNiagaraSystem;
class AA1SpaceshipBase;
/**
 *
 */
UCLASS()
class AA1FuelBase : public AA1WorldInteractable, public IA1SpaceshipComponent
{
    GENERATED_BODY()
public:
    AA1FuelBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
    virtual void BeginPlay() override;

    virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

public:
    virtual FA1InteractionInfo GetPreInteractionInfo(const FA1InteractionQuery& InteractionQuery) const override;
    virtual void GetMeshComponents(TArray<UMeshComponent*>& OutMeshComponents) const override;

    //IA1SpaceshipComponent
    virtual void RegisterWithSpaceship(class AA1SpaceshipBase* Spaceship) override;
    virtual ESpaceshipComponentType GetComponentType() const override { return ESpaceshipComponentType::Fuel; }

    UFUNCTION(BlueprintCallable)
    bool IsFuelItem(AActor* Item) const;

    UFUNCTION(BlueprintCallable)
    void DetectAndAbsorbFuelItems();

    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
    void AddFuel(float FuelAmount);

protected:
    void SetupTags();

    UFUNCTION()
    AA1SpaceshipBase* FindSpaceshipOwner() const;

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Info")
    FA1InteractionInfo InteractionInfo;

    UPROPERTY(EditDefaultsOnly, Category = "Fuel")
    float DetectionRadius = 500.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UArrowComponent> ArrowComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UStaticMeshComponent> MeshComponent;

    UPROPERTY(VisibleAnywhere, Transient)
    TWeakObjectPtr<AA1SpaceshipBase> OwningSpaceship;

    UPROPERTY(VisibleAnywhere)
    TArray<TWeakObjectPtr<AActor>> DetectedFuelItems;

    UPROPERTY(EditDefaultsOnly)
    TSubclassOf<AA1FuelDisplayUI> FuelUI;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<AA1FuelDisplayUI> SpawnedFuelUI;

    FVector UILocation;

    FRotator UIRotation;
};
