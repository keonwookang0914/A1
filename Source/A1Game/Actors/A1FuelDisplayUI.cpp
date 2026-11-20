// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "Actors/A1FuelDisplayUI.h"

#include "Components/TextRenderComponent.h"
#include "Kismet/GameplayStatics.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1FuelDisplayUI)

AA1FuelDisplayUI::AA1FuelDisplayUI()
{
    PrimaryActorTick.bCanEverTick = false;

    // 연료 수준 텍스트 설정
    FuelTextComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("FuelTextComponent"));
    SetRootComponent(FuelTextComponent);
    FuelTextComponent->SetRelativeLocation(FVector(-1.0f, 5.0f, 0.0f));
    FuelTextComponent->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
    FuelTextComponent->SetText(FText::FromString(TEXT("FUEL: 50.0%")));
    FuelTextComponent->SetTextRenderColor(FColor::Yellow);
    FuelTextComponent->SetHorizontalAlignment(EHTA_Center);
    FuelTextComponent->SetWorldSize(15.0f);
    FuelTextComponent->SetXScale(1.0f);
    FuelTextComponent->SetYScale(1.0f);

    // 프로그레스 바 메시 생성
    ProgressBarMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProgressBarMesh"));
    ProgressBarMesh->SetupAttachment(GetRootComponent());
    ProgressBarMesh->SetMaterial(0, ProgressBarMaterial);
    ProgressBarMesh->SetRelativeScale3D(FVector(0.4f, 0.05f, 0.1f));
    ProgressBarMesh->SetRelativeLocation(FVector(-2.0f, 3.0f, -15.0f));
    ProgressBarMesh->SetRelativeRotation(FRotator(0.f, 90.f, 0.f));

    // 기본값 설정
    UIUpdateInterval = 0.5f;
}

void AA1FuelDisplayUI::BeginPlay()
{
	Super::BeginPlay();
    if (ProgressBarMesh->GetMaterial(0))
    {
        ProgressBarMaterial = ProgressBarMesh->CreateAndSetMaterialInstanceDynamic(0);

        if (ProgressBarMaterial)
        {
            ProgressBarMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor::Yellow);
        }
    }

    //Update Timer
    GetWorldTimerManager().SetTimer(
        UIUpdateTimerHandle,
        this,
        &AA1FuelDisplayUI::UpdateUI,
        UIUpdateInterval,
        true
    );

    // Init UI Update
    UpdateUI();
}

void AA1FuelDisplayUI::UpdateFuelLevel(float NewFuelAmount)
{
    // 연료 레벨 업데이트
    CurrentFuelLevel = FMath::Clamp(NewFuelAmount, 0.0f, MaxFuelLevel);

    // UI 즉시 업데이트
    UpdateUI();
}

void AA1FuelDisplayUI::UpdateUI() const
{
    float FuelRatio = CurrentFuelLevel / MaxFuelLevel;

    //Update Text
    FString FuelText = FString::Printf(TEXT("FUEL: %.0f / %.0f"), CurrentFuelLevel, MaxFuelLevel);

    FuelTextComponent->SetText(FText::FromString(FuelText));

    
    if (FuelRatio > 0.6f)
    {
        FuelTextComponent->SetTextRenderColor(FColor::Green);
    }
    else if (FuelRatio > 0.3f)
    {
        FuelTextComponent->SetTextRenderColor(FColor::Yellow);
    }
    else
    {
        FuelTextComponent->SetTextRenderColor(FColor::Red);
    }

    
    FVector CurrentScale = ProgressBarMesh->GetRelativeScale3D();
    ProgressBarMesh->SetRelativeScale3D(FVector(0.4f * FuelRatio, CurrentScale.Y, CurrentScale.Z));

    // Set Location By Scale
    float XOffset = (0.4f * (1.0f - FuelRatio)) * 50.0f;
    ProgressBarMesh->SetRelativeLocation(FVector(-XOffset, 3.0f, -15.0f));

    // Update ProgressBar Color
    if (ProgressBarMaterial)
    {
        if (FuelRatio > 0.6f)
        {
            ProgressBarMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor::Green);
        }
        else if (FuelRatio > 0.3f)
        {
            ProgressBarMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor::Yellow);
        }
        else
        {
            ProgressBarMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor::Red);
        }
    }
}

void AA1FuelDisplayUI::InitSetting(float InMaxFuel, float InInitFuel)
{
    MaxFuelLevel = InMaxFuel;
    CurrentFuelLevel = InInitFuel;
}
