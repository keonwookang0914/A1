// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "Maps/A1RoomBridge.h"

AA1RoomBridge::AA1RoomBridge()
{
	//Arrow
	FrontArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("FrontArrow"));
	FrontArrow->SetRelativeLocation(FVector(2000.f, 0.f, 0.f));
	FrontArrow->ArrowColor = FColor::Green;
	FrontArrow->ArrowSize = 10.f;
	FrontArrow->SetupAttachment(ExitsFolder);

	//Floor
	Floor = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Floor"));
	Floor->SetRelativeLocation(FVector(1000.f, 0.f, -30.f));
	Floor->SetRelativeScale3D(FVector(20.f, 5.f, 1.f));
	Floor->SetStaticMesh(BaseStaticMesh);
	Floor->SetupAttachment(GeometryFolder);

	//Wall
	LeftWall = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftWall"));
	LeftWall->SetRelativeLocation(FVector(1000.f, -300.f, 120.f));
	LeftWall->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	LeftWall->SetRelativeScale3D(FVector(1.f, 20.f, 2.f));
	LeftWall->SetStaticMesh(BaseStaticMesh);
	LeftWall->SetupAttachment(GeometryFolder);

	RightWall = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightWall"));
	RightWall->SetRelativeLocation(FVector(1000.f, 300.f, 120.f));
	RightWall->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	RightWall->SetRelativeScale3D(FVector(1.f, 20.f, 2.f));
	RightWall->SetStaticMesh(BaseStaticMesh);
	RightWall->SetupAttachment(GeometryFolder);
}

void AA1RoomBridge::BeginPlay()
{
	Super::BeginPlay();
}
