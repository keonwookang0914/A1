#pragma once

#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "Interaction/A1InteractionInfo.h"
#include "A1GameplayAbility_Interact.generated.h"

USTRUCT(BlueprintType)
struct FA1InteractionMessage
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<AActor> Instigator = nullptr;

	UPROPERTY(BlueprintReadWrite)
	bool bShouldRefresh = false;

	UPROPERTY(BlueprintReadWrite)
	bool bSwitchActive = false;

	UPROPERTY(BlueprintReadWrite)
	FA1InteractionInfo InteractionInfo = FA1InteractionInfo();
};

UCLASS()
class UA1GameplayAbility_Interact : public ULyraGameplayAbility
{
	GENERATED_BODY()

public:
	UA1GameplayAbility_Interact(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	UFUNCTION(BlueprintCallable)
	void UpdateInteractions(const TArray<FA1InteractionInfo>& InteractionInfos);

	UFUNCTION(BlueprintCallable)
	void TriggerInteraction();

private:
	void WaitInputStart();

	UFUNCTION()
	void OnInputStart();

protected:
	UPROPERTY(BlueprintReadWrite)
	TArray<FA1InteractionInfo> CurrentInteractionInfos;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "A1|Interaction")
	float InteractionTraceRange = 1000.f; //Temp eric1306 after FP, must fix distance

	UPROPERTY(EditDefaultsOnly, Category = "A1|Interaction")
	float InteractionTraceRate = 0.1f;

	UPROPERTY(EditDefaultsOnly, Category = "A1|Interaction")
	float InteractionScanRange = 1000.f;//Temp eric1306 after FP, must fix distance

	UPROPERTY(EditDefaultsOnly, Category = "A1|Interaction")
	float InteractionScanRate = 0.1f;

	UPROPERTY(EditDefaultsOnly, Category = "A1|Interaction")
	bool bShowTraceDebug = false;

	UPROPERTY(EditDefaultsOnly, Category = "A1|Interaction")
	TSoftClassPtr<UUserWidget> DefaultInteractionWidgetClass;
};
