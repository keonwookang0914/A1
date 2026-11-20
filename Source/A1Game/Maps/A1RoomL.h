// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Maps/A1Room.h"
#include "A1RoomL.generated.h"

/**
 * 
 */
UCLASS()
class A1GAME_API AA1RoomL : public AA1Room
{
	GENERATED_BODY()
public:
	AA1RoomL();
protected:
	//Arrow
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UArrowComponent> LeftArrow;

	//Front, Right Walls
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UStaticMeshComponent> FrontWall;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UStaticMeshComponent> RightWall;
};
