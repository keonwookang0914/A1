// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#include "Maps/A1RaiderRoom.h"

#include "A1LogChannels.h"
#include "Actors/A1ChestBase.h"
#include "Actors/A1EquipmentBase.h"
#include "Character/Raider/A1RaiderBase.h"
#include "Data/A1ItemData.h"
#include "Item/A1ItemInstance.h"
#include "Item/Fragments/A1ItemFragment_Equipable_Attachment.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Physics/LyraCollisionChannels.h"
#include "Engine/OverlapResult.h"
#include "ProceduralMeshComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1RaiderRoom)

AA1RaiderRoom::AA1RaiderRoom(const FObjectInitializer& ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = true;

    // Create spawn components
    EssentialSpawn = CreateDefaultSubobject<USceneComponent>(TEXT("Essential Spawn Area"));
    EssentialSpawn->SetupAttachment(GetRootComponent());

    OptionalSpawn = CreateDefaultSubobject<USceneComponent>(TEXT("Optional Spawn Area"));
    OptionalSpawn->SetupAttachment(GetRootComponent());

    ItemSpawnLocation = CreateDefaultSubobject<USceneComponent>(TEXT("ItemSpawnLocation"));
    ItemSpawnLocation->SetupAttachment(GetRootComponent());

    ItemBoxSpawnLocation = CreateDefaultSubobject<USceneComponent>(TEXT("ChestLocation"));
    ItemBoxSpawnLocation->SetupAttachment(GetRootComponent());

    RemoveItemPosition = CreateDefaultSubobject<USceneComponent>(TEXT("Remove Pos Location"));
    RemoveItemPosition->SetupAttachment(GetRootComponent());

    // Create mesh components
    Inner = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Inner"));
    Inner->SetupAttachment(GeometryFolder);

    Floor = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Floor"));
    Floor->SetupAttachment(GeometryFolder);

    // ProceduralMeshComponent for cliff floor - Shipping build safe
    CliffProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("CliffProceduralMesh"));
    CliffProceduralMesh->SetupAttachment(GeometryFolder);

    // Initially hidden
    CliffProceduralMesh->SetVisibility(false);
    CliffProceduralMesh->SetHiddenInGame(true);
    CliffProceduralMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Collision settings
    CliffProceduralMesh->SetCollisionObjectType(ECC_WorldStatic);
    CliffProceduralMesh->SetCollisionResponseToAllChannels(ECR_Block);
    CliffProceduralMesh->SetCollisionProfileName(FName("BlockAll"));
}

void AA1RaiderRoom::BeginPlay()
{
    Super::BeginPlay();
}

void AA1RaiderRoom::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AA1RaiderRoom, SpawnedCreatures);
    DOREPLIFETIME(AA1RaiderRoom, SpawnedItems);
    DOREPLIFETIME(AA1RaiderRoom, SpawnedChests);
}

void AA1RaiderRoom::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (!HasAuthority())
        return;

    // Process spawn queue
    SpawnTimer += DeltaSeconds;
    if (SpawnTimer >= SpawnInterval)
    {
        SpawnTimer = 0.0f;
        ProcessSpawnQueue();
    }
}

/************************************************
 * SPAWN FUNCTIONS SECTION (keeping existing implementation)
 ************************************************/

void AA1RaiderRoom::SpawnEnemies(TSubclassOf<AA1CreatureBase> CreatureClass)
{
    if (!HasAuthority())
        return;

    // Essential spawns
    TArray<USceneComponent*> EssentialChildren;
    EssentialSpawn->GetChildrenComponents(false, EssentialChildren);

    for (auto Child : EssentialChildren)
    {
        if (Child)
        {
            AddEnemyToQueue(CreatureClass, Child->GetComponentTransform());
        }
    }

    // Optional spawns
    TArray<USceneComponent*> OptionalChildren;
    OptionalSpawn->GetChildrenComponents(false, OptionalChildren);
    int32 SpawnOptionalMonsterRate = FMath::RandRange(MinOptionalMonster, MaxOptionalMonster);

    for (auto Child : OptionalChildren)
    {
        int32 SpawnRate = FMath::RandRange(1, 101);
        if (SpawnPercentage < SpawnRate)
            continue;

        if (Child && SpawnOptionalMonsterRate-- > 0)
        {
            AddEnemyToQueue(CreatureClass, Child->GetComponentTransform());
        }
    }
}

void AA1RaiderRoom::SpawnEnemy(FTransform SpawnTransform, TSubclassOf<AA1CreatureBase> EnemyClass)
{
    AA1CreatureBase* Enemy = GetWorld()->SpawnActorDeferred<AA1CreatureBase>(EnemyClass, SpawnTransform, this);
    if (Enemy)
    {
        UGameplayStatics::FinishSpawningActor(Enemy, SpawnTransform);
        SpawnedCreatures.Add(Enemy);
    }
}

void AA1RaiderRoom::SpawnItems()
{
    TArray<USceneComponent*> ChildrenComp;
    ItemSpawnLocation->GetChildrenComponents(false, ChildrenComp);
    ItemSpawnLocations.Append(ChildrenComp);

    int32 SpawnItemRate = FMath::RandRange(MinOptionalItemCount, MaxOptionalItemCount);
    UA1ItemData::Get().GetAllItemTemplateClasses(CachedItemTemplates);

    for (int i = 0; i < SpawnItemRate; i++)
    {
        AddItemToQueue(i);
    }

    // Spawn chests
    TArray<USceneComponent*> ChestChildren;
    ItemBoxSpawnLocation->GetChildrenComponents(false, ChestChildren);
    ItemBoxSpawnLocations.Append(ChestChildren);

    for (auto Child : ChestChildren)
    {
        if (Child)
        {
            AddChestToQueue(Child->GetComponentTransform());
        }
    }
}

