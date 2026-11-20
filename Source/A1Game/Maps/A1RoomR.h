// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Maps/A1Room.h"
#include "A1RoomR.generated.h"

/**
 * 
 */
UCLASS()
class A1GAME_API AA1RoomR : public AA1Room
{
	GENERATED_BODY()
public:
	AA1RoomR();
protected:
	//Arrow
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UArrowComponent> RightArrow;

	//Left, FrontWall
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UStaticMeshComponent> LeftWall;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UStaticMeshComponent> FrontWall;
};
