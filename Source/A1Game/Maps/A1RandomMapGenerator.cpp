// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#include "Maps/A1RandomMapGenerator.h"

#include "A1EndWall.h"
#include "A1RoomBridge.h"
#include "A1MasterRoom.h"
#include "A1RaiderRoom.h"
#include "Actors/A1DayNightManager.h"
#include "Actors/A1SpaceshipBase.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY(LogMap);

// 기본값 설정
AA1RandomMapGenerator::AA1RandomMapGenerator()
{
    PrimaryActorTick.bCanEverTick = true;

    // 네트워크 설정 최적화
    bReplicates = true;
    bAlwaysRelevant = true;
    bNetLoadOnClient = true;
    NetUpdateFrequency = 60.0f;
    MinNetUpdateFrequency = 30.0f;
    NetPriority = 3.0f;

    // 초기값 설정
    bDungeonGenerateComplete = false;
    bIsResettingMap = false;
    bVerificationComplete = false;
    Seed = -1;                       // -1: Random Seed, > 0 : Fixed Seed
    MaxDungeonTime = 10.f;
    MaxRoomAmount = 30;             //default value
    RoomAmount = MaxRoomAmount;     //default Value
    SelectedExitPoint = nullptr;
    bIsServerTraveling = false;

    RepairAttempts = 0;
    ExpectedExitCount = 0;
    ExpectedEndWallCount = 0;
}

void AA1RandomMapGenerator::BeginPlay()
{
    Super::BeginPlay();

    SetReplicateMovement(true);

    AActor* Actor = UGameplayStatics::GetActorOfClass(GetWorld(), AA1DayNightManager::StaticClass());
    if (Actor)
    {
	    if (AA1DayNightManager* DayNight = Cast<AA1DayNightManager>(Actor))
	    {
            DayNightManager = DayNight;
	    }
    }

    // Start Generate map Only in Server
    /*if (HasAuthority())
    {
        Server_SetSeed();
    }*/
}

void AA1RandomMapGenerator::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // Replicated Variable
    DOREPLIFETIME_CONDITION(AA1RandomMapGenerator, Seed, COND_InitialOnly);
    DOREPLIFETIME_CONDITION(AA1RandomMapGenerator, Stream, COND_InitialOnly);
    DOREPLIFETIME(AA1RandomMapGenerator, LatestRoom);
    DOREPLIFETIME(AA1RandomMapGenerator, RoomAmount);
    DOREPLIFETIME(AA1RandomMapGenerator, MaxRoomAmount);
    DOREPLIFETIME(AA1RandomMapGenerator, bDungeonGenerateComplete);
    DOREPLIFETIME(AA1RandomMapGenerator, SpawnedRooms);
    DOREPLIFETIME(AA1RandomMapGenerator, SpawnedEndWalls);
    DOREPLIFETIME(AA1RandomMapGenerator, bIsResettingMap);
    DOREPLIFETIME(AA1RandomMapGenerator, ExpectedExitCount);
    DOREPLIFETIME(AA1RandomMapGenerator, ExpectedEndWallCount);
    DOREPLIFETIME(AA1RandomMapGenerator, RoomTransforms);
    DOREPLIFETIME(AA1RandomMapGenerator, RoomClassPaths);
    DOREPLIFETIME(AA1RandomMapGenerator, WallTransforms);
    DOREPLIFETIME(AA1RandomMapGenerator, bVerificationComplete);
}

void AA1RandomMapGenerator::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
    Super::PreReplication(ChangedPropertyTracker);

    // Upgrade Replication Priority
    if (SpawnedRooms.Num() > 0 || SpawnedEndWalls.Num() > 0)
    {
        DOREPLIFETIME_ACTIVE_OVERRIDE(AA1RandomMapGenerator, SpawnedRooms, true);
        DOREPLIFETIME_ACTIVE_OVERRIDE(AA1RandomMapGenerator, SpawnedEndWalls, true);
        DOREPLIFETIME_ACTIVE_OVERRIDE(AA1RandomMapGenerator, RoomTransforms, true);
        DOREPLIFETIME_ACTIVE_OVERRIDE(AA1RandomMapGenerator, WallTransforms, true);
    }
}

void AA1RandomMapGenerator::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

    if (!HasAuthority())
        return;

    // Check Spawn Term
    SpawnTimer += DeltaSeconds;
    if (SpawnTimer < SpawnInterval)
        return;

    SpawnTimer = 0.0f;

    float CurrentFrameTime = FApp::GetDeltaTime();
    if (CurrentFrameTime > TargetFrameTime * 1.5f) // 40 FPS
    {
        SpawnPerTick = MinSpawnPerTick;
    }
    else if (CurrentFrameTime < TargetFrameTime) //60 FPS
    {
        SpawnPerTick = MaxSpawnPerTick;
    }

    ProcessSpawnQueue();
}