void AA1RaiderRoom::SpawnItem(int32 idx)
{
    if (idx >= ItemSpawnLocations.Num() || CachedItemTemplates.Num() <= 5)
        return;

    int32 ItemToSpawn = FMath::RandRange(2, CachedItemTemplates.Num() - 6);
    TSubclassOf<UA1ItemTemplate> ItemTemplateClass = CachedItemTemplates[ItemToSpawn];
    int32 ItemTemplateId = UA1ItemData::Get().FindItemTemplateIDByClass(ItemTemplateClass);

    UA1ItemInstance* ItemInstance = NewObject<UA1ItemInstance>();
    ItemInstance->Init(ItemTemplateId, EItemRarity::Poor);

    const UA1ItemFragment_Equipable_Attachment* AttachmentFragment = ItemInstance->FindFragmentByClass<UA1ItemFragment_Equipable_Attachment>();
    if (!AttachmentFragment)
        return;

    const FA1ItemAttachInfo& AttachInfo = AttachmentFragment->ItemAttachInfo;
    if (AttachInfo.SpawnItemClass)
    {
        AA1EquipmentBase* NewItem = GetWorld()->SpawnActorDeferred<AA1EquipmentBase>(AttachInfo.SpawnItemClass, FTransform::Identity, this);
        NewItem->Init(ItemInstance->GetItemTemplateID(), EEquipmentSlotType::Count, ItemInstance->GetItemRarity());
        NewItem->SetActorRelativeTransform(ItemSpawnLocations[idx]->GetComponentTransform());
        NewItem->SetActorScale3D(FVector::OneVector);
        NewItem->SetPickup(false);
        NewItem->SetActorHiddenInGame(false);
        NewItem->FinishSpawning(FTransform::Identity, true);
    }
}

void AA1RaiderRoom::SpawnChest(FTransform SpawnTransform)
{
    AA1ChestBase* Chest = GetWorld()->SpawnActorDeferred<AA1ChestBase>(ChestClass, SpawnTransform, this);
    if (Chest)
    {
        UGameplayStatics::FinishSpawningActor(Chest, SpawnTransform);
        SpawnedChests.Add(Chest);
    }
}

void AA1RaiderRoom::RemoveEnemy()
{
    for (auto Enemy : SpawnedCreatures)
    {
        if (Enemy)
        {
            Enemy->Destroy();
        }
    }
    SpawnedCreatures.Empty();
}

void AA1RaiderRoom::RemoveItem()
{
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

    TArray<FOverlapResult> OverlapResults;
    FCollisionShape CollisionShape = FCollisionShape::MakeBox(FVector(1500.f, 1500.f, 1000.f));

    bool bResult = GetWorld()->OverlapMultiByChannel(
        OverlapResults,
        RemoveItemPosition->GetComponentLocation(),
        FQuat::Identity,
        A1_TraceChannel_AimAssist,
        CollisionShape,
        QueryParams
    );

    if (bResult)
    {
        for (const FOverlapResult& Overlap : OverlapResults)
        {
            AActor* OverlappedActor = Overlap.GetActor();
            if (!OverlappedActor)
                continue;

            if (OverlappedActor->ActorHasTag(FName("foam")))
            {
                OverlappedActor->Destroy();
            }

            if (AA1EquipmentBase* Equipment = Cast<AA1EquipmentBase>(OverlappedActor))
            {
                Equipment->SetLifeSpan(0.01f);
                Equipment->Destroy(true);
            }
        }
    }
}

void AA1RaiderRoom::RemoveChest()
{
    for (auto Chest : SpawnedChests)
    {
        if (Chest)
        {
            Chest->SetLifeSpan(0.01f);
            Chest->Destroy(true);
        }
    }
    SpawnedChests.Empty();
}

void AA1RaiderRoom::SetRoomType(int32 RoomIndex)
{
    switch (RoomIndex)
    {
    case 0:
    case 6:
    case 7:
        RoomType = ERoomType::Rounge;
        break;
    case 1:
        RoomType = ERoomType::Storage;
        break;
    case 2:
        RoomType = ERoomType::Container;
        break;
    case 3:
        RoomType = ERoomType::Master;
        break;
    case 4:
        RoomType = ERoomType::Bridge;
        break;
    case 5:
        RoomType = ERoomType::LBridge;
        break;
    case 8:
        RoomType = ERoomType::BedRoom;
        break;
    case 9:
        RoomType = ERoomType::SecondFloor;
        break;
    }
}
/************************************************
 * SPAWN QUEUE SECTION
 ************************************************/
void AA1RaiderRoom::AddEnemyToQueue(TSubclassOf<AA1CreatureBase> EnemyClass, const FTransform& Transform)
{
    SpawnQueue.Enqueue(FSpawnQueueItem(FSpawnQueueItem::Enemy, Transform, EnemyClass));
}

void AA1RaiderRoom::AddItemToQueue(int32 ItemIndex)
{
    SpawnQueue.Enqueue(FSpawnQueueItem(FSpawnQueueItem::Item, FTransform::Identity, nullptr, ItemIndex));
}

