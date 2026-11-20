// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Maps/A1MasterRoom.h"
#include "A1RoomBridge.generated.h"

/**
 * 
 */
UCLASS()
class A1GAME_API AA1RoomBridge : public AA1MasterRoom
{
	GENERATED_BODY()
public:
	AA1RoomBridge();

	virtual void BeginPlay() override;
protected:
	//Arrow
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UArrowComponent> FrontArrow;

	//Floor
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UStaticMeshComponent> Floor;

	//Wall
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UStaticMeshComponent> LeftWall;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UStaticMeshComponent> RightWall;
};
