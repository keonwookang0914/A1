// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "A1Define.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "A1EquipmentSlotsWidget.generated.h"

class UA1ItemInstance;
class UA1ItemSlotWidget;
class UA1EquipmentSlotWidget;
class UA1EquipManagerComponent;
class UA1EquipmentManagerComponent;

USTRUCT(BlueprintType)
struct FEquipmentInitializeMessage
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UA1EquipmentManagerComponent> EquipmentManager;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UA1EquipManagerComponent> EquipManager;
};

UCLASS()
class A1GAME_API UA1EquipmentSlotsWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UA1EquipmentSlotsWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void ConstructUI(FGameplayTag Channel, const FEquipmentInitializeMessage& Message);
	void DestructUI();

	void OnEquipmentEntryChanged(EEquipmentSlotType EquipmentSlotType, UA1ItemInstance* ItemInstance, int32 ItemCount);

public:
	UPROPERTY(EditAnywhere, meta = (Categories = "Message"))
	FGameplayTag MessageChannelTag;

private:
	UPROPERTY()
	TArray<TObjectPtr<UA1EquipmentSlotWidget>> ItemSlotWidgets;

	UPROPERTY()
	TObjectPtr<UA1EquipmentManagerComponent> EquipmentManager;
				
	UPROPERTY()	
	TObjectPtr<UA1EquipManagerComponent> EquipManager;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UA1EquipmentSlotWidget> Widget_Left_Hand;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UA1EquipmentSlotWidget> Widget_Right_Hand;

private:
	FDelegateHandle EntryChangedDelegateHandle;
	//FDelegateHandle EquipStateChangedDelegateHandle;
	FGameplayMessageListenerHandle MessageListenerHandle;
};
