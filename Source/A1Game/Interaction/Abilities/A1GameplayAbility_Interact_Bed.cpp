// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "Interaction/Abilities/A1GameplayAbility_Interact_Bed.h"

#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/A1CharacterAttributeSet.h"
#include "Actors/A1BedBase.h"
#include "Actors/A1DayNightManager.h"
#include "Character/LyraCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Player/LyraPlayerController.h"
#include "Score/A1ScoreManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1GameplayAbility_Interact_Bed)

UA1GameplayAbility_Interact_Bed::UA1GameplayAbility_Interact_Bed(const FObjectInitializer& ObjectInitializer)
    :Super(ObjectInitializer)
{
}

void UA1GameplayAbility_Interact_Bed::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
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

    AA1BedBase* BedActor = Cast<AA1BedBase>(InteractableActor);
    if (BedActor == nullptr)
    {
        CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
        return;
    }

    ALyraCharacter* LyraCharacter = Cast<ALyraCharacter>(GetAvatarActorFromActorInfo());
    if (LyraCharacter == nullptr)
    {
        CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
        return;
    }

    //이미 선점한 내가 아닌 캐릭터가 존재하는 경우
    if (BedActor->bIsOccupyingCharacterExist() && BedActor->GetOccupyingCharacter() != LyraCharacter)
    {
        CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
        return;
    }
	bool bDoTutorial = UA1ScoreManager::Get()->GetDoTutorial();
	if (!bDoTutorial)
	{
		//캐릭터의 피로도가 10 이하인데 들어오려고 시도하는 경우
		ULyraAbilitySystemComponent* ASC = LyraCharacter->GetLyraAbilitySystemComponent();
		if ( ASC )
		{
			const UA1CharacterAttributeSet* Attribute = ASC->GetSet<UA1CharacterAttributeSet>();

			if ( Attribute->GetWeight() <= 10.f && BedActor->GetBedState() == EBedState::Empty )
			{
				CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
				return;
			}
		}
	}

	if ( bDoTutorial && BedActor->GetUsedInTutoMode())
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

    AActor* Actor = UGameplayStatics::GetActorOfClass(GetWorld(), AA1DayNightManager::StaticClass());
    if (Actor == nullptr)
    {
        CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
        return;
    }

    switch (BedActor->GetBedState())
    {
    case EBedState::Empty:
    {
        LyraCharacter->GetCharacterMovement()->SetMovementMode(MOVE_None);
        BedActor->StorePlayerReturnTransform(LyraCharacter->GetActorTransform());

		if ( !UA1ScoreManager::Get()->GetDoTutorial() )
			PlayScreenFadeEffect(false);

        FTransform BedTransform = BedActor->GetLayDownTransform();
        LyraCharacter->SetActorTransform(BedTransform);

        //TODO eric1306: why rotation?

        //Temp Code eric1306: Character get BedActor's Transform -> Set Character Scale
        LyraCharacter->SetActorScale3D(FVector(1.f, 1.f, 1.f));

        BedActor->SetBedState(EBedState::Occupied);

        // 추가: 침대에 캐릭터 정보 저장
        BedActor->SetOccupyingCharacter(LyraCharacter);

        // 추가: DayNightManager에 침대 등록
        if (AA1DayNightManager* DayNightManager = Cast<AA1DayNightManager>(Actor))
        {
            DayNightManager->SetPlayerSleeping(LyraCharacter, true);
        }
    }

    break;

    case EBedState::Occupied:
    {
        LyraCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Walking);

		if ( !UA1ScoreManager::Get()->GetDoTutorial() )
			PlayScreenFadeEffect(true);

        FTransform ReturnTransform = BedActor->GetPlayerReturnTransform();
        LyraCharacter->SetActorTransform(ReturnTransform);

        BedActor->SetBedState(EBedState::Empty);

        // 추가: 침대에 캐릭터 정보 해제
        BedActor->SetOccupyingCharacter(nullptr);

        // 추가: DayNightManager에서 침대 등록 해제
        if (AA1DayNightManager* DayNightManager = Cast<AA1DayNightManager>(Actor))
        {
            DayNightManager->SetPlayerSleeping(LyraCharacter, false);
        }
		
		BedActor->SetUsedInTutoMode(true);
    }

    break;
    }

    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
