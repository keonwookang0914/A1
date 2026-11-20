#pragma once

#include "A1Define.h"
#include "Engine/DataAsset.h"
#include "A1AbilityData.generated.h"

class UA1ItemTemplate;
class UGameplayAbility;
class UGameplayEffect;

UCLASS(BlueprintType, Const, meta = (DisplayName = "A1 Ability Data"))
class A1GAME_API UA1AbilityData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	static const UA1AbilityData& Get();

public:
#if WITH_EDITOR
	virtual void PreSave(FObjectPreSaveContext ObjectSaveContext) override;
#endif // WITH_EDITOR

public:
	const UGameplayAbility& GetGameplayAbility(FName Label) const;
	const TSubclassOf<UGameplayEffect> GetGameplayEffect(FName Label) const;

private:
	UPROPERTY(EditDefaultsOnly)
	TMap<FName, TSubclassOf<UGameplayAbility>> AbilityEntries;

	UPROPERTY(EditDefaultsOnly)
	TMap<FName, TSubclassOf<UGameplayEffect>> EffectEntries;
};