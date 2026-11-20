#pragma once

#include "A1ItemData.generated.h"

class UA1ItemTemplate;

UCLASS(BlueprintType, Const, meta=(DisplayName="A1 Item Data"))
class UA1ItemData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	static const UA1ItemData& Get();

public:
#if WITH_EDITORONLY_DATA
	virtual void PreSave(FObjectPreSaveContext SaveContext) override;
#endif // WITH_EDITORONLY_DATA
	
#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif // WITH_EDITOR
	
public:
	const UA1ItemTemplate& FindItemTemplateByID(int32 ItemTemplateID) const;
	int32 FindItemTemplateIDByClass(TSubclassOf<UA1ItemTemplate> ItemTemplateClass) const;
	
	void GetAllItemTemplateClasses(TArray<TSubclassOf<UA1ItemTemplate>>& OutItemTemplateClasses) const;
	const TArray<TSubclassOf<UA1ItemTemplate>>& GetWeaponItemTemplateClasses() const { return WeaponItemTemplateClasses; }
	const TArray<TSubclassOf<UA1ItemTemplate>>& GetArmorItemTemplateClasses() const { return ArmorItemTemplateClasses; }
	
private:
	UPROPERTY(EditDefaultsOnly)
	TMap<int32, TSubclassOf<UA1ItemTemplate>> ItemTemplateIDToClass;

	UPROPERTY()
	TMap<TSubclassOf<UA1ItemTemplate>, int32> ItemTemplateClassToID;

private:
	UPROPERTY()
	TArray<TSubclassOf<UA1ItemTemplate>> WeaponItemTemplateClasses;

	UPROPERTY()
	TArray<TSubclassOf<UA1ItemTemplate>> ArmorItemTemplateClasses;
};
