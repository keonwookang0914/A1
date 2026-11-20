#include "A1GuideData.h"

#include "UObject/ObjectSaveContext.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include "A1LogChannels.h"
#include "System/LyraAssetManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1GuideData)


const UA1GuideData& UA1GuideData::Get()
{
	return ULyraAssetManager::Get().GetGuideData();
}

#if WITH_EDITOR
void UA1GuideData::PreSave(FObjectPreSaveContext ObjectSaveContext)
{
	Super::PreSave(ObjectSaveContext);

	TextGroupNameToSet.KeySort([](const FName& A, const FName& B)
	{
		return (A.Compare(B) < 0);
	});
	
}

EDataValidationResult UA1GuideData::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	for (const auto& Pair : TextGroupNameToSet)
	{
		const FGuideTextSet& TextSet = Pair.Value;
		for (int32 i = 0; i < TextSet.TextEntries.Num(); i++)
		{
			const FText& Text = TextSet.TextEntries[i];
			if (Text.IsEmpty())
			{
				Context.AddError(FText::FromString(FString::Printf(TEXT("Text is None : [Group Name : %s] - [Entry Index : %d]"), *Pair.Key.ToString(), i)));
				Result = EDataValidationResult::Invalid;
			}
		}
	}

	return Result;
}
#endif // WITH_EDITOR

const FGuideTextSet& UA1GuideData::GetTextSetByLabel(const FName& Label) const
{
	const FGuideTextSet* TextSet = TextGroupNameToSet.Find(Label);
	ensureAlwaysMsgf(TextSet, TEXT("Can't find Asset Set from Label [%s]."), *Label.ToString());
	return *TextSet;
}
