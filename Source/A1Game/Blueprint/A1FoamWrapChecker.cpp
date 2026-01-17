// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "Blueprint/A1FoamWrapChecker.h"

#include "A1LogChannels.h"
#include "Engine/OverlapResult.h"

bool UA1FoamWrapChecker::IsActorWrappedByFoam(AActor* TargetActor, float WrapThreshold)
{
    if (!IsValid(TargetActor))
        return false;

    float Coverage = GetActorFoamCoverage(TargetActor);
    //UE_LOG(LogA1, Log, TEXT("Coverage: %f"), Coverage);
    return Coverage >= WrapThreshold;
}

float UA1FoamWrapChecker::GetActorFoamCoverage(AActor* TargetActor)
{
    if (!IsValid(TargetActor))
        return 0.0f;

    FVector Center = TargetActor->GetActorLocation();
    FVector Extent = FVector(10.f, 10.f, 10.f); // 적절한 크기로 조정

    TArray<FOverlapResult> OverlapResults;
    FCollisionShape CollisionShape = FCollisionShape::MakeBox(Extent);

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(TargetActor); // 자기 자신은 제외

    // Overlap 검사로 범위 내 액터들 찾기
    bool bFoundOverlaps = TargetActor->GetWorld()->OverlapMultiByChannel(
        OverlapResults, 
        Center,
        FQuat::Identity,
        ECC_WorldDynamic,
        CollisionShape,
        QueryParams
    );

    DrawDebugBox(TargetActor->GetWorld(), Center, Extent, FColor::Red, true);

    // Foam 액터들만 필터링
    TArray<AActor*> FoamActors;
    if (bFoundOverlaps)
    {
        for (const FOverlapResult& Result : OverlapResults)
        {
            AActor* OverlapActor = Result.GetActor();
            
            if (IsValid(OverlapActor) &&
                OverlapActor->GetClass()->GetName().Contains(TEXT("Foam")))
            {
                UE_LOG(LogA1, Log, TEXT("%s"), *OverlapActor->GetName())
                FoamActors.AddUnique(OverlapActor);
            }
        }
    }

    if (FoamActors.Num() == 0)
        return 0.0f;

    // 체크 포인트 생성 (기존과 동일)
    TArray<FVector> CheckPoints;

    // 8개 모서리
    CheckPoints.Add(Center + FVector(Extent.X, Extent.Y, Extent.Z));
    CheckPoints.Add(Center + FVector(Extent.X, Extent.Y, -Extent.Z));
    CheckPoints.Add(Center + FVector(Extent.X, -Extent.Y, Extent.Z));
    CheckPoints.Add(Center + FVector(Extent.X, -Extent.Y, -Extent.Z));
    CheckPoints.Add(Center + FVector(-Extent.X, Extent.Y, Extent.Z));
    CheckPoints.Add(Center + FVector(-Extent.X, Extent.Y, -Extent.Z));
    CheckPoints.Add(Center + FVector(-Extent.X, -Extent.Y, Extent.Z));
    CheckPoints.Add(Center + FVector(-Extent.X, -Extent.Y, -Extent.Z));

    // 6개 면 중심
    CheckPoints.Add(Center + FVector(Extent.X, 0, 0));
    CheckPoints.Add(Center + FVector(-Extent.X, 0, 0));
    CheckPoints.Add(Center + FVector(0, Extent.Y, 0));
    CheckPoints.Add(Center + FVector(0, -Extent.Y, 0));
    CheckPoints.Add(Center + FVector(0, 0, Extent.Z));
    CheckPoints.Add(Center + FVector(0, 0, -Extent.Z));

    // 중심점
    CheckPoints.Add(Center);

    // 각 체크 포인트가 폼에 덮여있는지 확인
    int32 CoveredPoints = 0;

    for (const FVector& Point : CheckPoints)
    {
        bool bPointCovered = false;

        // 필터링된 폼 액터들과만 체크 (훨씬 적은 수)
        for (AActor* FoamActor : FoamActors)
        {
            if (!IsValid(FoamActor))
                continue;

            // 폼 액터의 모든 Primitive 컴포넌트 체크
            TArray<UPrimitiveComponent*> PrimitiveComponents;
            FoamActor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);

            for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
            {
                if (!IsValid(PrimComp) || !PrimComp->GetCollisionEnabled())
                    continue;

                // 포인트가 컴포넌트 바운드 안에 있는지 체크
                FBox ComponentBounds = PrimComp->Bounds.GetBox();
                if (ComponentBounds.IsInsideOrOn(Point))
                {
                    bPointCovered = true;
                    break;
                }
            }

            if (bPointCovered)
                break;
        }

        if (bPointCovered)
            CoveredPoints++;
    }

    // 커버리지 비율 반환
    return CheckPoints.Num() > 0 ? (float)CoveredPoints / (float)CheckPoints.Num() : 0.0f;
}
