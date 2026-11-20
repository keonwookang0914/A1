// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "A1InventorySlotWidget.generated.h"

class UImage;
class USizeBox;

UCLASS()
class A1GAME_API UA1InventorySlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UA1InventorySlotWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void NativeOnInitialized() override;

public:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<USizeBox> SizeBox_Root;
};
