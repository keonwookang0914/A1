#include "Interaction/Abilities/A1GameplayAbility_Interact_Info.h"

#include "Interaction/A1Interactable.h"
#include "Interaction/A1InteractionQuery.h"

UA1GameplayAbility_Interact_Info::UA1GameplayAbility_Interact_Info(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

bool UA1GameplayAbility_Interact_Info::InitializeAbility(AActor* TargetActor)
{
	TScriptInterface<IA1Interactable> TargetInteractable(TargetActor);
	if (TargetInteractable)
	{
		FA1InteractionQuery InteractionQuery;
		InteractionQuery.RequestingAvatar = GetAvatarActorFromActorInfo();
		InteractionQuery.RequestingController = GetControllerFromActorInfo();

		Interactable = TargetInteractable;
		InteractableActor = TargetActor;

		TArray<FA1InteractionInfo> InteractionInfos;
		FA1InteractionInfoBuilder InteractionInfoBuilder(Interactable, InteractionInfos);
		Interactable->GatherPostInteractionInfos(InteractionQuery, InteractionInfoBuilder);
		InteractionInfo = InteractionInfos[0];

		return true;
	}

	return false;
}
