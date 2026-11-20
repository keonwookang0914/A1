// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "Maps/A1RoomLFR.h"

AA1RoomLFR::AA1RoomLFR()
{
	LeftArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Left Arrow"));
	LeftArrow->SetRelativeLocation(FVector(1000.f, -1000.f, 0.f));
	LeftArrow->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	LeftArrow->ArrowColor = FColor::Green;
	LeftArrow->ArrowSize = 10.f;
	LeftArrow->SetupAttachment(ExitsFolder);

	FrontArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Front Arrow"));
	FrontArrow->SetRelativeLocation(FVector(2000.f, 0.f, 0.f));
	FrontArrow->ArrowColor = FColor::Green;
	FrontArrow->ArrowSize = 10.f;
	FrontArrow->SetupAttachment(ExitsFolder);

	RightArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Right Arrow"));
	RightArrow->SetRelativeLocation(FVector(1000.f, 1000.f, 0.f));
	RightArrow->SetRelativeRotation(FRotator(0.f, 90.f, 0.f));
	RightArrow->ArrowColor = FColor::Green;
	RightArrow->ArrowSize = 10.f;
	RightArrow->SetupAttachment(ExitsFolder);
}
