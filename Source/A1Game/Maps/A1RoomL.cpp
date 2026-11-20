// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "Maps/A1RoomL.h"

AA1RoomL::AA1RoomL()
{
	LeftArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Left Arrow"));
	LeftArrow->SetRelativeLocation(FVector(1000.f, -1000.f, 0.f));
	LeftArrow->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	LeftArrow->ArrowColor = FColor::Green;
	LeftArrow->ArrowSize = 10.f;
	LeftArrow->SetupAttachment(ExitsFolder);

	FrontWall = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FrontWall"));
	FrontWall->SetRelativeLocation(FVector(1950.f, 0.f, 150.f));
	FrontWall->SetRelativeScale3D(FVector(1.f, 5.f, 2.f));
	FrontWall->SetStaticMesh(BaseStaticMesh);
	FrontWall->SetupAttachment(GeometryFolder);

	RightWall = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftWall"));
	RightWall->SetRelativeLocation(FVector(1000.f, 950.f, 150.f));
	RightWall->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	RightWall->SetRelativeScale3D(FVector(1.f, 5.f, 2.f));
	RightWall->SetStaticMesh(BaseStaticMesh);
	RightWall->SetupAttachment(GeometryFolder);
}