void AA1RaiderRoom::AddChestToQueue(const FTransform& Transform)
{
    SpawnQueue.Enqueue(FSpawnQueueItem(FSpawnQueueItem::Chest, Transform));
}

void AA1RaiderRoom::ProcessSpawnQueue()
{
    if (!HasAuthority())
        return;

    int32 SpawnedThisTick = 0;
    FSpawnQueueItem Item;

    while (SpawnedThisTick < SpawnPerTick && SpawnQueue.Dequeue(Item))
    {
        switch (Item.Type)
        {
        case FSpawnQueueItem::Enemy:
            SpawnEnemy(Item.Transform, Item.EnemyClass);
            break;
        case FSpawnQueueItem::Item:
            SpawnItem(Item.ItemIndex);
            break;
        case FSpawnQueueItem::Chest:
            SpawnChest(Item.Transform);
            break;
        }

        SpawnedThisTick++;
    }
}

/************************************************
 * CLIFF CREATION - SHIPPING BUILD SAFE
 ************************************************/
void AA1RaiderRoom::CreateCliffHole()
{
    if (bCliffInProgress)
    {
        UE_LOG(LogA1Cliff, Warning, TEXT("Cliff creation already in progress"));
        return;
    }

    // Random Delay
    float RandomDelay = FMath::RandRange(0.0f, 2.0f);

    UE_LOG(LogA1Cliff, Warning, TEXT("Cliff creation scheduled with %.1f second delay"), RandomDelay);

    //Random Start Timer(Lambda)
    FTimerHandle DelayHandle;
    GetWorld()->GetTimerManager().SetTimer(DelayHandle, [this]()
        {
            StartCliffCreation();
        }, RandomDelay, false);
}

void AA1RaiderRoom::StartCliffCreation()
{
    UE_LOG(LogA1Cliff, Warning, TEXT("Starting Procedural cliff creation (80 x 80 grid)"));

    bCliffInProgress = true;
    CurrentCliffStep = 1;
    CliffStartTime = FPlatformTime::Seconds();

    // Initialize random stream
    int32 RandomSeed = UKismetMathLibrary::RandomInteger(INT32_MAX);
    RandomStream = UKismetMathLibrary::MakeRandomStream(RandomSeed);

    UE_LOG(LogA1Cliff, Warning, TEXT("Hole Seed: %d"), RandomSeed);

    GetWorld()->GetTimerManager().SetTimer(
        CliffTimerHandle,
        this,
        &AA1RaiderRoom::ExecuteCliffStep,
        StepTime,
        true
    );
}

void AA1RaiderRoom::ExecuteCliffStep()
{
    StartStepTimer();

    switch (CurrentCliffStep)
    {
    case 1:
        Step01_GetFloorDimensions();
        LogStepTime(TEXT("01_GetFloorDimensions"));
        break;
    case 2:
        Step02_CreateBasePlane();
        LogStepTime(TEXT("02_CreateBasePlane"));
        break;
    case 3:
        Step03_CreateHoleGeometry();
        LogStepTime(TEXT("03_CreateHoleGeometry"));
        break;
    case 4:
        Step04_RemoveTrianglesInHole();
        LogStepTime(TEXT("04_RemoveTrianglesInHole"));
        break;
    case 5:
        Step05_GenerateCliffWalls();
        LogStepTime(TEXT("05_GenerateCliffWalls"));
        break;
    case 6:
        Step06_CreateFinalMesh();
        LogStepTime(TEXT("06_CreateFinalMesh"));
        break;
    case 7:
        Step07_FinalizeCliff();
        LogStepTime(TEXT("07_FinalizeCliff"));
        break;
    case 8:
        Step08_FinalizeCliff_2();
        LogStepTime(TEXT("08_FinalizeCliff_2"));
        break;
    default:
        GetWorld()->GetTimerManager().ClearTimer(CliffTimerHandle);
        bCliffInProgress = false;

        double TotalTime = FPlatformTime::Seconds() - CliffStartTime;
        UE_LOG(LogA1Cliff, Warning, TEXT("=== Cliff creation completed in %.6f seconds (7 steps) ==="), TotalTime);

        if (TotalTime <= 0.25)
        {
            UE_LOG(LogA1Cliff, Warning, TEXT("SUCCESS: Target 250ms achieved for high-resolution mesh!"));
        }
        else
        {
            UE_LOG(LogA1Cliff, Warning, TEXT("Performance note: %.6f seconds (target: 0.25s for 80 x 80 resolution)"), TotalTime);
        }
        return;
    }

    CurrentCliffStep++;
}

