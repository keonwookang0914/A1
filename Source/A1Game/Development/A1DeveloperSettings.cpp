// Copyright Epic Games, Inc. All Rights Reserved.

#include "A1DeveloperSettings.h"
#include "Misc/App.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1DeveloperSettings)

#define LOCTEXT_NAMESPACE "A1Cheats"

UA1DeveloperSettings::UA1DeveloperSettings()
{
}

FName UA1DeveloperSettings::GetCategoryName() const
{
	return FApp::GetProjectName();
}

#if WITH_EDITOR
void UA1DeveloperSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	ApplySettings();
}

void UA1DeveloperSettings::PostReloadConfig(FProperty* PropertyThatWasLoaded)
{
	Super::PostReloadConfig(PropertyThatWasLoaded);

	ApplySettings();
}

void UA1DeveloperSettings::PostInitProperties()
{
	Super::PostInitProperties();

	ApplySettings();
}

void UA1DeveloperSettings::ApplySettings()
{
}

void UA1DeveloperSettings::OnPlayInEditorStarted() const
{
}
#endif

#undef LOCTEXT_NAMESPACE
