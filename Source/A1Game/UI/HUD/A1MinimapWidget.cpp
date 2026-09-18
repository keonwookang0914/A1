// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "UI/HUD/A1MinimapWidget.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"


void UA1MinimapWidget::UpdateNavigatePosition()
{
	APawn* PlayerPawn = GetOwningPlayerPawn();
	if ( !PlayerPawn || !Navigate )
	{
		return;
	}

	FVector PlayerLocation = PlayerPawn->GetActorLocation();
	FRotator PlayerRotation = PlayerPawn->GetActorRotation();
	float PlayerYaw = PlayerRotation.Yaw;

	//월드 좌표 -> 로컬 좌표 -> UI 좌표 변환 수행
	
	//플레이어 기준 위치 변환(Translation 변환. 105.6, 4.59는 Offset)
	double DeltaX = NavigatePosition.X - PlayerLocation.X + 105.6;
	double DeltaY = NavigatePosition.Y - PlayerLocation.Y - 4.59;

	//플레이어 기준 회전 변환(Rotation 변환)
	/*
	* |cos sin | * | x |
	* |-sin cos|   | y |
	*/
	double CosYaw = UKismetMathLibrary::DegCos(PlayerYaw);
	double SinYaw = UKismetMathLibrary::DegSin(PlayerYaw);
	double RotatedX = ( DeltaX * CosYaw ) + ( DeltaY * SinYaw );
	double RotatedY = ( DeltaY * CosYaw ) - ( DeltaX * SinYaw );

	//미니맵 사이즈로 축소(Scale 변환)
	double ScaleFactorX = MinimapRadiusX / MapScaleX;
	double ScaleFactorY = MinimapRadiusY / MapScaleY; 
	double ScaledX = RotatedX * ScaleFactorX;
	double ScaledY = RotatedY * ScaleFactorY;
	double ClampedX = FMath::Clamp(ScaledX, -MinimapRadiusX, MinimapRadiusX);
	double ClampedY = FMath::Clamp(ScaledY, -MinimapRadiusY, MinimapRadiusY);

	UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Navigate->Slot);
	if ( CanvasSlot )
	{
		FVector2D NewPosition(ClampedY, ClampedX * -1.0);
		CanvasSlot->SetPosition(NewPosition);
	}

	Navigate->SetRenderTransformAngle(PlayerYaw);
}
