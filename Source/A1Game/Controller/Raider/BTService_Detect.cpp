// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#include "BTService_Detect.h"

#include "A1RaiderController.h"
#include "A1LogChannels.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Raider/A1RaiderBase.h"
#include "Character/A1PlayerCharacter.h"
#include "Character/LyraCharacter.h"
#include "DrawDebugHelpers.h"
#include "Engine/OverlapResult.h"
#include "Physics/LyraCollisionChannels.h"

UBTService_Detect::UBTService_Detect()
{
	NodeName = TEXT("Detect");
	Interval = 1.0f;
}

void UBTService_Detect::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AA1RaiderBase* ControllingPawn = Cast<AA1RaiderBase>(OwnerComp.GetAIOwner()->GetPawn());
	if (ControllingPawn == nullptr)
		return;

	UWorld* World = ControllingPawn->GetWorld();
	if (World == nullptr)
		return;

	TArray<FHitResult> HitResults;
    FVector SelfLocation = ControllingPawn->GetActorLocation();
    FVector SelfForward = ControllingPawn->GetActorForwardVector();
	FCollisionQueryParams CollisionQueryParam;
    CollisionQueryParam.AddIgnoredActor(ControllingPawn);

    bool bResult = World->SweepMultiByChannel(
        HitResults,
        SelfLocation,
        SelfLocation,
		FQuat::Identity,
        A1_TraceChannel_Raider,
		FCollisionShape::MakeSphere(800.f),
		CollisionQueryParam
	);

	if (!bResult)	
        return;
	else
	{
        for (auto& HitResult : HitResults)
        {
            ALyraCharacter* Target = Cast<ALyraCharacter>(HitResult.GetActor());
            if (!Target)
                continue;

            // 사망한 캐릭터는 감지 대상 제외
            if (Target->GetDeathState() != EA1DeathState::NotDead)
                continue;

            FVector ToTarget = (Target->GetActorLocation() - SelfLocation).GetSafeNormal();

            // 2. Dot Product로 FOV 체크
            float Dot = FVector::DotProduct(SelfForward, ToTarget);
            float CosHalfFOV = FMath::Cos(FMath::DegreesToRadians(120 * 0.5f));

            if (Dot < CosHalfFOV)                           // 시야 범위 밖                  
                continue;


            // 3. 시야 막힘 체크 (Line Trace)
            FHitResult Hit;
            bool bHit = GetWorld()->LineTraceSingleByChannel(
                Hit,
                SelfLocation,
                Target->GetActorLocation(),
                ECC_Visibility,
                CollisionQueryParam
            );

            if (bHit && Hit.GetActor() != Target)          // 막혀 있음
                continue;

            // 4. 감지 성공
            OwnerComp.GetBlackboardComponent()->SetValueAsObject(AA1RaiderController::AggroTargetKey, Target);
            UE_LOG(LogA1Raider, Warning, TEXT("Detected Actor: %s"), *Target->GetName());


            // 디버그 시각화
            DrawDebugLine(GetWorld(), SelfLocation, Target->GetActorLocation(), FColor::Green, false, 1.0f);       
        }

        DrawDebugCone(
            GetWorld(), SelfLocation, SelfForward, 800.f,
            FMath::DegreesToRadians(120 * 0.5f), FMath::DegreesToRadians(120 * 0.5f),
            16, FColor::Cyan, false, 1.0f);
	}

	return;
}