void AA1RandomMapGenerator::Multicast_PlaySiren_Implementation()
{
    if (!HasAuthority())
    {
        AudioComp = UGameplayStatics::SpawnSoundAtLocation(GetWorld(), SirenSound, GetActorLocation(), GetActorRotation());
    }
}

void AA1RandomMapGenerator::Multicast_StopSiren_Implementation()
{
    if (!HasAuthority() && AudioComp)
    {
        AudioComp->Stop();
    }

}

void AA1RandomMapGenerator::Server_SpawnEnemy_Implementation()
{
    if (!HasAuthority())
        return;

    MAP_LOG(LogMap, Log, TEXT("SpawnedRooms Count: %d"), SpawnedRooms.Num());

    for (auto Room : SpawnedRooms)
    {
	    if (AA1RaiderRoom* RaiderRoom = Cast<AA1RaiderRoom>(Room))
	    {
            AddEnemyToQueue(RaiderRoom);
	    }
    }
}

void AA1RandomMapGenerator::Server_SpawnItem_Implementation()
{
    if (!HasAuthority())
        return;

    MAP_LOG(LogMap, Log, TEXT("SpawnedRooms Count: %d"), SpawnedRooms.Num());

    for (auto Room : SpawnedRooms)
    {
        if (AA1RaiderRoom* RaiderRoom = Cast<AA1RaiderRoom>(Room))
        {
            AddItemToQueue(RaiderRoom);
        }
    }
}

void AA1RandomMapGenerator::AddEnemyToQueue(AA1RaiderRoom* Room)
{
    SpawnQueue.Enqueue(FSpawnQueue(FSpawnQueue::Enemy, Room));
}

void AA1RandomMapGenerator::AddItemToQueue(AA1RaiderRoom* Room)
{
    SpawnQueue.Enqueue(FSpawnQueue(FSpawnQueue::Item, Room));
}

void AA1RandomMapGenerator::AddCliffToQueue(AA1RaiderRoom* Room)
{
    SpawnQueue.Enqueue(FSpawnQueue(FSpawnQueue::Cliff, Room));
}

void AA1RandomMapGenerator::ProcessSpawnQueue()
{
    if (!HasAuthority())
        return;

    int32 SpawnedThisTick = 0;
    FSpawnQueue Queue;

    // Spawn Something SpawnPerTick Amount
    while (SpawnedThisTick < SpawnPerTick && SpawnQueue.Dequeue(Queue))
    {
        switch (Queue.Type)
        {
        case FSpawnQueue::Enemy:
            Queue.RaiderRoom->SpawnEnemies(RaiderClass);
            break;
        case FSpawnQueue::Item:
            Queue.RaiderRoom->SpawnItems();
            break;
        case FSpawnQueue::Cliff:
            Queue.RaiderRoom->CreateCliffHole();
            break;
        }

        SpawnedThisTick++;
    }
}

void AA1RandomMapGenerator::Server_MakeCliff_Implementation()
{
    //Make Cliff
    for (auto Room : SpawnedRooms)
    {
        if (Room->GetActorLocation().Z < 1000.f)
        {
            if (AA1RaiderRoom* RaiderRoom = Cast<AA1RaiderRoom>(Room))
            {
                if (RaiderRoom->GetCanMakeCliff())
                {
                    AddCliffToQueue(RaiderRoom);
                }
            }
        }
    }
}

void AA1RandomMapGenerator::IsShowFirstFloor(bool bIsShow)
{
    for (auto FirstFloor : FirstFloorRooms)
    {
        if(UStaticMeshComponent* Mesh = Cast<UStaticMeshComponent>(FirstFloor->GetGeometryFolder()->GetChildComponent(0)))
        {
            Mesh->SetHiddenInSceneCapture(!bIsShow);
        }
    }
}

void AA1RandomMapGenerator::IsShowSecondFloor(bool bIsShow)
{
    for (auto FirstFloor : SecondFloorRooms)
    {
        if (UStaticMeshComponent* Mesh = Cast<UStaticMeshComponent>(FirstFloor->GetGeometryFolder()->GetChildComponent(0)))
        {
            Mesh->SetHiddenInSceneCapture(!bIsShow);
        }
    }
}

void AA1RandomMapGenerator::SetupNetworkProperties(AActor* Actor)
{
    if (!Actor)
        return;

    Actor->SetReplicates(true);
    Actor->SetReplicateMovement(true);
    Actor->SetActorTickEnabled(true);
    Actor->bAlwaysRelevant = true;
    Actor->NetUpdateFrequency = 60.0f;
    Actor->MinNetUpdateFrequency = 30.0f;
    Actor->NetPriority = 3.0f;
    Actor->SetNetDormancy(ENetDormancy::DORM_Awake);

    Actor->ForceNetUpdate();
}

