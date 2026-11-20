// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "A1RaiderRoom.h"
#include "GameFramework/Actor.h"
#include "A1RandomMapGenerator.generated.h"

class AA1DayNightManager;

USTRUCT()
struct FSpawnQueue
{
    GENERATED_BODY()

    enum ESpawnType
    {
        Enemy,
        Item,
        Cliff
    };

    ESpawnType Type;

    UPROPERTY()
    TObjectPtr<AA1RaiderRoom> RaiderRoom;

    FSpawnQueue()
    {
        Type = Enemy;
        RaiderRoom = nullptr;
    }
	FSpawnQueue(ESpawnType InType, AA1RaiderRoom* InRoom) : Type(InType), RaiderRoom(InRoom) { }
};

class AA1RoomBridge;
DECLARE_LOG_CATEGORY_EXTERN(LogMap, Log, All);

// Network Log Macro
#define LOG_NETMODEINFO ((GetNetMode() == ENetMode::NM_Client) ? *FString::Printf(TEXT("CLIENT_ID%d"), static_cast<int32>(GPlayInEditorID)) : ((GetNetMode() == ENetMode::NM_Standalone) ? TEXT("STANDALONE") : TEXT("SERVER")))
#define LOG_CALLINFO2 ANSI_TO_TCHAR(__FUNCTION__)
#define MAP_LOG(LogCat, Verbosity, Format, ...) UE_LOG(LogCat, Verbosity, TEXT("[%s] %s %s"), LOG_NETMODEINFO, LOG_CALLINFO2, *FString::Printf(Format,##__VA_ARGS__))

//forward declare
class AA1MasterRoom;
class AA1EndWall;

UCLASS()
class A1GAME_API AA1RandomMapGenerator : public AActor
{
    GENERATED_BODY()

public:
    AA1RandomMapGenerator();

protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;
public:
    virtual void Tick(float DeltaSeconds) override;
    //Call by GA_Interaction_DockingSignalHandler
    UFUNCTION(BlueprintImplementableEvent)
    void StartRandomMap();

    UFUNCTION(BlueprintCallable, Server, Reliable)
    void Server_SetSeed();

    UFUNCTION(Server, Reliable)
    void Server_SpawnStartRoom();

    UFUNCTION(Server, Reliable)
    void Server_StartDungeonTimer();

    UFUNCTION(Server, Reliable)
    void Server_SpawnNextRoom();

    UFUNCTION(Server, Reliable)
    void Server_CheckDungeonComplete();

    UFUNCTION(Server, Reliable)
    void Server_AddOverlappingRoomsFromList();

    UFUNCTION(Server, Reliable)
    void Server_CheckForOverlap();

    UFUNCTION(Server,Reliable)
    void Server_ResetMap();

    UFUNCTION(Server, Reliable)
    void Server_ResetAndRegenerateMap();

    UFUNCTION(Server, Reliable)
    void Server_CloseHoles();

    UFUNCTION(Server, Reliable)
    void Server_EnableReplicationForAllActors();

    UFUNCTION(Server, Reliable)
    void Server_SpawnEnemy();

    UFUNCTION(Server, Reliable)
    void Server_SpawnItem();

    UFUNCTION(Server, Reliable)
    void Server_MakeCliff();

    //RPC Functions

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_PlaySiren();

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_StopSiren();

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_CloseHoles();

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_OnDungeonGenerateComplete();

    //Client Verify functions
    UFUNCTION(Client, Reliable)
    void Client_VerifyMapGeneration();

    UFUNCTION(Server, Reliable)
    void Server_CheckAndRepairRooms();

    UFUNCTION(Client, Reliable)
    void Client_RepairInvalidRoom(int32 RoomIndex, FTransform RoomTransform, TSubclassOf<AA1MasterRoom> RoomClass);

    UFUNCTION(Client, Reliable)
    void Client_RepairInvalidWall(int32 WallIndex, FTransform WallTransform);

    uint8 GetbDungeonGenerateComplete() const { return bDungeonGenerateComplete; }

    //show in RenderScene
    UFUNCTION(BlueprintCallable)
    void IsShowFirstFloor(bool bIsShow);

