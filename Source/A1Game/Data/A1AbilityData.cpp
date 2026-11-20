#include "Data/A1AbilityData.h"
#include "System/LyraAssetManager.h"
#include "UObject/ObjectSaveContext.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystemComponent.h"

const UA1AbilityData& UA1AbilityData::Get()
{
	return ULyraAssetManager::Get().GetAbilityData();
}

#if WITH_EDITOR
void UA1AbilityData::PreSave(FObjectPreSaveContext ObjectSaveContext)
{
	Super::PreSave(ObjectSaveContext);

	AbilityEntries.KeySort([](const FName A, const FName B)
		{
			return (A.GetStringLength() < B.GetStringLength());
		});
}
#endif // WITH_EDITOR

const UGameplayAbility& UA1AbilityData::GetGameplayAbility(FName Label) const
{
	const TSubclassOf<UGameplayAbility>* AbilityClass = AbilityEntries.Find(Label);
	ensureAlwaysMsgf(AbilityClass, TEXT("Can't find AbilityClass from Label [%s]"), *Label.ToString());
	return *(AbilityClass->GetDefaultObject());
}

const TSubclassOf<UGameplayEffect> UA1AbilityData::GetGameplayEffect(FName Label) const
{
	const TSubclassOf<UGameplayEffect>* EffectClass = EffectEntries.Find(Label);
	ensureAlwaysMsgf(EffectClass, TEXT("Can't find EffectClass from Label [%s]"), *Label.ToString());
	return *EffectClass;
}