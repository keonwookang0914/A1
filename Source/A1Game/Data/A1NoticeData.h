#pragma once

#include "A1NoticeData.generated.h"


USTRUCT()
struct FNoticeTextSet
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
	TArray<FText> TextEntries;
};

UCLASS(Const, CollapseCategories, meta=(DisplayName="A1 Notice Data"))
class UA1NoticeData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	static const UA1NoticeData& Get();
	
protected:
#if WITH_EDITOR
	virtual void PreSave(FObjectPreSaveContext ObjectSaveContext) override;
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
	
public:
	const FNoticeTextSet& GetTextSetByLabel(const FName& Label) const;
	
private:
	UPROPERTY(EditDefaultsOnly)
	TMap<FName, FNoticeTextSet> TextGroupNameToSet;
};
