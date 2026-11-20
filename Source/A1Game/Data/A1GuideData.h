#pragma once

#include "A1GuideData.generated.h"


USTRUCT()
struct FGuideTextSet
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
	TArray<FText> TextEntries;
};

UCLASS(Const, CollapseCategories, meta=(DisplayName="A1 Guide Data"))
class UA1GuideData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	static const UA1GuideData& Get();
	
protected:
#if WITH_EDITOR
	virtual void PreSave(FObjectPreSaveContext ObjectSaveContext) override;
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
	
public:
	const FGuideTextSet& GetTextSetByLabel(const FName& Label) const;
	
private:
	UPROPERTY(EditDefaultsOnly)
	TMap<FName, FGuideTextSet> TextGroupNameToSet;
};
