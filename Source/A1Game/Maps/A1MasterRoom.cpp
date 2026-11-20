// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "Maps/A1MasterRoom.h"

#include "Net/UnrealNetwork.h"

// Sets default values
AA1MasterRoom::AA1MasterRoom()
{
	PrimaryActorTick.bCanEverTick = false;

    bReplicates = true;

    DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
    RootComponent = DefaultSceneRoot;

    Arrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
    Arrow->SetupAttachment(RootComponent);
    Arrow->ArrowSize = 8.f;
    Arrow->SetRelativeLocation(FVector(580.f, 0.f, 130.f));

    GeometryFolder = CreateDefaultSubobject<USceneComponent>(TEXT("GeometryFolder"));
    GeometryFolder->SetupAttachment(RootComponent);

    OverlapFolder = CreateDefaultSubobject<USceneComponent>(TEXT("OverlapFolder"));
    OverlapFolder->SetupAttachment(RootComponent);

    OverlapBox1 = CreateDefaultSubobject<UBoxComponent>(TEXT("OverlapBox1"));
    OverlapBox1->SetRelativeLocation(FVector(1000.f, 0.f, 0.f));
    OverlapBox1->SetRelativeScale3D(FVector(30.f, 30.f, 1.f));
    OverlapBox1->SetCollisionProfileName(TEXT("Trigger"));
    OverlapBox1->SetupAttachment(OverlapFolder);
    OverlapBox1->SetBoxExtent(FVector(32.f, 32.f, 32.f));


    ExitsFolder = CreateDefaultSubobject<USceneComponent>(TEXT("ExitsFolder"));
    ExitsFolder->SetupAttachment(RootComponent);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> BaseStaticMeshRef(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (BaseStaticMeshRef.Object)
    {
        BaseStaticMesh = BaseStaticMeshRef.Object;
    }
    bIsRoomActive = true;
}

void AA1MasterRoom::BeginPlay()
{
	Super::BeginPlay();

    SetReplicateMovement(true);
}

void AA1MasterRoom::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AA1MasterRoom, ItemCount);
    DOREPLIFETIME(AA1MasterRoom, SpawnableItem);
    DOREPLIFETIME(AA1MasterRoom, RandomLocation);
    DOREPLIFETIME(AA1MasterRoom, bIsRoomActive);
}
