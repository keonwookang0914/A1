// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "A1Define.h"
#include "GameplayTagContainer.h"
#include "A1UIData.generated.h"

class UImage;
class UA1ItemDragWidget;
class UA1ItemHoverWidget;
class UA1EquipmentEntryWidget;
class UA1InventorySlotWidget;
class UA1InventoryEntryWidget;
class UA1InventoryValidWidget;

USTRUCT(BlueprintType)
struct FA1ItemRarityInfoEntry
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, meta = (HideAlphaChannel))
	FColor Color = FColor::White;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UTexture2D> EntryTexture;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UTexture2D> HoverTexture;
};

USTRUCT(BlueprintType)
struct FA1UIInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditDefaultsOnly)
	FText Title;

	UPROPERTY(EditDefaultsOnly)
	FText Content;
};

UCLASS(BlueprintType, Const, meta=(DisplayName="A1 UI Data"))
class A1GAME_API UA1UIData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	static const UA1UIData& Get();

public: 
	UTexture2D* GetEntryRarityTexture(EItemRarity ItemRarity) const;
	UTexture2D* GetHoverRarityTexture(EItemRarity ItemRarity) const;
	FColor GetRarityColor(EItemRarity ItemRarity) const;

	const FA1UIInfo& GetTagUIInfo(FGameplayTag Tag) const;

public:
	UPROPERTY(EditDefaultsOnly)
	FIntPoint UnitInventorySlotSize = FIntPoint::ZeroValue;

public:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UA1ItemDragWidget> DragWidgetClass;

	//UPROPERTY(EditDefaultsOnly)
	//TSubclassOf<UA1ItemHoverWidget> ItemHoverWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UA1EquipmentEntryWidget> EquipmentEntryWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UA1InventorySlotWidget> InventorySlotWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UA1InventoryEntryWidget> InventoryEntryWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UA1InventoryValidWidget> InventoryValidWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> EndingCutSceneClass;

private:
	UPROPERTY(EditDefaultsOnly, meta = (ArraySizeEnum = "EItemRarity"))
	FA1ItemRarityInfoEntry RarityInfoEntries[(int32)EItemRarity::Count];

	UPROPERTY(EditDefaultsOnly, meta = (DisplayName = "Tag UI Infos"))
	TMap<FGameplayTag, FA1UIInfo> TagUIInfos;
};