void AA1RaiderRoom::Step01_GetFloorDimensions()
{
    // Get floor information
    if (Floor && Floor->GetStaticMesh())
    {
        // Get floor bounds
        FBoxSphereBounds Bounds = Floor->GetStaticMesh()->GetBounds();
        FloorSizeX = Bounds.BoxExtent.X * 2.0f;
        FloorSizeY = Bounds.BoxExtent.Y * 2.0f;

        // Store Floor transform
        OriginalFloorTransform = Floor->GetComponentTransform();

        // Extract all materials from original floor
        int32 MaterialCount = Floor->GetNumMaterials();


        UE_LOG(LogA1Cliff, Log, TEXT("Floor info - Dimensions: %.1f x %.1f, Transform: Loc(%.1f,%.1f,%.1f) Rot(%.1f,%.1f,%.1f) Scale(%.2f,%.2f,%.2f)"),
            FloorSizeX, FloorSizeY,
            OriginalFloorTransform.GetLocation().X, OriginalFloorTransform.GetLocation().Y, OriginalFloorTransform.GetLocation().Z,
            OriginalFloorTransform.GetRotation().Rotator().Pitch, OriginalFloorTransform.GetRotation().Rotator().Yaw, OriginalFloorTransform.GetRotation().Rotator().Roll,
            OriginalFloorTransform.GetScale3D().X, OriginalFloorTransform.GetScale3D().Y, OriginalFloorTransform.GetScale3D().Z);
    }
    else
    {
        FloorSizeX = 2000.0f;
        FloorSizeY = 2000.0f;
        OriginalFloorTransform = FTransform::Identity;

        UE_LOG(LogA1Cliff, Log, TEXT("Using default floor configuration: %.1f x %.1f"), FloorSizeX, FloorSizeY);
    }
}

void AA1RaiderRoom::Step02_CreateBasePlane()
{
    // Create high-resolution procedural plane mesh for smooth hole edges
    FinalVertices.Empty();
    FinalTriangles.Empty();
    FinalNormals.Empty();
    FinalUVs.Empty();

    // Increase resolution significantly for smooth hole edges
    const int32 PlaneResolution = 80; // 80 x 80 grid for smooth geometry
    const float StepX = FloorSizeX / PlaneResolution;
    const float StepY = FloorSizeY / PlaneResolution;
    const float HalfSizeX = FloorSizeX * 0.5f;
    const float HalfSizeY = FloorSizeY * 0.5f;

    // Reserve memory
    const int32 TotalVertices = (PlaneResolution + 1) * (PlaneResolution + 1);
    const int32 TotalTriangles = PlaneResolution * PlaneResolution * 6;

    FinalVertices.Reserve(TotalVertices);
    FinalNormals.Reserve(TotalVertices);
    FinalUVs.Reserve(TotalVertices);
    FinalTriangles.Reserve(TotalTriangles);

    // Generate high-resolution vertices grid
    for (int32 Y = 0; Y <= PlaneResolution; Y++)
    {
        for (int32 X = 0; X <= PlaneResolution; X++)
        {
            FVector VertexPos(
                -HalfSizeX + X * StepX,
                -HalfSizeY + Y * StepY,
                0.0f
            );

            FinalVertices.Add(VertexPos);
            FinalNormals.Add(FVector::UpVector);

            //UV mapping
            FinalUVs.Add(FVector2D(
                static_cast<float>(X) / static_cast<float>(PlaneResolution),
                static_cast<float>(Y) / static_cast<float>(PlaneResolution)
            ));
        }
    }

    // Generate triangles with proper winding order
    for (int32 Y = 0; Y < PlaneResolution; Y++)
    {
        for (int32 X = 0; X < PlaneResolution; X++)
        {
            int32 V0 = Y * (PlaneResolution + 1) + X;
            int32 V1 = V0 + 1;
            int32 V2 = V0 + (PlaneResolution + 1);
            int32 V3 = V2 + 1;

            // Validate indices
            if (V0 < FinalVertices.Num() && V1 < FinalVertices.Num() &&
                V2 < FinalVertices.Num() && V3 < FinalVertices.Num())
            {
                // Triangle 1: V0->V2->V1 (counter-clockwise)
                FinalTriangles.Add(V0);
                FinalTriangles.Add(V2);
                FinalTriangles.Add(V1);

                // Triangle 2: V1->V2->V3 (counter-clockwise)
                FinalTriangles.Add(V1);
                FinalTriangles.Add(V2);
                FinalTriangles.Add(V3);
            }
        }
    }

    UE_LOG(LogA1Cliff, Log, TEXT("Created high-resolution base plane: %d vertices, %d triangles (resolution: %dx%d)"),
        FinalVertices.Num(), FinalTriangles.Num() / 3, PlaneResolution, PlaneResolution);

    // Validate mesh integrity
    bool bMeshValid = true;
    if (FinalVertices.Num() == 0 || FinalTriangles.Num() == 0 || FinalTriangles.Num() % 3 != 0)
    {
        bMeshValid = false;
        UE_LOG(LogA1Cliff, Error, TEXT("Invalid mesh data: Vertices=%d, Triangles=%d"),
            FinalVertices.Num(), FinalTriangles.Num());
    }

    if (bMeshValid)
    {
        UE_LOG(LogA1Cliff, Log, TEXT("High-resolution mesh validation passed"));
    }
}

void AA1RaiderRoom::Step03_CreateHoleGeometry()
{
    // Generate random hole position
    HoleCenter = FVector(
        UKismetMathLibrary::RandomFloatInRangeFromStream(RandomStream, -FloorSizeX * 0.25f, FloorSizeX * 0.25f),
        UKismetMathLibrary::RandomFloatInRangeFromStream(RandomStream, -FloorSizeY * 0.25f, FloorSizeY * 0.25f),
        0.0f
    );

    HoleHalfSize = HoleSize * 0.5f;

    UE_LOG(LogA1Cliff, Log, TEXT("Square hole created at position (%.1f, %.1f) with half-size %.1f"),
        HoleCenter.X, HoleCenter.Y, HoleHalfSize);
}

