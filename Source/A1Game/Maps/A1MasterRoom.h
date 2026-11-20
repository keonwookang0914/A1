// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"
#include "A1MasterRoom.generated.h"

UCLASS()
class A1GAME_API AA1MasterRoom : public AActor
{
	GENERATED_BODY()
	
public:
	AA1MasterRoom();
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
    FORCEINLINE USceneComponent* GetGeometryFolder() const { return GeometryFolder; }
    FORCEINLINE USceneComponent* GetExitsFolder() const { return ExitsFolder; }
    FORCEINLINE USceneComponent* GetOverlapFolder() const { return OverlapFolder; }
protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UStaticMesh> BaseStaticMesh;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<USceneComponent> DefaultSceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<class UArrowComponent> Arrow;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<USceneComponent> GeometryFolder;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<USceneComponent> OverlapFolder;

    // 오버랩 박스 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<class UBoxComponent> OverlapBox1;

    // 출구 폴더 (씬 컴포넌트로 구현)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<USceneComponent> ExitsFolder;

	//Item Section
protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item", meta = (AllowPrivateAccess = "true"), Replicated)
    int32 ItemCount;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item", meta = (AllowPrivateAccess = "true"), Replicated)
    TObjectPtr<class AActor> SpawnableItem;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item", meta = (AllowPrivateAccess = "true"), Replicated)
    FVector RandomLocation;

    //부모 클래스 리플리케이션 위한 추가 상태 변수
    UPROPERTY(Replicated)
    bool bIsRoomActive;
};
