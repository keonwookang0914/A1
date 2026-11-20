// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "Maps/A1RoomR.h"

AA1RoomR::AA1RoomR()
{
	RightArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Right Arrow"));
	RightArrow->SetRelativeLocation(FVector(1000.f, 1000.f, 0.f));
	RightArrow->SetRelativeRotation(FRotator(0.f, 90.f, 0.f));
	RightArrow->ArrowColor = FColor::Green;
	RightArrow->ArrowSize = 10.f;
	RightArrow->SetupAttachment(ExitsFolder);

	LeftWall = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftWall"));
	LeftWall->SetRelativeLocation(FVector(1000.f, -950.f, 150.f));
	LeftWall->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	LeftWall->SetRelativeScale3D(FVector(1.f, 5.f, 2.f));
	LeftWall->SetStaticMesh(BaseStaticMesh);
	LeftWall->SetupAttachment(GeometryFolder);

	FrontWall = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FrontWall"));
	FrontWall->SetRelativeLocation(FVector(1950.f, 0.f, 150.f));
	FrontWall->SetRelativeScale3D(FVector(1.f, 5.f, 2.f));
	FrontWall->SetStaticMesh(BaseStaticMesh);
	FrontWall->SetupAttachment(GeometryFolder);
}
