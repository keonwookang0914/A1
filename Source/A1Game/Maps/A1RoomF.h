// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Maps/A1Room.h"
#include "A1RoomF.generated.h"

/**
 * 
 */
UCLASS()
class A1GAME_API AA1RoomF : public AA1Room
{
	GENERATED_BODY()
public:
	AA1RoomF();
protected:
	//Arrow
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UArrowComponent> FrontArrow;

	//Left, Right Wall
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UStaticMeshComponent> LeftWall;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UStaticMeshComponent> RightWall;
};
