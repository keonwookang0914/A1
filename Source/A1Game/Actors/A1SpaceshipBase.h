// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "Score/A1ScoreData.h"
#include "A1SpaceshipBase.generated.h"

class AA1RepairBase;
class AA1SignalDetectionBase;
class AA1FuelDisplayUI;
class AA1StorageBase;
class AA1ShipOutputBase;
class AA1FuelBase;
class AA1BedBase;
class AA1DockingSignalHandlerBase;
class AA1DoorBase;

UENUM(BlueprintType)
enum class ESpaceshipComponentType : uint8
{
    Door,
    DockingSignalHandler,
    Bed,
    Fuel,
    Storage,
    ShipOutput,
    SignalDetection
};

UENUM(BlueprintType)
enum class EGameEndState : uint8
{
    None,
    GameOver,
    Rescued
};

//SpaceshipInterface
UINTERFACE(MinimalAPI, BlueprintType)
class UA1SpaceshipComponent : public UInterface
{
    GENERATED_BODY()
};

class IA1SpaceshipComponent
{
    GENERATED_BODY()

public:
    virtual void RegisterWithSpaceship(class AA1SpaceshipBase* Spaceship) = 0;
    virtual ESpaceshipComponentType GetComponentType() const = 0;
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameEndEvent, EGameEndState, EndState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFuelChanged, float, NewFuelAmount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeactivateMap);

UCLASS()
class AA1SpaceshipBase : public AActor
{
    GENERATED_BODY()
public:
    AA1SpaceshipBase();

protected:
    virtual void BeginPlay() override;
public:
    virtual void Tick(float DeltaTime) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION(BlueprintCallable, Category = "Spaceship|Components")
    void RegisterDoor(AA1DoorBase* Door);

    UFUNCTION(BlueprintCallable, Category = "Spaceship|Components")
    void RegisterDockingSignalHandler(AA1DockingSignalHandlerBase* Signal);

    UFUNCTION(BlueprintCallable, Category = "Spaceship|Components")
    void RegisterBed(AA1BedBase* Bed);

    UFUNCTION(BlueprintCallable, Category = "Spaceship|Components")
    void RegisterFuelSystem(AA1FuelBase* Fuel);

    UFUNCTION(BlueprintCallable, Category = "Spaceship|Components")
    void RegisterStorage(AA1StorageBase* Storage);

    UFUNCTION(BlueprintCallable, Category = "Spaceship|Components")
    void RegisterShipOutput(AA1ShipOutputBase* Output);

    UFUNCTION(BlueprintCallable, Category = "Spaceship|Components")
    void RegisterSignalDetection(AA1SignalDetectionBase* Output);


    UFUNCTION(BlueprintCallable, Category = "Spaceship|GameState", BlueprintAuthorityOnly)
    void HandleGameOver();

    
    UFUNCTION(BlueprintCallable, Category = "Spaceship|GameState", BlueprintAuthorityOnly)
    void HandleRescue();

    
    UFUNCTION(BlueprintPure, Category = "Spaceship|GameState")
    bool IsRescued() const;

    
    UFUNCTION(BlueprintPure, Category = "Spaceship|GameState")
    EGameEndState GetGameEndState() const { return GameEndState; }

    
    UFUNCTION(BlueprintPure, Category = "Spaceship|GameState")
    bool IsGameOver() const;

    
    UFUNCTION(BlueprintCallable, Category = "Spaceship|Fuel")
    float GetCurrentFuelAmount() const { return CurrentFuelAmount; }

    UFUNCTION(BlueprintCallable, Category = "Spaceship|Fuel")
    float GetMaxFuelAmount() const { return MaxFuelAmount; }

    UFUNCTION(BlueprintCallable, Category = "Spaceship|Fuel")
    float GetFuelPercentage() const { return (MaxFuelAmount > 0.0f) ? (CurrentFuelAmount / MaxFuelAmount) : 0.0f; }

    UFUNCTION(BlueprintCallable, Category = "Spaceship|Fuel", BlueprintAuthorityOnly)
    void AddFuel(float AmountToAdd);

    UFUNCTION(BlueprintCallable, Category = "Spaceship|Fuel", BlueprintAuthorityOnly)
    void ConsumeFuel(float AmountToConsume);

    UFUNCTION(BlueprintCallable, Category = "Spaceship|Fuel", BlueprintAuthorityOnly)
    void ConsumeDefaultFuel();

    
    UFUNCTION(BlueprintCallable, Category = "Spaceship|ExternalMap", BlueprintAuthorityOnly)
    void ActivateExternalMap();

    UFUNCTION(BlueprintCallable, Category = "Spaceship|ExternalMap", BlueprintAuthorityOnly)
    void DeactivateExternalMap();

    UFUNCTION(BlueprintPure, Category = "Spaceship|ExternalMap")
    bool IsExternalMapActive() const { return bIsExternalMapActive; }

    
    UFUNCTION(BlueprintCallable, Category = "Spaceship|GameState")
    bool HasEnoughFuelToSurvive() const;

    UFUNCTION()
    void OnStopFuelConsume(const FA1ScoreData& FinalScore);

    UFUNCTION()
    void OnRep_CurrentFuel();

    UFUNCTION()
    void OnRep_GameEndState();

    void FindAllRepairBases();