void AA1RaiderRoom::Step04_RemoveTrianglesInHole()
{
    // Simple and safe hole removal
    TArray<FVector> TempVertices = FinalVertices;
    TArray<int32> TempTriangles = FinalTriangles;
    TArray<FVector> TempNormals = FinalNormals;
    TArray<FVector2D> TempUVs = FinalUVs;

    FinalVertices.Empty();
    FinalTriangles.Empty();
    FinalNormals.Empty();
    FinalUVs.Empty();

    // Clear hole boundary indices
    HoleBoundaryIndices.Empty();

    // Vertex remapping
    TMap<int32, int32> OldToNewVertexMap;
    int32 NewVertexIndex = 0;

    // Hole Boundary (Hole-Size)
    const float HoleMinX = HoleCenter.X - HoleHalfSize;
    const float HoleMaxX = HoleCenter.X + HoleHalfSize;
    const float HoleMinY = HoleCenter.Y - HoleHalfSize;
    const float HoleMaxY = HoleCenter.Y + HoleHalfSize;

    // Process triangles - check overlap
    for (int32 TriIndex = 0; TriIndex < TempTriangles.Num(); TriIndex += 3)
    {
        int32 V0 = TempTriangles[TriIndex];
        int32 V1 = TempTriangles[TriIndex + 1];
        int32 V2 = TempTriangles[TriIndex + 2];

        FVector Pos0 = TempVertices[V0];
        FVector Pos1 = TempVertices[V1];
        FVector Pos2 = TempVertices[V2];

        // Calculate triangle Boundary's Box
        float TriMinX = FMath::Min3(Pos0.X, Pos1.X, Pos2.X);
        float TriMaxX = FMath::Max3(Pos0.X, Pos1.X, Pos2.X);
        float TriMinY = FMath::Min3(Pos0.Y, Pos1.Y, Pos2.Y);
        float TriMaxY = FMath::Max3(Pos0.Y, Pos1.Y, Pos2.Y);

        bool TriangleOverlapsHole = !(TriMaxX <= HoleMinX || TriMinX >= HoleMaxX ||
            TriMaxY <= HoleMinY || TriMinY >= HoleMaxY);

        //if Overlap-> Not Add Triangle
        if (TriangleOverlapsHole)
        {
            continue;
        }

        TArray<int32> NewTriangleIndices;
        for (int32 VertexIndex : {V0, V1, V2})
        {
            if (!OldToNewVertexMap.Contains(VertexIndex))
            {
                OldToNewVertexMap.Add(VertexIndex, NewVertexIndex);
                FinalVertices.Add(TempVertices[VertexIndex]);
                FinalNormals.Add(TempNormals[VertexIndex]);
                FinalUVs.Add(TempUVs[VertexIndex]);
                NewVertexIndex++;
            }
            NewTriangleIndices.Add(OldToNewVertexMap[VertexIndex]);
        }
        FinalTriangles.Append(NewTriangleIndices);
    }

    //After Clipping, find real open edge vertex
    TMap<int32, TArray<int32>> VertexToTriangles;
    TSet<int32> BoundaryVertexSet;

    // make vertex-triangle connect info
    for (int32 TriIndex = 0; TriIndex < FinalTriangles.Num(); TriIndex += 3)
    {
        int32 V0 = FinalTriangles[TriIndex];
        int32 V1 = FinalTriangles[TriIndex + 1];
        int32 V2 = FinalTriangles[TriIndex + 2];

        VertexToTriangles.FindOrAdd(V0).Add(TriIndex / 3);
        VertexToTriangles.FindOrAdd(V1).Add(TriIndex / 3);
        VertexToTriangles.FindOrAdd(V2).Add(TriIndex / 3);
    }

    for (int32 TriIndex = 0; TriIndex < FinalTriangles.Num(); TriIndex += 3)
    {
        int32 V0 = FinalTriangles[TriIndex];
        int32 V1 = FinalTriangles[TriIndex + 1];
        int32 V2 = FinalTriangles[TriIndex + 2];

        TArray<TPair<int32, int32>> Edges = {
            {V0, V1}, {V1, V2}, {V2, V0}
        };

        for (auto& Edge : Edges)
        {
            int32 SharedTriangles = 0;

            const TArray<int32>* TriList1 = VertexToTriangles.Find(Edge.Key);
            const TArray<int32>* TriList2 = VertexToTriangles.Find(Edge.Value);

            if (TriList1 && TriList2)
            {
                for (int32 Tri1 : *TriList1)
                {
                    if (TriList2->Contains(Tri1))
                    {
                        SharedTriangles++;
                    }
                }
            }

            if (SharedTriangles == 1)
            {
                BoundaryVertexSet.Add(Edge.Key);
                BoundaryVertexSet.Add(Edge.Value);
            }
        }
    }

    for (int32 VertexIndex : BoundaryVertexSet)
    {
        const FVector& Vertex = FinalVertices[VertexIndex];
        float DistanceToHoleCenter = FVector::Dist2D(Vertex, HoleCenter);

        if (DistanceToHoleCenter <= HoleHalfSize * 1.5f)
        {
            HoleBoundaryIndices.Add(VertexIndex);
        }
    }

    if (HoleBoundaryIndices.Num() > 0)
    {
        auto SortClockwise = [&](const int32& A, const int32& B) -> bool
            {
                const FVector& VertexA = FinalVertices[A];
                const FVector& VertexB = FinalVertices[B];

                FVector DirA = VertexA - HoleCenter;
                FVector DirB = VertexB - HoleCenter;

                float AngleA = FMath::Atan2(DirA.Y, DirA.X);
                float AngleB = FMath::Atan2(DirB.Y, DirB.X);

                if (AngleA < 0) AngleA += 2.0f * PI;
                if (AngleB < 0) AngleB += 2.0f * PI;

                return AngleA < AngleB;
            };

        HoleBoundaryIndices.StableSort(SortClockwise);
    }

    if (HoleBoundaryIndices.Num() == 0)
    {
        UE_LOG(LogA1Cliff, Warning, TEXT("No open edge vertices found, creating precise boundary vertices"));

        TArray<FVector> PreciseBoundaryVertices = {
            FVector(HoleCenter.X - HoleHalfSize, HoleCenter.Y - HoleHalfSize, 0.0f), // Bottom-Left
            FVector(HoleCenter.X + HoleHalfSize, HoleCenter.Y - HoleHalfSize, 0.0f), // Bottom-Right
            FVector(HoleCenter.X + HoleHalfSize, HoleCenter.Y + HoleHalfSize, 0.0f), // Top-Right
            FVector(HoleCenter.X - HoleHalfSize, HoleCenter.Y + HoleHalfSize, 0.0f)  // Top-Left
        };

        int32 BoundaryStartIndex = FinalVertices.Num();
        for (int32 i = 0; i < PreciseBoundaryVertices.Num(); i++)
        {
            FinalVertices.Add(PreciseBoundaryVertices[i]);
            FinalNormals.Add(FVector::UpVector);
            FinalUVs.Add(FVector2D((PreciseBoundaryVertices[i].X + FloorSizeX * 0.5f) / FloorSizeX,
                (PreciseBoundaryVertices[i].Y + FloorSizeY * 0.5f) / FloorSizeY));
            HoleBoundaryIndices.Add(BoundaryStartIndex + i);
        }
    }

    UE_LOG(LogA1Cliff, Log, TEXT("Found %d actual boundary vertices from clipped mesh"),
        HoleBoundaryIndices.Num());
}

