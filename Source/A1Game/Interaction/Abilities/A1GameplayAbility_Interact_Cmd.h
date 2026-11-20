// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "Interaction/Abilities/A1GameplayAbility_Interact_Object.h"
#include "A1GameplayAbility_Interact_Cmd.generated.h"

class UAbilityTask_WaitGameplayEvent;

USTRUCT(BlueprintType)
struct FA1WidgetActiveMessage
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	bool bActive = false;
};

UCLASS()
class UA1GameplayAbility_Interact_Cmd : public UA1GameplayAbility_Interact_Object
{
	GENERATED_BODY()
public:
	UA1GameplayAbility_Interact_Cmd();
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
public:
	UFUNCTION(BlueprintImplementableEvent)
	void ActiveWidget();
	UFUNCTION(BlueprintImplementableEvent)
	void DeactiveWidget();

protected:
	UFUNCTION()
	void CloseCmd(FGameplayEventData Payload);

protected:
	UPROPERTY(BlueprintReadOnly)
	bool bTutorialMode = false;

private:
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> ExitWidgetTask;
};
