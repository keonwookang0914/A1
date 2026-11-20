// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "Maps/A1EndWall.h"

#include "Net/UnrealNetwork.h"

// Sets default values
AA1EndWall::AA1EndWall()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;

	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent = DefaultSceneRoot;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> BaseStaticMeshRef(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (BaseStaticMeshRef.Object)
	{
		BaseStaticMesh = BaseStaticMeshRef.Object;
	}

	EndWall = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EndWall"));
	EndWall->SetRelativeLocation(FVector(-50.f, 0.f, 150.f));
	EndWall->SetRelativeScale3D(FVector(1.f, 5.f, 2.f));
	EndWall->SetStaticMesh(BaseStaticMesh);
	EndWall->SetupAttachment(RootComponent);

	bIsWallActive = true;
}

void AA1EndWall::BeginPlay()
{
	Super::BeginPlay();

	SetReplicateMovement(true);
}

void AA1EndWall::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AA1EndWall, bIsWallActive);
}

