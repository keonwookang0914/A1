// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "Interaction/Abilities/A1GameplayAbility_Interact_Chest.h"

#include "Actors/A1ChestBase.h"

UA1GameplayAbility_Interact_Chest::UA1GameplayAbility_Interact_Chest(const FObjectInitializer& ObjectInitializer)
    :Super(ObjectInitializer)
{
}

void UA1GameplayAbility_Interact_Chest::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    if (TriggerEventData == nullptr || bInitialized == false)
    {
        CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
        return;
    }

    if (HasAuthority(&CurrentActivationInfo) == false)
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        return;
    }

    AA1ChestBase* Chest = Cast<AA1ChestBase>(InteractableActor);
    if (Chest == nullptr)
    {
        CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
        return;
    }

    if (Chest->GetChestState() == EChestState::Close)
    {
        Chest->SetChestState(EChestState::Open);
    }
}