#pragma once

#include "EnhancedPlayerInput.h"
#include "A1EnhancedPlayerInput.generated.h"

UCLASS()
class UA1EnhancedPlayerInput : public UEnhancedPlayerInput
{
	GENERATED_BODY()
	
public:
	UA1EnhancedPlayerInput();

public:
	void FlushPressedInput(UInputAction* InputAction);
};
