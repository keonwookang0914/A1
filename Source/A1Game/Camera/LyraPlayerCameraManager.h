// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Camera/PlayerCameraManager.h"

#include "LyraPlayerCameraManager.generated.h"

class FDebugDisplayInfo;
class UCanvas;
class UObject;


#define LYRA_CAMERA_DEFAULT_FOV			(80.0f)
#define LYRA_CAMERA_DEFAULT_PITCH_MIN	(-89.0f)
#define LYRA_CAMERA_DEFAULT_PITCH_MAX	(89.0f)


/**
 * ALyraPlayerCameraManager
 *
 *	The base player camera manager class used by this project.
 */
UCLASS(notplaceable, MinimalAPI)
class ALyraPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

public:

	ALyraPlayerCameraManager(const FObjectInitializer& ObjectInitializer);

protected:


	virtual void DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos) override;

};
