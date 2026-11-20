#include "A1GameplayAbility_Utility_FlashLight_Charge.h"

#include "A1GameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Actors/A1UtilityBase.h"
#include "Item/Managers/A1EquipmentManagerComponent.h"
#include "Item/Managers/A1InventoryManagerComponent.h"
#include "Item/Managers/A1ItemManagerComponent.h"
#include "Player/LyraPlayerController.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1GameplayAbility_Utility_FlashLight_Charge)

UA1GameplayAbility_Utility_FlashLight_Charge::UA1GameplayAbility_Utility_FlashLight_Charge(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ActivationOwnedTags.AddTag(A1GameplayTags::Status_ActiveUtility);
	ActivationRequiredTags.AddTag(A1GameplayTags::Status_MainHand_Left);
	ActivationRequiredTags.AddTag(A1GameplayTags::Status_FlashLight_ChargingZone);
	//ActivationBlockedTags.AddTag(A1GameplayTags::Status_MainHand_Right);
}

void UA1GameplayAbility_Utility_FlashLight_Charge::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("ChargeFlashLight"), ChargeMontage, GetSnapshottedAttackRate(), NAME_None, false, 1.f, 0.f, false))
	{
		PlayMontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageFinished);
		PlayMontageTask->ReadyForActivation();
	}
}

void UA1GameplayAbility_Utility_FlashLight_Charge::OnMontageFinished()
{
	if (HasAuthority(&CurrentActivationInfo))
	{
		AA1UtilityBase* flashlight = Cast<AA1UtilityBase>(GetFirstEquipmentActor());
		flashlight->ChargeGauge();

		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}