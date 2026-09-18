// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "A1MinimapWidget.generated.h"

class UImage;

/**
 * 
 */
UCLASS()
class A1GAME_API UA1MinimapWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
    // 네비게이션 위치 업데이트 함수
    UFUNCTION(BlueprintCallable, Category = "Minimap")
    void UpdateNavigatePosition();
	
protected:
    // 네비게이션 아이콘 이미지
	UPROPERTY(BlueprintReadWrite, meta = ( BindWidget ))
    UImage* Navigate;

    // 맵 스케일 변수들
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap")
    float MapScaleX = 880.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap")
    float MapScaleY = 1330.f;

    // 미니맵이 표시할 수 있는 최대 월드 거리
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Minimap")
    float MaxWorldDistance = 1100.f;

    // 목표 위치
    UPROPERTY(BlueprintReadWrite, Category = "Minimap")
    FVector NavigatePosition = FVector(0.f, 0.f, 0.f);

    // 미니맵 UI의 반경 (픽셀)
    static constexpr float MinimapRadiusX = 92.f;
    static constexpr float MinimapRadiusY = 176.f;

};