    FORCEINLINE bool GetIsExternalMapActive() const { return bIsExternalMapActive; }
    void SetIsExternamMapActive(bool InExternalMapActive);
    FORCEINLINE AA1DoorBase* GetCachedDoor() const { return CacheDoor; }
    FORCEINLINE AA1SignalDetectionBase* GetSignalDetection() const { return SignalDetection; }
    FORCEINLINE bool GetCanUseDockingSignalHandler() const { return bCanUseDockingSignalHandler; }
    FORCEINLINE void SetCanUseDockingSignalHandler(bool InDockingSingalHandler) { bCanUseDockingSignalHandler = InDockingSingalHandler; }
    FORCEINLINE bool GetbTutorial() const { return bTutorial; }
    FORCEINLINE float GetFuelConsumeAmount() const {return CurrentFuelConsumeAmount;}
    FORCEINLINE void SetFuelConsumeAmount(float InCurrentFuelConsumeAmount) { CurrentFuelConsumeAmount = InCurrentFuelConsumeAmount; }
	FORCEINLINE AA1RepairBase* GetPipeRepairBase() const { return SpecificRepairBase; }

    UFUNCTION(BlueprintCallable, Category = "Spaceship|GameState", BlueprintAuthorityOnly)
    void SetMeetRescueShip(bool bMeetRescue);
    UFUNCTION()
    void CheckTwoDaysAgo(int32 NewDay);

	void BreakFoam();

	void BreakPipeRepairBase();

protected:
    UFUNCTION(BlueprintCallable, Category = "Spaceship|Components")
    void FindSpaceshipComponents();

    UFUNCTION(BlueprintCallable, Category = "Spaceship|Components")
    void FindComponentsByTags();

private:
	void SpawnOneRepairBaseByTutoMode();

public:
    //Fuel Change Delegate
    UPROPERTY(BlueprintAssignable, Category = "Spaceship|Fuel")
    FOnFuelChanged OnFuelChanged;

    UPROPERTY(BlueprintAssignable, Category = "Spaceship|GameState")
    FOnGameEndEvent OnGameEndEvent;

    UPROPERTY(BlueprintAssignable, Category = "Spaceship|DeactivateSpace")
    FOnDeactivateMap OnDeactivateMap;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spaceship | RepairBase")
	TSubclassOf<AA1RepairBase> DefaultRepairBaseClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spaceship | RepairBase")
	TSubclassOf<AA1RepairBase> PipeRepairBase;

	UPROPERTY()
	TObjectPtr<AA1RepairBase> SpecificRepairBase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spaceship|Fuel")
    float MaxFuelAmount = 200000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spaceship|Fuel", ReplicatedUsing = OnRep_CurrentFuel)
    float CurrentFuelAmount = 5000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spaceship|Fuel")
    float FuelConsumptionRate = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spaceship|Interactables")
    TObjectPtr<AA1DoorBase> CacheDoor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spaceship|Interactables")
    TObjectPtr<AA1DockingSignalHandlerBase> DockingSignalHandler;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spaceship|Interactables")
    TArray<TObjectPtr<AA1BedBase>> Beds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spaceship|Interactables")
    TObjectPtr<AA1FuelBase> FuelSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spaceship|Interactables")
    TArray<TObjectPtr<AA1StorageBase>> Storages;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spaceship|Interactables")
    TObjectPtr<AA1ShipOutputBase> ShipOutput;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spaceship|Interactables")
    TObjectPtr<AA1SignalDetectionBase> SignalDetection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spaceship|ExternalMap", Replicated)
    bool bIsExternalMapActive = false;

    UPROPERTY(BlueprintReadOnly, Category = "Spaceship|GameState", ReplicatedUsing = OnRep_GameEndState)
    EGameEndState GameEndState = EGameEndState::None;

    UPROPERTY(EditDefaultsOnly, Category = "Spaceship|Tags")
    FName SpaceshipComponentTag = "SpaceshipComponent";

    UPROPERTY(EditDefaultsOnly, Category = "Spaceship|Tags")
    FName DoorTag = "Door";

    UPROPERTY(EditDefaultsOnly, Category = "Spaceship|Tags")
    FName DockingSignalHandlerTag = "DockingSignalHandler";

    UPROPERTY(EditDefaultsOnly, Category = "Spaceship|Tags")
    FName BedTag = "Bed";

    UPROPERTY(EditDefaultsOnly, Category = "Spaceship|Tags")
    FName FuelSystemTag = "FuelSystem";

    UPROPERTY(EditDefaultsOnly, Category = "Spaceship|Tags")
    FName FuelDisplayTag = "FuelDisplay";

    UPROPERTY(EditDefaultsOnly, Category = "Spaceship|Tags")
    FName StorageTag = "Storage";

    UPROPERTY(EditDefaultsOnly, Category = "Spaceship|Tags")
    FName ShipOutputTag = "ShipOutput";

    UPROPERTY(EditDefaultsOnly, Category = "Spaceship|Tags")
    FName SignalDetectionTag = "SignalDetection";

    UPROPERTY(Replicated)
    bool bMeetRescueShip = false;

    UPROPERTY(Replicated)
    bool bCanUseDockingSignalHandler = false;

    UPROPERTY(VisibleAnywhere, Category = "Spaceship|RepairBase")
    TArray<AA1RepairBase*> CachedRepairs;

    UPROPERTY(VisibleAnywhere, Category = "Spaceship|RepairBase")
    TArray<AA1RepairBase*> CachedNonBrokenRepairs;

    UPROPERTY()
    int32 CurrentDay = 1;

    UPROPERTY()
    float CurrentFuelConsumeAmount = 1.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spaceship|Tutorial")
    bool bTutorial = false;

private:
    FTimerHandle FuelConsumeTimer;

    bool bGameEndHandled = false;
};
