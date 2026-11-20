// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#include "AbilitySystem/Abilities/Docker/A1GameplayAbility_OpenWidget.h"

#include "A1GameplayTags.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"

UA1GameplayAbility_OpenWidget::UA1GameplayAbility_OpenWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bServerRespectsRemoteAbilityCancellation = true;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ClientOrServer;
	ActivationGroup = ELyraAbilityActivationGroup::Independent;
}

void UA1GameplayAbility_OpenWidget::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ULyraAbilitySystemComponent* SourceASC = GetLyraAbilitySystemComponentFromActorInfo();
	if (SourceASC == nullptr)
		return;

	if (WidgetTag.Num() > 0)
	{
		FGameplayTag temp = WidgetTag.GetByIndex(0);
		if (SourceASC->HasMatchingGameplayTag(temp) == false)
		{
			ActiveWidget();
		}
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UA1GameplayAbility_OpenWidget::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	ULyraAbilitySystemComponent* SourceASC = GetLyraAbilitySystemComponentFromActorInfo();
	if (SourceASC == nullptr)
		return;

	if (HasAuthority(&CurrentActivationInfo) == true)
	{
		if (WidgetTag.Num() > 0)
		{
			if (SourceASC->HasAnyMatchingGameplayTags(WidgetTag) == false)
			{
				UAbilitySystemBlueprintLibrary::AddLooseGameplayTags(GetAvatarActorFromActorInfo(), WidgetTag, true);
			}
			else
			{
				UAbilitySystemBlueprintLibrary::RemoveLooseGameplayTags(GetAvatarActorFromActorInfo(), WidgetTag, true);
				DeactiveWidget();
			}
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}