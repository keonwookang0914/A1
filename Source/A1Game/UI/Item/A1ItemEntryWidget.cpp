// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#include "A1ItemEntryWidget.h"

#include "AbilitySystemGlobals.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Data/A1ItemData.h"
#include "Data/A1UIData.h"
#include "Item/A1ItemInstance.h"
#include "Item/A1ItemTemplate.h"

UA1ItemEntryWidget::UA1ItemEntryWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UA1ItemEntryWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	Text_Count->SetText(FText::GetEmpty());
}

void UA1ItemEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Image_Hover->SetVisibility(ESlateVisibility::Hidden);
}

void UA1ItemEntryWidget::NativeDestruct()
{
	//if (HoverWidget)
	//{
	//	HoverWidget->RemoveFromParent();
	//	HoverWidget = nullptr;
	//}

	Super::NativeDestruct();
}

void UA1ItemEntryWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	Image_Hover->SetVisibility(ESlateVisibility::Visible);

	//if (HoverWidget == nullptr)
	//{
	//	TSubclassOf<UA1ItemHoverWidget> HoverWidgetClass = UA1UIData::Get().ItemHoverWidgetClass;
	//	HoverWidget = CreateWidget<UA1ItemHoverWidget>(GetOwningPlayer(), HoverWidgetClass);
	//}

	//if (HoverWidget)
	//{
	//	HoverWidget->RefreshUI(ItemInstance);
	//	HoverWidget->AddToViewport();
	//}
}

FReply UA1ItemEntryWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseMove(InGeometry, InMouseEvent);

	//if (HoverWidget)
	//{
	//	HoverWidget->SetPosition(InMouseEvent.GetScreenSpacePosition());
	//	return FReply::Handled();
	//}

	return Reply;
}

void UA1ItemEntryWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);

	Image_Hover->SetVisibility(ESlateVisibility::Hidden);

	//if (HoverWidget)
	//{
	//	HoverWidget->RemoveFromParent();
	//	HoverWidget = nullptr;
	//}
}

FReply UA1ItemEntryWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		Reply.DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
	}
	return Reply;
}

void UA1ItemEntryWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	RefreshWidgetOpacity(false);

	// Tag를 가지고 있지 않다면 Drag 중이라는 태그 부여
	FGameplayTag TagToCheck = FGameplayTag::RequestGameplayTag(FName("GameplayEvent.Inventory.Drag"));
	bool HasTag = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwningPlayerPawn())->HasMatchingGameplayTag(TagToCheck);
	if (!HasTag)
	{
		FGameplayTagContainer TagContainer;
		TagContainer.AddTag(TagToCheck);
		UAbilitySystemBlueprintLibrary::AddLooseGameplayTags(GetOwningPlayerPawn(), TagContainer, true);
	}
}

void UA1ItemEntryWidget::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragCancelled(InDragDropEvent, InOperation);

	RefreshWidgetOpacity(true);
}

void UA1ItemEntryWidget::RefreshWidgetOpacity(bool bClearlyVisible)
{
	SetRenderOpacity(bClearlyVisible ? 1.f : 0.5f);
}

void UA1ItemEntryWidget::RefreshUI(UA1ItemInstance* NewItemInstance, int32 NewItemCount)
{
	if (NewItemInstance == nullptr || NewItemCount < 1)
		return;

	ItemInstance = NewItemInstance;
	ItemCount = NewItemCount;

	const UA1ItemTemplate& ItemTemplate = UA1ItemData::Get().FindItemTemplateByID(ItemInstance->GetItemTemplateID());
	Image_Icon->SetBrushFromTexture(ItemTemplate.IconTexture, true);
	Text_Count->SetText(ItemCount <= 1 ? FText::GetEmpty() : FText::AsNumber(ItemCount));

	UTexture2D* RarityTexture = UA1UIData::Get().GetEntryRarityTexture(ItemInstance->GetItemRarity());
	Image_RarityCover->SetBrushFromTexture(RarityTexture, true);
}

void UA1ItemEntryWidget::RefreshItemCount(int32 NewItemCount)
{
	if (NewItemCount < 1)
		return;

	ItemCount = NewItemCount;
	Text_Count->SetText(ItemCount <= 1 ? FText::GetEmpty() : FText::AsNumber(ItemCount));
}
