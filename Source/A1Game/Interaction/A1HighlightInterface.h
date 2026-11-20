// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "A1HighlightInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UA1HighlightInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class A1GAME_API IA1HighlightInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void Highlight() = 0;
	virtual void UnHighlight() = 0;
};
