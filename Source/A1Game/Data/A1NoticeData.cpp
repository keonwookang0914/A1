#include "A1NoticeData.h"

#include "UObject/ObjectSaveContext.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include "System/LyraAssetManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1NoticeData)

const UA1NoticeData& UA1NoticeData::Get()
{
	return ULyraAssetManager::Get().GetNoticeData();
}

#if WITH_EDITOR
void UA1NoticeData::PreSave(FObjectPreSaveContext ObjectSaveContext)
{
	Super::PreSave(ObjectSaveContext);

	TextGroupNameToSet.KeySort([](const FName& A, const FName& B)
	{
		return (A.Compare(B) < 0);
	});
	
}

EDataValidationResult UA1NoticeData::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	for (const auto& Pair : TextGroupNameToSet)
	{
		const FNoticeTextSet& TextSet = Pair.Value;
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

const FNoticeTextSet& UA1NoticeData::GetTextSetByLabel(const FName& Label) const
{
	const FNoticeTextSet* TextSet = TextGroupNameToSet.Find(Label);
	ensureAlwaysMsgf(TextSet, TEXT("Can't find Asset Set from Label [%s]."), *Label.ToString());
	return *TextSet;
}
