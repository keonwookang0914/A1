#pragma once

#include "A1ItemFragment_Equipable_Attachment.h"
#include "A1ItemFragment_Equipable_Utility.generated.h"

class UGameplayEffect;

UCLASS()
class UA1ItemFragment_Equipable_Utility : public UA1ItemFragment_Equipable_Attachment
{
	GENERATED_BODY()

public:
	UA1ItemFragment_Equipable_Utility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
#if WITH_EDITORONLY_DATA
	virtual void PreSave(FObjectPreSaveContext SaveContext) override;
#endif // WITH_EDITORONLY_DATA

public:
	virtual void OnInstanceCreated(UA1ItemInstance* ItemInstance) const override;

public:
	UPROPERTY(EditDefaultsOnly)
	EUtilityType UtilityType = EUtilityType::Count;

	UPROPERTY(EditDefaultsOnly)
	TArray<TSubclassOf<UGameplayEffect>> UtilityEffectClasses;
	
	UPROPERTY(EditDefaultsOnly, meta=(ForceInlineRow))
	TArray<FRarityStatSet> RarityStatSets;
};
