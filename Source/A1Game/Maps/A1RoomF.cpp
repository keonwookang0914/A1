// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "Maps/A1RoomF.h"

AA1RoomF::AA1RoomF()
{
	FrontArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Front Arrow"));
	FrontArrow->SetRelativeLocation(FVector(2000.f, 0.f, 0.f));
	FrontArrow->ArrowColor = FColor::Green;
	FrontArrow->ArrowSize = 10.f;
	FrontArrow->SetupAttachment(ExitsFolder);

	LeftWall = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftWall"));
	LeftWall->SetRelativeLocation(FVector(1000.f, -950.f, 150.f));
	LeftWall->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	LeftWall->SetRelativeScale3D(FVector(1.f, 5.f, 2.f));
	LeftWall->SetStaticMesh(BaseStaticMesh);
	LeftWall->SetupAttachment(GeometryFolder);


	RightWall = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightWall"));
	RightWall->SetRelativeLocation(FVector(1000.f, 950.f, 150.f));
	RightWall->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	RightWall->SetRelativeScale3D(FVector(1.f, 5.f, 2.f));
	RightWall->SetStaticMesh(BaseStaticMesh);
	RightWall->SetupAttachment(GeometryFolder);
}
