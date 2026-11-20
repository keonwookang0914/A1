// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "AbilitySystem/Abilities/Docker/A1GameplayAbility_Sprint.h"

#include "A1GameplayTags.h"
#include "AbilitySystemComponent.h"
#include "Character/LyraCharacter.h"
#include "Character/LyraCharacterMovementComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1GameplayAbility_Sprint)

UA1GameplayAbility_Sprint::UA1GameplayAbility_Sprint(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	AbilityTags.AddTag(A1GameplayTags::Ability_Sprint_Active);
	ActivationOwnedTags.AddTag(A1GameplayTags::Status_Sprint);

	bReplicateInputDirectly = true;

	OriginalMaxSpeed = 0.0f;
	bIsSprintActive = false;
}

void UA1GameplayAbility_Sprint::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
    UE_LOG(LogTemp, Log, TEXT("Call Sprint Activate Ability"));
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    ALyraCharacter* LyraCharacter = Cast<ALyraCharacter>(ActorInfo->AvatarActor.Get());
    if (!LyraCharacter)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    UAbilitySystemComponent* ASc = LyraCharacter->GetAbilitySystemComponent();
    if (!ASc)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    ULyraCharacterMovementComponent* MovementComponent = Cast<ULyraCharacterMovementComponent>(LyraCharacter->GetCharacterMovement());
    if (MovementComponent)
    {
        OriginalMaxSpeed = MovementComponent->MaxWalkSpeed;
        MovementComponent->MaxWalkSpeed = OriginalMaxSpeed * 1.8f;
    }

    if (SprintEffectClass)
    {
        FGameplayEffectContextHandle EffectContext = ASc->MakeEffectContext();
        EffectContext.AddSourceObject(this);

        SprintEffectHandle = ASc->ApplyGameplayEffectToSelf(
            SprintEffectClass->GetDefaultObject<UGameplayEffect>(),
            1.0f,
            EffectContext
        );
    }
    bIsSprintActive = true;
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UA1GameplayAbility_Sprint::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
    UE_LOG(LogTemp, Log, TEXT("Call Sprint End Ability"));
    if (!bIsSprintActive)
    {
        return;
    }

    ALyraCharacter* Character = Cast<ALyraCharacter>(ActorInfo->AvatarActor.Get());
    if (Character)
    {
        ULyraCharacterMovementComponent* MovementComponent = Cast<ULyraCharacterMovementComponent>(Character->GetCharacterMovement());
        if (MovementComponent && OriginalMaxSpeed > 0.0f)
        {
            MovementComponent->MaxWalkSpeed = OriginalMaxSpeed;
        }
    }

    if (ActorInfo->AbilitySystemComponent.IsValid() && SprintEffectHandle.IsValid())
    {
        ActorInfo->AbilitySystemComponent->RemoveActiveGameplayEffect(SprintEffectHandle);
        SprintEffectHandle.Invalidate();
    }
    bIsSprintActive = false;

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UA1GameplayAbility_Sprint::InputPressed(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
    Super::InputPressed(Handle, ActorInfo, ActivationInfo);

    if (!bIsSprintActive && ActorInfo && ActorInfo->AbilitySystemComponent.IsValid())
    {
        ActorInfo->AbilitySystemComponent->TryActivateAbility(Handle);
    }
}

void UA1GameplayAbility_Sprint::InputReleased(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
    Super::InputReleased(Handle, ActorInfo, ActivationInfo);

    if (bIsSprintActive && ActorInfo != nullptr)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
    }
}
