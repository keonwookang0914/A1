// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "A1FoamWrapChecker.generated.h"

/**
 * 
 */
UCLASS()
class A1GAME_API UA1FoamWrapChecker : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
    // 메인 함수 - 액터가 폼에 감싸져있는지 확인
    UFUNCTION(BlueprintCallable, Category = "Foam System")
    static bool IsActorWrappedByFoam(AActor* TargetActor, float WrapThreshold = 0.6f);

    // 커버리지 퍼센트 확인
    UFUNCTION(BlueprintCallable, Category = "Foam System")
    static float GetActorFoamCoverage(AActor* TargetActor);
};