void AA1RaiderRoom::Step05_GenerateCliffWalls()
{
    if (HoleBoundaryIndices.Num() == 0)
    {
        UE_LOG(LogA1Cliff, Error, TEXT("No hole boundary vertices found"));
        return;
    }

    int32 StartVertexIndex = FinalVertices.Num();

    UE_LOG(LogA1Cliff, Log, TEXT("Creating walls from %d boundary vertices"), HoleBoundaryIndices.Num());

    // Add Wall's Vertices and Normal
    for (int32 i = 0; i < HoleBoundaryIndices.Num(); i++)
    {
        int32 BoundaryIndex = HoleBoundaryIndices[i];
        FVector TopVertex = FinalVertices[BoundaryIndex];
        FVector BottomVertex = TopVertex + FVector(0.0f, 0.0f, -HoleDepth);

        FinalVertices.Add(TopVertex);
        FinalVertices.Add(BottomVertex);

        int32 NextI = (i + 1) % HoleBoundaryIndices.Num();
        FVector NextVertex = FinalVertices[HoleBoundaryIndices[NextI]];
        FVector EdgeDir = (NextVertex - TopVertex).GetSafeNormal();
        FVector WallNormal = FVector::CrossProduct(EdgeDir, FVector(0.0f, 0.0f, -1.0f)).GetSafeNormal();

        FinalNormals.Add(WallNormal);
        FinalNormals.Add(WallNormal);

        // UV Coordinate
        float U = float(i) / HoleBoundaryIndices.Num();
        FinalUVs.Add(FVector2D(U, 0.0f)); // Top
        FinalUVs.Add(FVector2D(U, 1.0f)); // Bottom
    }

    //make triangle
    int32 WallTrianglesAdded = 0;
    for (int32 i = 0; i < HoleBoundaryIndices.Num(); i++)
    {
        int32 NextI = (i + 1) % HoleBoundaryIndices.Num();

        int32 TopCurrent = StartVertexIndex + i * 2;
        int32 BottomCurrent = StartVertexIndex + i * 2 + 1;
        int32 TopNext = StartVertexIndex + NextI * 2;
        int32 BottomNext = StartVertexIndex + NextI * 2 + 1;

        //Counter-clockwise -> For Material
        FinalTriangles.Add(TopCurrent);
        FinalTriangles.Add(BottomCurrent);
        FinalTriangles.Add(TopNext);

        FinalTriangles.Add(TopNext);
        FinalTriangles.Add(BottomCurrent);
        FinalTriangles.Add(BottomNext);

        WallTrianglesAdded += 2;
    }

    UE_LOG(LogA1Cliff, Log, TEXT("Cliff walls created: %d wall triangles added"), WallTrianglesAdded);
}

void AA1RaiderRoom::Step06_CreateFinalMesh()
{
    if (!CliffProceduralMesh)
    {
        UE_LOG(LogA1Cliff, Error, TEXT("CliffProceduralMesh is null"));
        return;
    }

    if (FinalVertices.Num() == 0 || FinalTriangles.Num() == 0)
    {
        UE_LOG(LogA1Cliff, Error, TEXT("No mesh data to create"));
        return;
    }

    // Simple validation
    for (int32 Index : FinalTriangles)
    {
        if (Index >= FinalVertices.Num())
        {
            UE_LOG(LogA1Cliff, Error, TEXT("Invalid triangle index"));
            return;
        }
    }

    // Create tangents and colors
    TArray<FProcMeshTangent> Tangents;
    TArray<FLinearColor> VertexColors;

    for (int32 i = 0; i < FinalVertices.Num(); i++)
    {
        Tangents.Add(FProcMeshTangent(FVector::ForwardVector, false));
        VertexColors.Add(FLinearColor::White);
    }

    // Clear and create mesh
    CliffProceduralMesh->ClearAllMeshSections();
    CliffProceduralMesh->CreateMeshSection_LinearColor(
        0, FinalVertices, FinalTriangles, FinalNormals, FinalUVs,
        TArray<FVector2D>(), TArray<FVector2D>(), TArray<FVector2D>(),
        VertexColors, Tangents, true
    );

    UE_LOG(LogA1Cliff, Log, TEXT("Mesh with outward walls: %d vertices, %d triangles"),
        FinalVertices.Num(), FinalTriangles.Num() / 3);
}

