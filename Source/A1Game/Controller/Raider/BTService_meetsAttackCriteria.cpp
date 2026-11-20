// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "Controller/Raider/BTService_meetsAttackCriteria.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Character/LyraCharacter.h"
#include "Character/Raider/A1RaiderBase.h"
#include "Controller/Raider/A1RaiderController.h"
#include "Item/Managers/A1EquipManagerComponent.h"

UBTService_meetsAttackCriteria::UBTService_meetsAttackCriteria()
{
	NodeName = TEXT("meetsAttackCriteria");
	Interval = 1.0f;
}

void UBTService_meetsAttackCriteria::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AA1RaiderBase* ControllingPawn = Cast<AA1RaiderBase>(OwnerComp.GetAIOwner()->GetPawn());
	if (ControllingPawn == nullptr)
		return;

	if (OwnerComp.GetBlackboardComponent()->GetValueAsObject(AA1RaiderController::AggroTargetKey) == nullptr)
		return;
	// AggroTarget이 있을때만
	else
	{
		auto Target = OwnerComp.GetBlackboardComponent()->GetValueAsObject(AA1RaiderController::AggroTargetKey);
		ALyraCharacter* TargetActor = Cast<ALyraCharacter>(Target);
		if (TargetActor == nullptr)
			return;

		// 캐릭터 사망 시 Aggro 제외
		if (TargetActor->GetDeathState() != EA1DeathState::NotDead)
		{
			OwnerComp.GetBlackboardComponent()->SetValueAsObject(AA1RaiderController::AggroTargetKey, nullptr);
			OwnerComp.GetBlackboardComponent()->SetValueAsBool(AA1RaiderController::CanAttackKey, false);
			return;
		}

		// 두 객체 간 거리 계산, cm 단위라고 함
		float dist = FVector::Dist(ControllingPawn->GetActorLocation(), TargetActor->GetActorLocation());

		//타겟이 10m 이상 멀어지면 aggro 해제
		if (dist >= 1000.f)
		{
			OwnerComp.GetBlackboardComponent()->SetValueAsBool(AA1RaiderController::CanAttackKey, false);
			OwnerComp.GetBlackboardComponent()->SetValueAsObject(AA1RaiderController::AggroTargetKey, nullptr);	

			ControllingPawn->EnterAttackMode(false);
		}

		// 선공 상태가 아닐때만 계산
		if (OwnerComp.GetBlackboardComponent()->GetValueAsBool(AA1RaiderController::CanAttackKey) == false)
		{
			// 선공 조건 만족하면 공격 가능 상태로 전환
			bool bStrikeFirst = false;
			// 뒤돌고 있는가
			{
				FVector MyForward = ControllingPawn->GetActorForwardVector();
				FVector SelfToTarget = (TargetActor->GetActorLocation() - ControllingPawn->GetActorLocation()).GetSafeNormal();

				FVector TargetForward = TargetActor->GetActorForwardVector();
				FVector TargetToSelf = (ControllingPawn->GetActorLocation() - TargetActor->GetActorLocation()).GetSafeNormal();

				float Dot1 = FVector::DotProduct(MyForward, SelfToTarget);			// 타겟이 내 앞?
				float Dot2 = FVector::DotProduct(TargetForward, TargetToSelf);		// 타겟이 나에게 등 돌림?

				// 135도 이상 -0.7f
				if (Dot1 > 0.0f && Dot2 < -0.7f)
					bStrikeFirst = true;
			}

			// 무기 없는가?
			{
				UA1EquipManagerComponent* EquipManager = TargetActor->FindComponentByClass<UA1EquipManagerComponent>();
				if (EquipManager == nullptr)
					return;

				if (EquipManager->GetCurrentEquipState() == EEquipState::Unarmed)
					bStrikeFirst = true;
			}

			OwnerComp.GetBlackboardComponent()->SetValueAsBool(AA1RaiderController::CanAttackKey, bStrikeFirst);

			// 기본 Idle 음성 멈추기
			if(bStrikeFirst)
				ControllingPawn->EnterAttackMode(true);	
		}
	}
}
