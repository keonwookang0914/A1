// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "GA/A1GA_Jump.h"
#include "GameFramework/Character.h"
#include "GA/AT/A1AT_JumpAndWaitForLanding.h"

UA1GA_Jump::UA1GA_Jump()
{
	//Set Policy
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UA1GA_Jump::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	UA1AT_JumpAndWaitForLanding* JumpAndWaitingForLandingTask = UA1AT_JumpAndWaitForLanding::CreateTask(this);

	JumpAndWaitingForLandingTask->OnComplete.AddDynamic(this, &UA1GA_Jump::OnEndedCallback);
	JumpAndWaitingForLandingTask->ReadyForActivation();
}

void UA1GA_Jump::InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputPressed(Handle, ActorInfo, ActivationInfo);
	ACharacter* Character = CastChecked<ACharacter>(ActorInfo->AvatarActor.Get());
	Character->StopJumping();
}

bool UA1GA_Jump::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	bool bResult = Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
	if (!bResult)
	{
		return false;
	}
	//약포인터라 Get 사용
	const ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	return (Character && Character->CanJump());

}

void UA1GA_Jump::OnEndedCallback()
{
	//어빌리티 종료구문 추가
	bool bReplicatedEndAbility = true;
	bool bWasCancelled = false;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicatedEndAbility, bWasCancelled);
}
