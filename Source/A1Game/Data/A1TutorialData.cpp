// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "Data/A1TutorialData.h"

#include "System/LyraAssetManager.h"
#include "UObject/ObjectSaveContext.h"

const UA1TutorialData& UA1TutorialData::Get()
{
	return ULyraAssetManager::Get().GetTutorialData();
}
#if WITH_EDITOR
void UA1TutorialData::PreSave(FObjectPreSaveContext ObjectSaveContext)
{
	Super::PreSave(ObjectSaveContext);

}
#endif //WITH_EDITOR

FA1TutorialStepInfo UA1TutorialData::GetTutorialStepInfoByStepName(FString InStepID)
{
	for (FA1TutorialStepInfo TutorialStepInfo : TutorialStepInfos )
	{
		if (TutorialStepInfo.StepID == InStepID)
		{
			return TutorialStepInfo;
		}
	}

	return FA1TutorialStepInfo();
}

