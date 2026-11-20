#include "Interaction/A1WorldInteractable.h"

#include "A1GameplayTags.h"
#include "Character/LyraCharacter.h"
#include "Net/UnrealNetwork.h"

AA1WorldInteractable::AA1WorldInteractable(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bReplicates = true;
}

void AA1WorldInteractable::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, bIsUsed);
}

bool AA1WorldInteractable::CanInteraction(const FA1InteractionQuery& InteractionQuery) const
{
	return bCanUsed ? (bIsUsed == false) : true;
}

void AA1WorldInteractable::OnInteractActiveStarted(AActor* Interactor)
{
	if (IsValid(Interactor) == false)
		return;

	if (HasAuthority())
	{
		CachedInteractors.Add(Interactor);
	}

	K2_OnInteractActiveStarted(Interactor);
}

void AA1WorldInteractable::OnInteractActiveEnded(AActor* Interactor)
{
	if (IsValid(Interactor) == false)
		return;

	if (HasAuthority())
	{
		CachedInteractors.Remove(Interactor);
	}

	K2_OnInteractActiveEnded(Interactor);
}

void AA1WorldInteractable::OnInteractionSuccess(AActor* Interactor)
{
	if (IsValid(Interactor) == false)
		return;

	if (HasAuthority())
	{
		//Only Server Code
		if (bCanUsed)
		{
			bIsUsed = true;
		}

		for (TWeakObjectPtr<AActor> CachedInteractor : CachedInteractors)
		{
			if (ALyraCharacter* LyraCharacter = Cast<ALyraCharacter>(CachedInteractor.Get()))
			{
				if (Interactor == LyraCharacter)
					continue;

				if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(LyraCharacter))
				{
					FGameplayTagContainer CancelAbilitiesTag;
					CancelAbilitiesTag.AddTag(A1GameplayTags::Ability_Interact_Active);
					ASC->CancelAbilities(&CancelAbilitiesTag);
				}
			}
		}
		CachedInteractors.Empty();
	}
	K2_OnInteractionSuccess(Interactor);
}
