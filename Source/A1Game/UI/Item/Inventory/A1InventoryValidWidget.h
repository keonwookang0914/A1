// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "A1Define.h"
#include "Blueprint/UserWidget.h"
#include "A1InventoryValidWidget.generated.h"

class UImage;
class USizeBox;

UCLASS()
class A1GAME_API UA1InventoryValidWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UA1InventoryValidWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void NativeOnInitialized() override;

public:
	void ChangeSlotState(ESlotState SlotState);

public:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<USizeBox> SizeBox_Root;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_Red;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_Green;
};
