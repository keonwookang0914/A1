// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Maps/A1Room.h"
#include "A1RoomLFR.generated.h"

/**
 * 
 */
UCLASS()
class A1GAME_API AA1RoomLFR : public AA1Room
{
	GENERATED_BODY()
public:
	AA1RoomLFR();
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UArrowComponent> LeftArrow;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UArrowComponent> FrontArrow;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UArrowComponent> RightArrow;
};
