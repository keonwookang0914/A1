// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "GA/AT/A1AT_JumpAndWaitForLanding.h"
#include "GameFramework/Character.h"

UA1AT_JumpAndWaitForLanding::UA1AT_JumpAndWaitForLanding()
{
}

UA1AT_JumpAndWaitForLanding* UA1AT_JumpAndWaitForLanding::CreateTask(UGameplayAbility* OwningAbility)
{
	UA1AT_JumpAndWaitForLanding* NewTask = NewAbilityTask<UA1AT_JumpAndWaitForLanding>(OwningAbility);

	return NewTask;
}

void UA1AT_JumpAndWaitForLanding::Activate()
{
	Super::Activate();

	ACharacter* Character = CastChecked<ACharacter>(GetAvatarActor());
	Character->LandedDelegate.AddDynamic(this, &UA1AT_JumpAndWaitForLanding::OnLandedCallback);
	Character->Jump();

	//언제 끝날지 모르니
	SetWaitingOnAvatar();
}

void UA1AT_JumpAndWaitForLanding::OnDestroy(bool AbilityEnded)
{
	ACharacter* Character = CastChecked<ACharacter>(GetAvatarActor());
	Character->LandedDelegate.RemoveDynamic(this, &UA1AT_JumpAndWaitForLanding::OnLandedCallback);

	Super::OnDestroy(AbilityEnded);
}

void UA1AT_JumpAndWaitForLanding::OnLandedCallback(const FHitResult& Hit)
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnComplete.Broadcast();
	}
}
