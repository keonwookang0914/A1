// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "Maps/A1Room.h"

AA1Room::AA1Room()
{
	//Floor
	Floor = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Floor"));
	Floor->SetRelativeLocation(FVector(1000.f, 0.f, 0.f));
	Floor->SetRelativeScale3D(FVector(20.f, 20.f, 1.f));
	Floor->SetupAttachment(GeometryFolder);
	Floor->SetStaticMesh(BaseStaticMesh);

	//Walls
	RightDownRowWall = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightDownRowWall"));
	RightDownRowWall->SetRelativeLocation(FVector(50.f, 625.f, 150.f));
	RightDownRowWall->SetRelativeScale3D(FVector(1.f, 7.5f, 2.f));
	RightDownRowWall->SetStaticMesh(BaseStaticMesh);
	RightDownRowWall->SetupAttachment(GeometryFolder);


	RightDownColWall = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightDownColWall"));
	RightDownColWall->SetRelativeLocation(FVector(425.f, 950.f, 150.f));
	RightDownColWall->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	RightDownColWall->SetRelativeScale3D(FVector(1.f, 6.5f, 2.f));
	RightDownColWall->SetStaticMesh(BaseStaticMesh);
	RightDownColWall->SetupAttachment(GeometryFolder);


	LeftDownRowWall = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftDownRowWall"));
	LeftDownRowWall->SetRelativeLocation(FVector(50.f, -625.f, 150.f));
	LeftDownRowWall->SetRelativeScale3D(FVector(1.f, 7.5f, 2.f));
	LeftDownRowWall->SetStaticMesh(BaseStaticMesh);
	LeftDownRowWall->SetupAttachment(GeometryFolder);

	LeftDownColWall = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftDownColWall"));
	LeftDownColWall->SetRelativeLocation(FVector(425.f, -950.f, 150.f));
	LeftDownColWall->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	LeftDownColWall->SetRelativeScale3D(FVector(1.f, 6.5f, 2.f));
	LeftDownColWall->SetStaticMesh(BaseStaticMesh);
	LeftDownColWall->SetupAttachment(GeometryFolder);

	RightUpRowWall = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightUpRowWall"));
	RightUpRowWall->SetRelativeLocation(FVector(1950.f, 625.f, 150.f));
	RightUpRowWall->SetRelativeScale3D(FVector(1.f, 7.5f, 2.f));
	RightUpRowWall->SetStaticMesh(BaseStaticMesh);
	RightUpRowWall->SetupAttachment(GeometryFolder);

	RightUpColWall = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightUpColWall"));
	RightUpColWall->SetRelativeLocation(FVector(1575.f, 950.f, 150.f));
	RightUpColWall->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	RightUpColWall->SetRelativeScale3D(FVector(1.f, 6.5f, 2.f));
	RightUpColWall->SetStaticMesh(BaseStaticMesh);
	RightUpColWall->SetupAttachment(GeometryFolder);

	LeftUpRowWall = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftUpRowWall"));
	LeftUpRowWall->SetRelativeLocation(FVector(1950.f, -625.f, 150.f));
	LeftUpRowWall->SetRelativeScale3D(FVector(1.f, 7.5f, 2.f));
	LeftUpRowWall->SetStaticMesh(BaseStaticMesh);
	LeftUpRowWall->SetupAttachment(GeometryFolder);

	LeftUpColWall = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftUpColWall"));
	LeftUpColWall->SetRelativeLocation(FVector(1575.f, -950.f, 150.f));
	LeftUpColWall->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	LeftUpColWall->SetRelativeScale3D(FVector(1.f, 6.5f, 2.f));
	LeftUpColWall->SetStaticMesh(BaseStaticMesh);
	LeftUpColWall->SetupAttachment(GeometryFolder);
}
