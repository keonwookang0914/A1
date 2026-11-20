// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "Interaction/Abilities/A1GameplayAbility_Interact_Repair.h"

#include "A1GameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Actors/A1RepairBase.h"
#include "Actors/A1EquipmentBase.h"
#include "Character/LyraCharacter.h"
#include "Data/A1ItemData.h"
#include "Item/Managers/A1EquipManagerComponent.h"
#include "Item/Managers/A1EquipmentManagerComponent.h"
#include "Item/Fragments/A1ItemFragment_Equipable_Utility.h"
#include "Player/LyraPlayerController.h"
#include "Score/A1ScoreBlueprintFunctionLibrary.h"
#include "Score/A1ScoreManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1GameplayAbility_Interact_Repair)

UA1GameplayAbility_Interact_Repair::UA1GameplayAbility_Interact_Repair(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
    AbilityTags.AddTag(A1GameplayTags::Ability_Interact_Repair);
    ActivationOwnedTags.AddTag(A1GameplayTags::Status_Repair);
    ActivationRequiredTags.AddTag(A1GameplayTags::Status_MainHand_Left);
    ActivationBlockedTags.AddTag(A1GameplayTags::Status_MainHand_Right);
}

void UA1GameplayAbility_Interact_Repair::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
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

    AA1RepairBase* RepairActor = Cast<AA1RepairBase>(InteractableActor);
    if (RepairActor == nullptr || RepairActor->CurrentState != RepairState::Break && RepairActor->CurrentState != RepairState::Foamed)
    {
        CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
        return;
    }

    if (!CheckHoldRefairKit())
    {
        CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
        return;
    }

    DoRepair();
}

bool UA1GameplayAbility_Interact_Repair::CheckHoldRefairKit()
{
    ALyraCharacter* Character = Cast<ALyraCharacter>(GetAvatarActorFromActorInfo());
    UA1EquipManagerComponent* EquipManager = Character->FindComponentByClass<UA1EquipManagerComponent>();
    if (Character == nullptr || EquipManager == nullptr)
    {
        CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
        return false;
    }

    AA1EquipmentBase* RefairKit = EquipManager->GetEquippedActor(EEquipmentSlotType::LeftHand);
    const UA1ItemTemplate& ItemTemplate = UA1ItemData::Get().FindItemTemplateByID(RefairKit->GetTemplateID());
    const UA1ItemFragment_Equipable_Utility* ItemFragment = ItemTemplate.FindFragmentByClass<UA1ItemFragment_Equipable_Utility>();
    if (ItemFragment == nullptr)
        return false;

    if (ItemFragment->UtilityType != EUtilityType::Repairkit)
        return false;

    return true;
}

void UA1GameplayAbility_Interact_Repair::DoRepair()
{
    // 荐府 kit 家葛
    ALyraCharacter* Character = Cast<ALyraCharacter>(GetAvatarActorFromActorInfo());
    UA1EquipmentManagerComponent* EquipmentManager = Character->FindComponentByClass<UA1EquipmentManagerComponent>();
    if (Character == nullptr || EquipmentManager == nullptr)
    {
        CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
        return;
    }
    EquipmentManager->RemoveEquipment_Unsafe(EEquipmentSlotType::LeftHand, 1);
    UA1ScoreBlueprintFunctionLibrary::AddConsumedItems();

    // 荐府 按眉 力芭
    AA1RepairBase* RepairActor = Cast<AA1RepairBase>(InteractableActor);
    RepairActor->SetCurrentState(RepairState::Complete);
    UA1ScoreManager::Get()->SetTotalRepair(UA1ScoreManager::Get()->GetTotalRepair() + 1);
    UA1ScoreManager::Get()->SetCompleteRepair(UA1ScoreManager::Get()->GetCompleteRepair() + 1);

    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
