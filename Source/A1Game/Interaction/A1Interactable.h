#pragma once

#include "AbilitySystemBlueprintLibrary.h"
#include "A1InteractionInfo.h"
#include "A1InteractionQuery.h"
#include "AbilitySystem/Attributes/A1CombatSet.h"
#include "UObject/Interface.h"
#include "A1Interactable.generated.h"

class IA1Interactable;
struct FA1InteractionInfo;
struct FA1InteractionQuery;

class FA1InteractionInfoBuilder
{
public:
	FA1InteractionInfoBuilder(TScriptInterface<IA1Interactable> InInteractable, TArray<FA1InteractionInfo>& InInteractionInfos)
		: Interactable(InInteractable)
		, InteractionInfos(InInteractionInfos) {
	}

public:
	void AddInteractionInfo(const FA1InteractionInfo& InteractionInfo)
	{
		FA1InteractionInfo& Entry = InteractionInfos.Add_GetRef(InteractionInfo);
		Entry.Interactable = Interactable;
	}

private:
	TScriptInterface<IA1Interactable> Interactable;
	TArray<FA1InteractionInfo>& InteractionInfos;
};

UINTERFACE(MinimalAPI, BlueprintType, meta = (CannotImplementInterfaceInBlueprint))
class UA1Interactable : public UInterface
{
	GENERATED_BODY()
};

class IA1Interactable
{
	GENERATED_BODY()

public:
	virtual FA1InteractionInfo GetPreInteractionInfo(const FA1InteractionQuery& InteractionQuery) const { return FA1InteractionInfo(); }

	virtual void GatherPostInteractionInfos(const FA1InteractionQuery& InteractionQuery, FA1InteractionInfoBuilder& InteractionInfoBuilder) const
	{
		FA1InteractionInfo InteractionInfo = GetPreInteractionInfo(InteractionQuery);

		if (UAbilitySystemComponent* AbilitySystem = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InteractionQuery.RequestingAvatar.Get()))
		{
			float Resourcefulness = AbilitySystem->GetNumericAttribute(UA1CombatSet::GetResourcefulnessAttribute());
			InteractionInfo.Duration = FMath::Max<float>(0.f, InteractionInfo.Duration - Resourcefulness * 0.01f);
		}

		InteractionInfoBuilder.AddInteractionInfo(InteractionInfo);
	}

	virtual void CustomizeInteractionEventData(const FGameplayTag& InteractionEventTag, FGameplayEventData& InOutEventData) const {}

	UFUNCTION(BlueprintCallable)
	virtual void GetMeshComponents(TArray<UMeshComponent*>& OutMeshComponents) const {}

	UFUNCTION(BlueprintCallable)
	virtual bool CanInteraction(const FA1InteractionQuery& InteractionQuery) const { return true; }
};
