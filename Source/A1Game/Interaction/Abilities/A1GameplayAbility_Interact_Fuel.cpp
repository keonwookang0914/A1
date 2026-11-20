// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "Interaction/Abilities/A1GameplayAbility_Interact_Fuel.h"

#include "Actors/A1FuelBase.h"

UA1GameplayAbility_Interact_Fuel::UA1GameplayAbility_Interact_Fuel(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

void UA1GameplayAbility_Interact_Fuel::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
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

    AA1FuelBase* FuelActor = Cast<AA1FuelBase>(InteractableActor);
    if (FuelActor == nullptr)
    {
        CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
        return;
    }

    // 연료 감지 활성화 - 대부분의 상호작용은 자동으로 이루어지므로 여기서는 특별한 작업 없음
    // 필요하다면 수동으로 연료 감지 트리거 가능
    FuelActor->DetectAndAbsorbFuelItems();

    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);

}