void AA1RaiderRoom::Step07_FinalizeCliff()
{
    // Set Offset, FVector(1440, -30, 0)
    FVector RelativePosition(1440.0f, -30.0f, 0.0f);
    CliffProceduralMesh->SetRelativeLocation(RelativePosition);

    if (!OriginalFloorTransform.Equals(FTransform::Identity))
    {
        CliffProceduralMesh->SetRelativeRotation(OriginalFloorTransform.GetRotation());
        CliffProceduralMesh->SetRelativeScale3D(OriginalFloorTransform.GetScale3D());

        UE_LOG(LogA1Cliff, Log, TEXT("Applied relative transform - Pos(%.1f,%.1f,%.1f) Rot(%.1f,%.1f,%.1f) Scale(%.2f,%.2f,%.2f)"),
            RelativePosition.X, RelativePosition.Y, RelativePosition.Z,
            OriginalFloorTransform.GetRotation().Rotator().Pitch,
            OriginalFloorTransform.GetRotation().Rotator().Yaw,
            OriginalFloorTransform.GetRotation().Rotator().Roll,
            OriginalFloorTransform.GetScale3D().X,
            OriginalFloorTransform.GetScale3D().Y,
            OriginalFloorTransform.GetScale3D().Z);
    }
    else
    {
        CliffProceduralMesh->SetRelativeRotation(FRotator::ZeroRotator);
        CliffProceduralMesh->SetRelativeScale3D(FVector::OneVector);

        UE_LOG(LogA1Cliff, Log, TEXT("Applied relative position only: (%.1f,%.1f,%.1f)"),
            RelativePosition.X, RelativePosition.Y, RelativePosition.Z);
    }

    // Apply materials from original floor
    bool bMaterialApplied = false;

    // Fallback to StaticMeshMaterials array if floor materials not available
    if (!bMaterialApplied && StaticMeshMaterials.Num() > 0 && StaticMeshMaterials[0])
    {
        CliffProceduralMesh->SetMaterial(0, StaticMeshMaterials[0]);
        bMaterialApplied = true;
        UE_LOG(LogA1Cliff, Log, TEXT("Applied fallback material: %s"), *StaticMeshMaterials[0]->GetName());
    }

    // Final fallback - try to get material from Floor component directly
    if (!bMaterialApplied && Floor)
    {
        UMaterialInterface* FloorMaterial = Floor->GetMaterial(0);
        if (FloorMaterial)
        {
            CliffProceduralMesh->SetMaterial(0, FloorMaterial);
            bMaterialApplied = true;
            UE_LOG(LogA1Cliff, Log, TEXT("Applied direct floor material: %s"), *FloorMaterial->GetName());
        }
    }

    if (!bMaterialApplied)
    {
        UE_LOG(LogA1Cliff, Warning, TEXT("No material applied - using default"));
    }

    UE_LOG(LogA1Cliff, Warning, TEXT("Shipping-safe procedural cliff creation completed at relative position (%.1f, %.1f, %.1f)"),
        RelativePosition.X, RelativePosition.Y, RelativePosition.Z);
}

void AA1RaiderRoom::Step08_FinalizeCliff_2()
{
    // Set collision and visibility
    CliffProceduralMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    CliffProceduralMesh->SetCollisionObjectType(ECC_WorldStatic);
    CliffProceduralMesh->SetCollisionResponseToAllChannels(ECR_Block);
    CliffProceduralMesh->SetCollisionProfileName(FName("BlockAll"));

    // Enable auto-smooth normals for better lighting
    CliffProceduralMesh->bUseComplexAsSimpleCollision = true;
    CliffProceduralMesh->SetNotifyRigidBodyCollision(true);

    // Make visible
    CliffProceduralMesh->SetVisibility(true);
    CliffProceduralMesh->SetHiddenInGame(false);
    CliffProceduralMesh->SetCastShadow(true);

    // Hide original floor
    if (Floor)
    {
        Floor->SetVisibility(false);
        Floor->SetHiddenInGame(true);
        Floor->SetCollisionEnabled(ECollisionEnabled::NoCollision);

        UE_LOG(LogA1Cliff, Log, TEXT("Original floor hidden"));
    }

    // Force a final collision and rendering update
    CliffProceduralMesh->MarkRenderStateDirty();
}

/********************************************
 * HELPER FUNCTIONS
 ********************************************/
void AA1RaiderRoom::StartStepTimer()
{
    StepStartTime = FPlatformTime::Seconds();
}

