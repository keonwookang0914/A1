#pragma once

#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "Interaction/A1InteractionInfo.h"
#include "A1GameplayAbility_Interact_Info.generated.h"

UCLASS()
class UA1GameplayAbility_Interact_Info : public ULyraGameplayAbility
{
	GENERATED_BODY()
public:
	UA1GameplayAbility_Interact_Info(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	UFUNCTION(BlueprintCallable)
	bool InitializeAbility(AActor* TargetActor);

protected:
	UPROPERTY(BlueprintReadOnly)
	TScriptInterface<IA1Interactable> Interactable;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> InteractableActor;

	UPROPERTY(BlueprintReadOnly)
	FA1InteractionInfo InteractionInfo;
};
