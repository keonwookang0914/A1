// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidget.h"
#include "A1HoverWidget.generated.h"

class UCanvasPanel;
class UHorizontalBox;

UCLASS()
class A1GAME_API UA1HoverWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UA1HoverWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	void SetPosition(const FVector2D& AbsolutePosition);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> CanvasPanel_Root;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> HorizontalBox_Hovers;
};