void AA1RandomMapGenerator::Server_SetSeed_Implementation()
{
    if (!HasAuthority())
        return;

    StartTimer = GetWorld()->GetTimeSeconds();

    if (Seed == -1)
    {
        Seed = FMath::Rand32();
        Stream.Initialize(Seed);
    }
    else
    {
        Stream.Initialize(Seed);
    }

    //Set Room Number by Day
    int32 CurrentDay = DayNightManager->GetCurrentDay();
    float DayPercentage = static_cast<float>(CurrentDay) / 40;
    MaxRoomAmount = 10 + 20 * FMath::Min(DayPercentage, 1.f);
    RoomAmount = MaxRoomAmount;

    MAP_LOG(LogMap, Log, TEXT("Set Seed!: %d"), Stream.GetInitialSeed());

    Server_SpawnStartRoom();
}

void AA1RandomMapGenerator::Server_SpawnStartRoom_Implementation()
{
    // Only Server
    if (!HasAuthority())
        return;

    //Play Sound by NetMode
    if (GetNetMode() == NM_Standalone)
        AudioComp = UGameplayStatics::SpawnSoundAtLocation(GetWorld(), SirenSound, GetActorLocation(), GetActorRotation());
    else
        Multicast_PlaySiren();

    Multicast_PlaySiren();

    FTransform InitMapTransform = RootComponent->GetComponentTransform();

    // SpawnActorDeferred
    AA1RoomBridge* SpawnedActor = GetWorld()->SpawnActorDeferred<AA1RoomBridge>(
        DockingBridge,
        InitMapTransform,
        this,                  
        nullptr,               
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

    if (SpawnedActor)
    {
        // Set Network Setting
        SpawnedActor->SetReplicates(true);
        SpawnedActor->bAlwaysRelevant = true;
        SpawnedActor->NetUpdateFrequency = 60.0f;
        SpawnedActor->MinNetUpdateFrequency = 30.0f;
        SpawnedActor->NetPriority = 3.0f;

        //Done in Server
        UGameplayStatics::FinishSpawningActor(SpawnedActor, InitMapTransform);


        //Save in FirstFloor Array
        FirstFloorRooms.Add(SpawnedActor);

        // Set SpawnedActor Network Setting
        SetupNetworkProperties(SpawnedActor);

        MAP_LOG(LogMap, Log, TEXT("First Spawned Actor: %s"), *SpawnedActor->GetActorLocation().ToString());

        SpawnedRooms.Add(Cast<AA1MasterRoom>(SpawnedActor));
        RoomAmount--;

        RoomTransforms.Add(InitMapTransform);
        RoomClassPaths.Add(SpawnedActor->GetClass()->GetPathName());
    }

    //Add SpawnedActor's ExitsList to Global Variable ExitsList
    USceneComponent* TargetComponent = SpawnedActor->GetExitsFolder();
    TArray<USceneComponent*> TmpList;
    TargetComponent->GetChildrenComponents(false, TmpList);
    ExitsList.Append(TmpList);
	
    ExpectedExitCount = TmpList.Num();

    Server_StartDungeonTimer();
}

void AA1RandomMapGenerator::Server_StartDungeonTimer_Implementation()
{
    if (!HasAuthority())
        return;

    GetWorld()->GetTimerManager().SetTimer(GenerateTimer, this,
        &AA1RandomMapGenerator::Server_CheckDungeonComplete, 1.0f, true);

    Server_SpawnNextRoom();
}

void AA1RandomMapGenerator::Server_SpawnNextRoom_Implementation()
{
    if (!HasAuthority())
        return;

	//No Exit or Spawn Attemps
    if (ExitsList.Num() <= 0)
        return;
    

    if (RoomList.IsEmpty())
    {
        MAP_LOG(LogMap, Error, TEXT("RoomList is Empty."));
        return;
    }

    // Set Random Room Type and Exits
    int32 OutIndex = Stream.RandHelper(ExitsList.Num());
    USceneComponent* OutItem = ExitsList[OutIndex];
    SelectedExitPoint = OutItem;

    if (MaxRoomAmount == RoomAmount + 1)
    {
        OutIndex = 0; //Rounge
    }
    else if (MaxRoomAmount == RoomAmount + 9)
    {
        OutIndex = RoomList.Num() - 1; //Second Floor
    }
    else
    {
        OutIndex = Stream.RandHelper(RoomList.Num() - 1);
    }

    TSubclassOf<AA1MasterRoom> RoomClass = RoomList[OutIndex];

    if (!RoomClass)
    {
        MAP_LOG(LogMap, Error, TEXT("Index %d's Room Clas is Invalid."), OutIndex);
        return;
    }

    
    FTransform NextRoomTransform = SelectedExitPoint->GetComponentTransform();
    AA1MasterRoom* SpawnedActor = GetWorld()->SpawnActorDeferred<AA1MasterRoom>(
        RoomClass,
        NextRoomTransform,
        this,                  
        nullptr,               
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

    if (SpawnedActor)
    {
        SpawnedActor->SetReplicates(true);
        SpawnedActor->bAlwaysRelevant = true;
        SpawnedActor->NetUpdateFrequency = 60.0f;
        SpawnedActor->MinNetUpdateFrequency = 30.0f;
        SpawnedActor->NetPriority = 3.0f;
        if (AA1RaiderRoom* RaiderRoom = Cast<AA1RaiderRoom>(SpawnedActor))
        {
            RaiderRoom->SetRoomType(OutIndex);
        }


        UGameplayStatics::FinishSpawningActor(SpawnedActor, NextRoomTransform);

        //Save Rooms By Z location
        if (SpawnedActor->GetActorLocation().Z > 1000.f)
        {
            SecondFloorRooms.Add(SpawnedActor);
        }
        else
        {
            FirstFloorRooms.Add(SpawnedActor);

            if (OutIndex == RoomList.Num() - 1)
            {
                SecondFloorRooms.Add(SpawnedActor);
            }
        }

        SetupNetworkProperties(SpawnedActor);

        SpawnedRooms.Add(SpawnedActor);

        RoomTransforms.Add(NextRoomTransform);
        RoomClassPaths.Add(SpawnedActor->GetClass()->GetPathName());

        LatestRoom = SpawnedActor;

        FString printlog = NextRoomTransform.GetLocation().ToString();
        MAP_LOG(LogMap, Log, TEXT("Generate %d actor!: %s, name: %s"), MaxRoomAmount - RoomAmount,
            *printlog, *SpawnedActor->GetName());
    }

    if (MaxRoomAmount == RoomAmount + 1)
    {
        if(AA1RaiderRoom* Raider = Cast<AA1RaiderRoom>(SpawnedActor))
        {
            Raider->SpawnPlundererSpawner();
        }
    }

    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AA1RandomMapGenerator::Server_CheckForOverlap, 0.1f, false);
}

void AA1RandomMapGenerator::Server_CheckDungeonComplete_Implementation()
{
    if (!HasAuthority() || bIsServerTraveling)
        return;

    if (bDungeonGenerateComplete)
    {
        GetWorld()->GetTimerManager().ClearTimer(GenerateTimer);
        return;
    }

    float GameTimer = GetWorld()->GetTimeSeconds();
    if (GameTimer - StartTimer > MaxDungeonTime)
    {
        //Set Map Reset flag true
        bIsResettingMap = true;

        // Timeout Log
        UE_LOG(LogTemp, Warning, TEXT("===== Resetting Map and Restart Geneartor ====="));
        MAP_LOG(LogMap, Warning, TEXT("Timeout by %.2f seconds. Reset Map."), GameTimer - StartTimer);

        // Remove Timer
        GetWorld()->GetTimerManager().ClearTimer(GenerateTimer);

        Server_ResetAndRegenerateMap();
    }
}

void AA1RandomMapGenerator::Server_AddOverlappingRoomsFromList_Implementation()
{
    if (!HasAuthority())
        return;

    MAP_LOG(LogMap, Log, TEXT("Start AddOverlappingRoomsFromList"));

    auto OverlapFolder = LatestRoom->GetOverlapFolder();
    TArray<USceneComponent*> ChildrenComp;
    OverlapFolder->GetChildrenComponents(false, ChildrenComp);

    TArray<UPrimitiveComponent*> TmpList;
    for (auto child : ChildrenComp)
    {
        UBoxComponent* BoxComp = Cast<UBoxComponent>(child);
        if (BoxComp)
        {
            BoxComp->GetOverlappingComponents(TmpList);

            for (auto Tmp : TmpList)
            {
                // ignore owner
                if (Tmp->GetOwner() != LatestRoom)
                {
                    OverlappedList.Add(Tmp);
                }
            }
        }
    }
}

void AA1RandomMapGenerator::Server_CheckForOverlap_Implementation()
{
    if (!HasAuthority())
        return;

    MAP_LOG(LogMap, Log, TEXT("Start CheckForOverlap"));
    Server_AddOverlappingRoomsFromList();

    if (OverlappedList.IsEmpty())
    {
        MAP_LOG(LogMap, Log, TEXT("OverlappedList is Empty!"));
        
        OverlappedList.Reset();
        RoomAmount--;
        ExitsList.Remove(SelectedExitPoint);

        //Add ExitsList
        if (LatestRoom != nullptr)
        {
            auto LatestRoomArray = LatestRoom->GetExitsFolder();
            TArray<USceneComponent*> ChildrenComp;
            LatestRoomArray->GetChildrenComponents(false, ChildrenComp);

            ExitsList.Append(ChildrenComp);

            ExpectedExitCount += ChildrenComp.Num() - 1;
        }

        if (RoomAmount > 0)
        {
            Server_SpawnNextRoom();
        }
        else
        {
            FTimerHandle TimerHandle;
            GetWorldTimerManager().SetTimer(TimerHandle, this, &AA1RandomMapGenerator::Server_CloseHoles, 0.1f, false);
        }
    }
    else
    {
        //Overlapped List Log info
        for (auto& Component : OverlappedList)
        {
            if (Component)
            {
                MAP_LOG(LogMap, Warning, TEXT("%s Overlapped with: %s"),
                    *LatestRoom->GetName(), *Component->GetName());

                AActor* OwnerActor = Component->GetOwner();
                if (OwnerActor)
                {
                    MAP_LOG(LogMap, Warning, TEXT("  Owner: %s"), *OwnerActor->GetName());
                }
            }
        }

        // Remove Overlapped Room's ExitsList
        if (LatestRoom)
        {
            auto LatestRoomArray = LatestRoom->GetExitsFolder();
            if (LatestRoomArray)
            {
                TArray<USceneComponent*> RoomExits;
                LatestRoomArray->GetChildrenComponents(false, RoomExits);

                for (auto Exit : RoomExits)
                {
                    if (Exit)
                    {
                        ExitsList.Remove(Exit);
                        MAP_LOG(LogMap, Log, TEXT("Removed exit point from overlapped room"));
                    }
                }
            }
            //Remove Floor Rooms
            if (LatestRoom->GetActorLocation().Z > 1000.f)
            {
                int32 RoomIndex = SecondFloorRooms.Find(LatestRoom);
                SecondFloorRooms.RemoveAt(RoomIndex);
            }
            else
            {
                int32 RoomIndex = FirstFloorRooms.Find(LatestRoom);
                FirstFloorRooms.RemoveAt(RoomIndex);

                if (LatestRoom.GetName().Contains(TEXT("SecondFloor")))
                {
                    int32 RoomIndex2 = SecondFloorRooms.Find(LatestRoom);
                    SecondFloorRooms.RemoveAt(RoomIndex2);
                }
            }

            //Remove Room
            int32 RoomIndex = SpawnedRooms.Find(LatestRoom);
            if (RoomIndex != INDEX_NONE)
            {
                SpawnedRooms.RemoveAt(RoomIndex);

                // Remove Recovery Info
                if (RoomTransforms.IsValidIndex(RoomIndex))
                {
                    RoomTransforms.RemoveAt(RoomIndex);
                }
                if (RoomClassPaths.IsValidIndex(RoomIndex))
                {
                    RoomClassPaths.RemoveAt(RoomIndex);
                }
            }

            LatestRoom->Destroy();
            LatestRoom = nullptr;
        }

        
        OverlappedList.Reset();

        //Retry
        Server_SpawnNextRoom();
    }
}

void AA1RandomMapGenerator::Server_ResetMap_Implementation()
{
    //Reset All Map generated.
    if (!HasAuthority())
        return;

    FirstFloorRooms.Reset();
    SecondFloorRooms.Reset();

    for (auto Room : SpawnedRooms)
    {
        if (Room)
        {
            if (AA1RaiderRoom* RaiderRoom = Cast<AA1RaiderRoom>(Room))
            {
                RaiderRoom->RemoveEnemy();
                RaiderRoom->RemoveItem();
                RaiderRoom->RemoveChest();
            }
        }
    }

    //Destroy EndWalls and Rooms
    for (auto EndWall : SpawnedEndWalls)
    {
        if (EndWall)
        {
            EndWall->Destroy();
        }
    }
    SpawnedEndWalls.Empty();

    for (auto Room : SpawnedRooms)
    {
        if (Room)
        {
            Room->Destroy();
        }
    }

    SpawnedRooms.Empty();

    //Reset Generator values
    ExitsList.Empty();
    OverlappedList.Empty();
    LatestRoom = nullptr;
    SelectedExitPoint = nullptr;
    bDungeonGenerateComplete = false;
    bVerificationComplete = false;
    ExpectedExitCount = 0;
    ExpectedEndWallCount = 0;
    RoomTransforms.Empty();
    RoomClassPaths.Empty();
    WallTransforms.Empty();

    //Generate new random seed
    Seed = -1;

    //reset room amount
    RoomAmount = MaxRoomAmount;

    //remove reset flag
    bIsResettingMap = false;

    //Play Sound depend on NetMode
    if (AudioComp)
    {
        if (GetNetMode() == NM_Standalone)
            AudioComp->Stop();
        else
            Multicast_StopSiren();
    }

    AActor* Actor = UGameplayStatics::GetActorOfClass(GetWorld(), PlundererSpawnClass);
    if (Actor)
    {
        UE_LOG(LogA1, Log, TEXT("Actor Name: %s"), *Actor->GetName());
        Actor->Destroy();
    }
    

    Multicast_StopSiren();
}

void AA1RandomMapGenerator::Server_ResetAndRegenerateMap_Implementation()
{
    if (!HasAuthority())
        return;

    Server_ResetMap();

    // Reset Log
    MAP_LOG(LogMap, Log, TEXT("=== Complete Reset. Retry Random Map Generator ==="));

    //Reset Timer
    StartTimer = GetWorld()->GetTimeSeconds();

    Server_SetSeed();
}

void AA1RandomMapGenerator::Server_CloseHoles_Implementation()
{
    if (!HasAuthority())
        return;

    if (bDungeonGenerateComplete)
    {
        MAP_LOG(LogMap, Warning, TEXT("=== Close Hole function is already Executed! ==="));
        return;
    }

    // Filtering Valid Exits
    TArray<USceneComponent*> ValidExits;
    for (auto Exit : ExitsList)
    {
        if (!Exit) continue;

        AActor* OwnerActor = Exit->GetOwner();
        if (!OwnerActor || !IsValid(OwnerActor)) continue;

        AA1MasterRoom* RoomOwner = Cast<AA1MasterRoom>(OwnerActor);
        if (!RoomOwner || !SpawnedRooms.Contains(RoomOwner)) continue;

        ValidExits.Add(Exit);
    }

    // remove duplicated exits list
    TSet<USceneComponent*> UniqueExits(ValidExits);

    // update ExpectedEndWallCount
    ExpectedEndWallCount = UniqueExits.Num();

    // Expected Log
    MAP_LOG(LogMap, Log, TEXT("Expected to create %d end walls for %d unique exits"),
        ExpectedEndWallCount, UniqueExits.Num());

    // generate EndWalls
    WallTransforms.Empty(ExpectedEndWallCount);

    for (auto Exit : UniqueExits)
    {
        if (!Exit) continue;

        FTransform ExitTransform = Exit->GetComponentToWorld();

        
        AA1EndWall* SpawnedEndWall = GetWorld()->SpawnActorDeferred<AA1EndWall>(
            EndWallClass,
            ExitTransform,
            this,
            nullptr,
            ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

        if (SpawnedEndWall)
        {
            SpawnedEndWall->SetReplicates(true);
            SpawnedEndWall->bAlwaysRelevant = true;
            SpawnedEndWall->NetUpdateFrequency = 60.0f;
            SpawnedEndWall->MinNetUpdateFrequency = 30.0f;
            SpawnedEndWall->NetPriority = 3.0f;

            UGameplayStatics::FinishSpawningActor(SpawnedEndWall, ExitTransform);

            SetupNetworkProperties(SpawnedEndWall);

            SpawnedEndWalls.Add(SpawnedEndWall);

            WallTransforms.Add(ExitTransform);
        }
    }

    Server_EnableReplicationForAllActors();

    bDungeonGenerateComplete = true;
    GetWorld()->GetTimerManager().ClearTimer(GenerateTimer);

    //Complete Log
    MAP_LOG(LogMap, Log, TEXT(" ======= Dungeon Generate Complete! ======= "));
    MAP_LOG(LogMap, Log, TEXT("Seed: %d"), Stream.GetInitialSeed());
    MAP_LOG(LogMap, Log, TEXT("Rooms created: %d"), SpawnedRooms.Num());
    MAP_LOG(LogMap, Log, TEXT("EndWalls created: %d"), SpawnedEndWalls.Num());
    MAP_LOG(LogMap, Log, TEXT("FirstFloorRooms created: %d"), FirstFloorRooms.Num());
    MAP_LOG(LogMap, Log, TEXT("SecondFloorRooms created: %d"), SecondFloorRooms.Num());

    //Play Sound depend on NetMode
    if (GetNetMode() == NM_Standalone)
        AudioComp->Stop();
    else
        Multicast_StopSiren();

    // Call RPC Functions
    FTimerHandle TimerHandle;
    GetWorldTimerManager().SetTimer(TimerHandle, this, &AA1RandomMapGenerator::Multicast_CloseHoles, 0.3f, false);

    
    IsShowSecondFloor(false);
    IsShowFirstFloor(true);

    AActor* Actor = UGameplayStatics::GetActorOfClass(GetWorld(), AA1SpaceshipBase::StaticClass());
    if (Actor)
    {
        if (AA1SpaceshipBase* SpaceshipActor = Cast<AA1SpaceshipBase>(Actor))
        {
            SpaceshipActor->SetIsExternamMapActive(true);
        }
    }

    Server_SpawnEnemy();
    Server_SpawnItem();
    //Server_MakeCliff(); TODO eric1306: 낭떠러지 생성 부하로 인한 임시 block
}

void AA1RandomMapGenerator::Multicast_CloseHoles_Implementation()
{
    //Multicast to all Clients
    if (!HasAuthority())
    {
        MAP_LOG(LogMap, Log, TEXT("Client received close holes notification!"));

        // Check Room and Wall
        MAP_LOG(LogMap, Log, TEXT("Client has %d/%d rooms and %d/%d end walls"),
            SpawnedRooms.Num(), MaxRoomAmount,
            SpawnedEndWalls.Num(), ExpectedEndWallCount);

        Multicast_OnDungeonGenerateComplete();

        Client_VerifyMapGeneration();
    }
}

void AA1RandomMapGenerator::Multicast_OnDungeonGenerateComplete_Implementation()
{
    //Check All Clients EndWall
    if (!HasAuthority())
    {
        for (auto EndWall : SpawnedEndWalls)
        {
            if (EndWall)
            {
                EndWall->SetActorHiddenInGame(false);
            }
        }

        MAP_LOG(LogMap, Log, TEXT("Client map generation complete. Rooms: %d, End Walls: %d"),
            SpawnedRooms.Num(), SpawnedEndWalls.Num());
    }

    if (AudioComp)
    {
        AudioComp->Stop();
    }
}

void AA1RandomMapGenerator::OnRep_DungeonGenerateComplete()
{
    // Call by Client when bDungeonGenerateComplete is modified
    MAP_LOG(LogMap, Log, TEXT("Client received dungeon completion notification!"));

    // Check Expected Room amount and Endwall Amount
    if (SpawnedRooms.Num() != MaxRoomAmount)
    {
        MAP_LOG(LogMap, Warning, TEXT("Client has incorrect number of rooms: %d/%d"),
            SpawnedRooms.Num(), MaxRoomAmount);
    }

    if (SpawnedEndWalls.Num() != ExpectedEndWallCount)
    {
        MAP_LOG(LogMap, Warning, TEXT("Client has incorrect number of end walls: %d/%d"),
            SpawnedEndWalls.Num(), ExpectedEndWallCount);
    }
}

void AA1RandomMapGenerator::Client_VerifyMapGeneration_Implementation()
{
    //Only Client
    if (HasAuthority() || bVerificationComplete)
        return;

    MAP_LOG(LogMap, Log, TEXT("Client verification of map generation:"));
    MAP_LOG(LogMap, Log, TEXT("- Rooms: %d/%d"), SpawnedRooms.Num(), MaxRoomAmount);
    MAP_LOG(LogMap, Log, TEXT("- End Walls: %d/%d"), SpawnedEndWalls.Num(), ExpectedEndWallCount);

    // Check Invalid Room or Endwalls
    int32 InvalidRooms = 0;
    TArray<int32> InvalidRoomIndices;

    for (int32 i = 0; i < SpawnedRooms.Num(); ++i)
    {
        if (!SpawnedRooms[i] || !IsValid(SpawnedRooms[i]))
        {
            InvalidRooms++;
            InvalidRoomIndices.Add(i);
        }
    }

    int32 InvalidWalls = 0;
    TArray<int32> InvalidWallIndices;

    for (int32 i = 0; i < SpawnedEndWalls.Num(); ++i)
    {
        if (!SpawnedEndWalls[i] || !IsValid(SpawnedEndWalls[i]))
        {
            InvalidWalls++;
            InvalidWallIndices.Add(i);
        }
    }

    if (InvalidRooms > 0 || InvalidWalls > 0)
    {
        MAP_LOG(LogMap, Warning, TEXT("Client has %d invalid rooms and %d invalid walls"),
            InvalidRooms, InvalidWalls);

        // Request Server to restore
        Server_CheckAndRepairRooms();

        FTimerHandle VerifyTimerHandle;
        GetWorldTimerManager().SetTimer(VerifyTimerHandle, this, &AA1RandomMapGenerator::Client_VerifyMapGeneration, 1.0f, false);
    }
    else
    {
        MAP_LOG(LogMap, Log, TEXT("Client verification successful: All rooms and walls are valid."));
        bVerificationComplete = true;

        AudioComp->Stop();
    }
}

void AA1RandomMapGenerator::Server_CheckAndRepairRooms_Implementation()
{
    if (!HasAuthority())
        return;

    MAP_LOG(LogMap, Log, TEXT(" === Server Ready to Restore Rooms and Walls==="));

    // Check all rooms and walls
    for (int32 i = 0; i < SpawnedRooms.Num(); ++i)
    {
        if (RoomClassPaths.IsValidIndex(i) && RoomTransforms.IsValidIndex(i))
        {
            TSubclassOf<AA1MasterRoom> RoomClass = nullptr;
            FString ClassPath = RoomClassPaths[i];

            //Find Class
            FSoftClassPath SoftClassPath(ClassPath);
            UClass* LoadedClass = SoftClassPath.TryLoadClass<AA1MasterRoom>();

            if (LoadedClass)
            {
                //Request Client to Repair Room
                Client_RepairInvalidRoom(i, RoomTransforms[i], LoadedClass);
            }
        }
    }

    for (int32 i = 0; i < SpawnedEndWalls.Num(); ++i)
    {
        if (WallTransforms.IsValidIndex(i))
        {
            //Request Client to Repair walls
            Client_RepairInvalidWall(i, WallTransforms[i]);
        }
    }
}

void AA1RandomMapGenerator::Client_RepairInvalidRoom_Implementation(int32 RoomIndex, FTransform RoomTransform, TSubclassOf<AA1MasterRoom> RoomClass)
{
    if (HasAuthority() || bVerificationComplete)
        return;

    //check room is already valid
    bool bNeedsRepair = true;

    if (SpawnedRooms.IsValidIndex(RoomIndex) && SpawnedRooms[RoomIndex] && IsValid(SpawnedRooms[RoomIndex]))
    {
        bNeedsRepair = false;
    }

    if (bNeedsRepair && RoomClass)
    {
        //generate room
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        SpawnParams.bNoFail = true;

        AA1MasterRoom* NewRoom = GetWorld()->SpawnActorDeferred<AA1MasterRoom>(
            RoomClass,
            RoomTransform,
            this,
            nullptr,
            ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

        if (NewRoom)
        {
            NewRoom->SetReplicates(true);
            NewRoom->bAlwaysRelevant = true;
            NewRoom->NetUpdateFrequency = 60.0f;
            NewRoom->MinNetUpdateFrequency = 30.0f;
            NewRoom->NetPriority = 3.0f;

            UGameplayStatics::FinishSpawningActor(NewRoom, RoomTransform);

            if (RoomIndex >= SpawnedRooms.Num())
            {
                SpawnedRooms.SetNum(RoomIndex + 1);
            }

            SpawnedRooms[RoomIndex] = NewRoom;

            MAP_LOG(LogMap, Log, TEXT(" == Completely Restore Room Index:  %d == "), RoomIndex);
        }
    }
}

void AA1RandomMapGenerator::Client_RepairInvalidWall_Implementation(int32 WallIndex, FTransform WallTransform)
{
    
    if (HasAuthority() || bVerificationComplete)
        return;

    bool bNeedsRepair = true;

    if (SpawnedEndWalls.IsValidIndex(WallIndex) && SpawnedEndWalls[WallIndex] && IsValid(SpawnedEndWalls[WallIndex]))
    {
        bNeedsRepair = false;
    }
    
    if (bNeedsRepair)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        SpawnParams.bNoFail = true;

        AA1EndWall* NewWall = GetWorld()->SpawnActorDeferred<AA1EndWall>(
            AA1EndWall::StaticClass(),
            WallTransform,
            this,
            nullptr,
            ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

        if (NewWall)
        {
            NewWall->SetReplicates(true);
            NewWall->bAlwaysRelevant = true;
            NewWall->NetUpdateFrequency = 60.0f;
            NewWall->MinNetUpdateFrequency = 30.0f;
            NewWall->NetPriority = 3.0f;

            UGameplayStatics::FinishSpawningActor(NewWall, WallTransform);

            if (WallIndex >= SpawnedEndWalls.Num())
            {
                SpawnedEndWalls.SetNum(WallIndex + 1);
            }

            SpawnedEndWalls[WallIndex] = NewWall;

            NewWall->SetActorHiddenInGame(false);

            MAP_LOG(LogMap, Log, TEXT("== Completely Restore Wall Index:  %d == "), WallIndex);
        }
    }
}

void AA1RandomMapGenerator::Server_EnableReplicationForAllActors_Implementation()
{
    if (!HasAuthority())
    {
        MAP_LOG(LogMap, Log, TEXT("Client tried to access server function"));
        return;
    }

    // Optimize Room and EndWawll Network Setting
    MAP_LOG(LogMap, Log, TEXT("SpawnedRooms num: %d"), SpawnedRooms.Num());
    for (auto Room : SpawnedRooms)
    {
        if (!Room)
        {
            MAP_LOG(LogMap, Warning, TEXT("Some Rooms Invalid!"));
            continue;
        }
        SetupNetworkProperties(Room);
    }

    MAP_LOG(LogMap, Log, TEXT("SpawnedEndWalls num: %d"), SpawnedEndWalls.Num());
    for (auto EndWall : SpawnedEndWalls)
    {
        if (!EndWall)
        {
            MAP_LOG(LogMap, Warning, TEXT("Some End Walls Invalid!"));
            continue;
        }

        SetupNetworkProperties(EndWall);
    }

    //Random Map Generator's Net Update
    ForceNetUpdate();
}