    UFUNCTION(BlueprintCallable)
    void IsShowSecondFloor(bool bIsShow);

protected:
    void SetupNetworkProperties(AActor* Actor);

private:
    void AddEnemyToQueue(AA1RaiderRoom* Room);
    void AddItemToQueue(AA1RaiderRoom* Room);
    void AddCliffToQueue(AA1RaiderRoom* Room);
    void ProcessSpawnQueue();

protected:

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Generator", meta = (AllowPrivateAccess = "true"))
    TArray<TObjectPtr<USceneComponent>> ExitsList;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator")
    TArray<TSubclassOf<AA1MasterRoom>> RoomList;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Generator", meta = (AllowPrivateAccess = "true"), Replicated)
    TObjectPtr<AA1MasterRoom> LatestRoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Generator", meta = (AllowPrivateAccess = "true"), Replicated)
    int32 RoomAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator", meta = (AllowPrivateAccess = "true"), Replicated)
    int32 MaxRoomAmount;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Generator", meta = (AllowPrivateAccess = "true"))
    TArray<TObjectPtr<class UPrimitiveComponent>> OverlappedList;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Generator", meta = (AllowPrivateAccess = "true"), ReplicatedUsing = OnRep_DungeonGenerateComplete)
    uint8 bDungeonGenerateComplete : 1;

    UFUNCTION()
    void OnRep_DungeonGenerateComplete();

    UPROPERTY(EditDefaultsOnly, Category = "Generator|StartRoom")
    TSubclassOf<AA1RoomBridge> DockingBridge;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator", meta = (AllowPrivateAccess = "true"))
    float MaxDungeonTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generator", meta = (AllowPrivateAccess = "true"), Replicated)
    int32 Seed;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Generator", meta = (AllowPrivateAccess = "true"), Replicated)
    FRandomStream Stream;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Generator", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<USceneComponent> SelectedExitPoint;

    //For Replication
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Generator", Replicated)
    TArray<TObjectPtr<AA1MasterRoom>> SpawnedRooms;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Generator", Replicated)
    TArray<TObjectPtr<class AA1EndWall>> SpawnedEndWalls;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Generator")
    TArray<TObjectPtr<AA1MasterRoom>> FirstFloorRooms;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Generator")
    TArray<TObjectPtr<AA1MasterRoom>> SecondFloorRooms;

    //For Recovery
    UPROPERTY(Replicated)
    TArray<FTransform> RoomTransforms;

    UPROPERTY(Replicated)
    TArray<FString> RoomClassPaths;

    UPROPERTY(Replicated)
    TArray<FTransform> WallTransforms;

    //Check Server Make Random Map
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Generator", meta = (AllowPrivateAccess = "true"))
    uint8 bIsServerTraveling : 1;

    //Map Reset Flag
    UPROPERTY(Replicated)
    uint8 bIsResettingMap : 1;

    UPROPERTY(Replicated)
    int32 ExpectedExitCount;

    UPROPERTY(Replicated)
    int32 ExpectedEndWallCount;

    //Verify Check Flag
    UPROPERTY(Replicated)
    uint8 bVerificationComplete : 1;

    UPROPERTY(EditDefaultsOnly, Category = "SFX")
    TObjectPtr<USoundBase> SirenSound;

    UPROPERTY()
    TObjectPtr<UAudioComponent> AudioComp;

    UPROPERTY(EditDefaultsOnly, Category = "Generator")
    TSubclassOf<AA1EndWall> EndWallClass;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<AA1DayNightManager> DayNightManager;

    UPROPERTY(EditDefaultsOnly)
    TSubclassOf<AActor> PlundererSpawnClass;

    //Enemy
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Map|Enemy")
    TSubclassOf<AA1CreatureBase> RaiderClass;

private:
    FTimerHandle GenerateTimer;
    FTimerHandle RepairTimer;

    float StartTimer;

    int32 RepairAttempts;
    static const int32 MaxRepairAttempts = 3;

	TQueue<FSpawnQueue> SpawnQueue;

    // Restrict SpawnQueue Amount
    UPROPERTY(EditAnywhere, Category = "Spawn Optimization")
    int32 MinSpawnPerTick = 1;

    UPROPERTY(EditAnywhere, Category = "Spawn Optimization")
    int32 MaxSpawnPerTick = 5;

    // Check Frame Time
    float LastFrameTime = 0.0f;
    float TargetFrameTime = 0.016f;

    // Spawn Term
    UPROPERTY(EditAnywhere, Category = "Spawn Optimization")
    float SpawnInterval = 0.1f;

    float SpawnTimer = 0.0f;

    // Spawn Per Tick Count
    UPROPERTY(EditAnywhere, Category = "Spawn")
    int32 SpawnPerTick = 1;
};
