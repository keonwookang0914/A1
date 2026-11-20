#include "A1AttributeSet.h"

#include "AbilitySystem/LyraAbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1AttributeSet)

UA1AttributeSet::UA1AttributeSet()
{
	
}

UWorld* UA1AttributeSet::GetWorld() const
{
	const UObject* Outer = GetOuter();
	check(Outer);

	return Outer->GetWorld();
}

ULyraAbilitySystemComponent* UA1AttributeSet::GetLyraAbilitySystemComponent() const
{
	return Cast<ULyraAbilitySystemComponent>(GetOwningAbilitySystemComponent());
}
