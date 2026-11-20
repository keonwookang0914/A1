// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"
#include "A1FuelDisplayUI.generated.h"

class UTextRenderComponent;
class AA1SpaceshipBase;
class UWidgetComponent;

UCLASS()
class AA1FuelDisplayUI : public AActor
{
	GENERATED_BODY()
	
public:	
	AA1FuelDisplayUI();

protected:
    virtual void BeginPlay() override;

public:
    // Fuel Update Function
    UFUNCTION(BlueprintCallable, Category = "UI")
    void UpdateFuelLevel(float NewFuelAmount);

    UFUNCTION()
    void UpdateUI() const;

    void InitSetting(float InMaxFuel, float InInitFuel);
protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
    TObjectPtr<UTextRenderComponent> FuelTextComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
    TObjectPtr<UTextRenderComponent> TitleTextComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
    TObjectPtr<UStaticMeshComponent> ProgressBarMesh;

    // 프로그레스 바의 재질 인스턴스
    UPROPERTY(VisibleAnywhere)
	TObjectPtr<UMaterialInstanceDynamic> ProgressBarMaterial;

private:
    FTimerHandle UIUpdateTimerHandle;

    UPROPERTY(EditAnywhere, Category = "UI")
    float UIUpdateInterval;

    UPROPERTY(EditAnywhere, Category = "UI")
    float CurrentFuelLevel;

    UPROPERTY(EditAnywhere, Category = "UI")
    float MaxFuelLevel;
};
