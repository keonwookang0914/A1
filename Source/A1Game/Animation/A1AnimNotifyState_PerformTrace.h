// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "A1Define.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "A1AnimNotifyState_PerformTrace.generated.h"

class AA1EquipmentBase;

UCLASS()
class A1GAME_API UA1AnimNotifyState_PerformTrace : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UA1AnimNotifyState_PerformTrace(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComponent, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComponent, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComponent, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

private:
	void PerformTrace(USkeletalMeshComponent* MeshComponent);

public:
	UPROPERTY(EditAnywhere)
	EEquipmentSlotType WeaponHandType = EEquipmentSlotType::LeftHand;

	UPROPERTY(EditAnywhere)
	TEnumAsByte<ENetRole> ExecuteNetRole = ROLE_Authority;

	UPROPERTY(EditAnywhere)
	FGameplayTag EventTag;

	UPROPERTY(EditAnywhere, Category="Trace")
	bool bDrawDebugShape = false;

	UPROPERTY(EditAnywhere, Category = "Trace")
	FColor TraceColor = FColor::Red;

	UPROPERTY(EditAnywhere, Category = "Trace")
	FColor HitColor = FColor::Green;

	UPROPERTY(EditAnywhere, Category = "Trace")
	float TargetDistance = 20.f;

	UPROPERTY(EditAnywhere, Category = "Trace")
	FName TraceSocketName = "TraceSocket";

private:
	UPROPERTY()
	TWeakObjectPtr<AA1EquipmentBase> WeaponActor;

	UPROPERTY()
	TSet<TWeakObjectPtr<AActor>> HitActors;

private:
	FTransform PreviousTraceTransform;
	FTransform PreviousDebugTransform;
	FTransform PreviousSocketTransform;
};