FVector AA1RaiderRoom::GetSquareEdgeIntersection(const FVector& P1, const FVector& P2, int32 EdgeIndex)
{
    float t = 0.0f;
    FVector Intersection = P1;

    switch (EdgeIndex)
    {
    case 0: // Bottom edge (Y = HoleCenter.Y - HoleHalfSize)
        if (FMath::Abs(P2.Y - P1.Y) > KINDA_SMALL_NUMBER)
        {
            t = (HoleCenter.Y - HoleHalfSize - P1.Y) / (P2.Y - P1.Y);
            if (t >= 0.0f && t <= 1.0f)
            {
                Intersection = P1 + t * (P2 - P1);
                Intersection.Y = HoleCenter.Y - HoleHalfSize; // Ensure exact boundary
            }
        }
        break;
    case 1: // Right edge (X = HoleCenter.X + HoleHalfSize)
        if (FMath::Abs(P2.X - P1.X) > KINDA_SMALL_NUMBER)
        {
            t = (HoleCenter.X + HoleHalfSize - P1.X) / (P2.X - P1.X);
            if (t >= 0.0f && t <= 1.0f)
            {
                Intersection = P1 + t * (P2 - P1);
                Intersection.X = HoleCenter.X + HoleHalfSize; // Ensure exact boundary
            }
        }
        break;
    case 2: // Top edge (Y = HoleCenter.Y + HoleHalfSize)
        if (FMath::Abs(P2.Y - P1.Y) > KINDA_SMALL_NUMBER)
        {
            t = (HoleCenter.Y + HoleHalfSize - P1.Y) / (P2.Y - P1.Y);
            if (t >= 0.0f && t <= 1.0f)
            {
                Intersection = P1 + t * (P2 - P1);
                Intersection.Y = HoleCenter.Y + HoleHalfSize; // Ensure exact boundary
            }
        }
        break;
    case 3: // Left edge (X = HoleCenter.X - HoleHalfSize)
        if (FMath::Abs(P2.X - P1.X) > KINDA_SMALL_NUMBER)
        {
            t = (HoleCenter.X - HoleHalfSize - P1.X) / (P2.X - P1.X);
            if (t >= 0.0f && t <= 1.0f)
            {
                Intersection = P1 + t * (P2 - P1);
                Intersection.X = HoleCenter.X - HoleHalfSize; // Ensure exact boundary
            }
        }
        break;
    }

    return Intersection;
}

TArray<FVector> AA1RaiderRoom::ClipTriangleToSquare(const TArray<FVector>& TriangleVertices)
{
    TArray<FVector> ClippedVertices = TriangleVertices;

    for (int32 EdgeIndex = 0; EdgeIndex < 4; EdgeIndex++)
    {
        TArray<FVector> InputVertices = ClippedVertices;
        ClippedVertices.Empty();

        if (InputVertices.Num() == 0) break;

        for (int32 i = 0; i < InputVertices.Num(); i++)
        {
            FVector CurrentVertex = InputVertices[i];
            FVector NextVertex = InputVertices[(i + 1) % InputVertices.Num()];

            bool CurrentInside = IsVertexOutsideHole(CurrentVertex);
            bool NextInside = IsVertexOutsideHole(NextVertex);

            if (CurrentInside)
            {
                if (NextInside)
                {
                    ClippedVertices.Add(NextVertex);
                }
                else
                {
                    FVector Intersection = GetSquareEdgeIntersection(CurrentVertex, NextVertex, EdgeIndex);
                    ClippedVertices.Add(Intersection);
                }
            }
            else
            {
                if (NextInside)
                {
                    FVector Intersection = GetSquareEdgeIntersection(CurrentVertex, NextVertex, EdgeIndex);
                    ClippedVertices.Add(Intersection);
                    ClippedVertices.Add(NextVertex);
                }
            }
        }
    }

    return ClippedVertices;
}

bool AA1RaiderRoom::IsVertexOutsideHole(const FVector& Vertex)
{
    float DeltaX = FMath::Abs(Vertex.X - HoleCenter.X);
    float DeltaY = FMath::Abs(Vertex.Y - HoleCenter.Y);
    return (DeltaX >= HoleHalfSize || DeltaY >= HoleHalfSize);
}

void AA1RaiderRoom::LogStepTime(const FString& StepName)
{
    if (!bShowStepTiming)
        return;

    double StepDuration = FPlatformTime::Seconds() - StepStartTime;
    double StepDurationMs = StepDuration * 1000.0;

    FColor LogColor = (StepDurationMs > 5.0) ? FColor::Red : FColor::Green;

    UE_LOG(LogA1Cliff, Warning, TEXT("Step %s: %.6f seconds (%.3f ms)"),
        *StepName, StepDuration, StepDurationMs);

    if (StepDurationMs > 10.0)
    {
        UE_LOG(LogA1Cliff, Error, TEXT("CRITICAL: Step %s took %.3f ms (target: <5ms)"), *StepName, StepDurationMs);
    }

    if (GEngine)
    {
        FString DisplayText = FString::Printf(TEXT("Step %s: %.3f ms"), *StepName, StepDurationMs);
        GEngine->AddOnScreenDebugMessage(-1, 3.f, LogColor, DisplayText);
    }
}

bool AA1RaiderRoom::IsVertexInSquareHole(const FVector& Vertex)
{
    return (Vertex.X > HoleCenter.X - HoleHalfSize &&
        Vertex.X < HoleCenter.X + HoleHalfSize &&
        Vertex.Y > HoleCenter.Y - HoleHalfSize &&
        Vertex.Y < HoleCenter.Y + HoleHalfSize);
}