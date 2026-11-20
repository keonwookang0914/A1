#pragma once

#include "Abilities/Tasks/AbilityTask.h"
#include "Interaction/A1InteractionQuery.h"
#include "A1AbilityTask_WaitForInteractableTraceHit.generated.h"

struct FA1InteractionInfo;
class IA1Interactable;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractableChanged, const TArray<FA1InteractionInfo>&,InteractableInfos);

UCLASS()
class A1GAME_API UA1AbilityTask_WaitForInteractableTraceHit : public UAbilityTask
{
	GENERATED_BODY()

public:
	UA1AbilityTask_WaitForInteractableTraceHit(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	UFUNCTION(BlueprintCallable, Category="Ability|Tasks", meta=(HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"))
	static UA1AbilityTask_WaitForInteractableTraceHit* WaitForInteractableTraceHit(UGameplayAbility* OwningAbility, FA1InteractionQuery InteractionQuery, ECollisionChannel TraceChannel, FGameplayAbilityTargetingLocationInfo StartLocation, float InteractionTraceRange = 100.f, float InteractionTraceRate = 0.1f, bool bShowDebug = false);

protected:
	virtual void Activate() override;
	virtual void OnDestroy(bool bInOwnerFinished) override;

private:
	void PerformTrace();

	void AimWithPlayerController(const AActor* InSourceActor, FCollisionQueryParams Params, const FVector& TraceStart, float MaxRange, FVector& OutTraceEnd, bool bIgnorePitch = false) const;
	bool ClipCameraRayToAbilityRange(FVector CameraLocation, FVector CameraDirection, FVector AbilityCenter, float AbilityRange, FVector& OutClippedPosition) const;
	void LineTrace(const FVector& Start, const FVector& End, const FCollisionQueryParams& Params, FHitResult& OutHitResult) const;

	void UpdateInteractionInfos(const FA1InteractionQuery& InteractQuery, const TArray<TScriptInterface<IA1Interactable>>& Interactables);
	void HighlightInteractables(const TArray<FA1InteractionInfo>& InteractionInfos, bool bShouldHighlight);

public:
	UPROPERTY(BlueprintAssignable)
	FOnInteractableChanged InteractableChanged;

private:
	UPROPERTY()
	FA1InteractionQuery InteractionQuery;

	UPROPERTY()
	FGameplayAbilityTargetingLocationInfo StartLocation;
	ECollisionChannel TraceChannel = ECC_Visibility;
	float InteractionTraceRange = 1000.f;
	float InteractionTraceRate = 0.1f;
	bool bShowDebug = false;

	FTimerHandle TraceTimerHandle;
	TArray<FA1InteractionInfo> CurrentInteractionInfos;
};
