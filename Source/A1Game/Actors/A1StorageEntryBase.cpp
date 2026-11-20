// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "Actors/A1StorageEntryBase.h"

#include "A1EquipmentBase.h"
#include "A1GameplayTags.h"
#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"
#include "Data/A1ItemData.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Item/A1ItemInstance.h"
#include "Item/A1ItemTemplate.h"
#include "Item/Fragments/A1ItemFragment_Equipable_Attachment.h"
#include "Item/Fragments/A1ItemFragment_Equipable_Utility.h"
#include "Net/UnrealNetwork.h"
#include "Physics/LyraCollisionChannels.h"
#include "Score/A1ScoreBlueprintFunctionLibrary.h"
#include "Score/A1ScoreManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1StorageEntryBase)

AA1StorageEntryBase::AA1StorageEntryBase(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("Root"));
	ArrowComponent->SetHiddenInGame(true);
	SetRootComponent(ArrowComponent);

	ItemTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Trigger"));
	ItemTrigger->SetupAttachment(GetRootComponent());
	ItemTrigger->SetBoxExtent(FVector(15.f, 15.f, 15.f));
	ItemTrigger->SetHiddenInGame(true);
}

void AA1StorageEntryBase::BeginPlay()
{
	Super::BeginPlay();

	ItemState = EItemEntryState::None;
}

void AA1StorageEntryBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AA1StorageEntryBase, CachedItem);
}

FA1InteractionInfo AA1StorageEntryBase::GetPreInteractionInfo(const FA1InteractionQuery& InteractionQuery) const
{
	switch (ItemState)
	{
	case EItemEntryState::None: return ItemInputInfo;
	case EItemEntryState::Exist: return ItemOutputInfo;
	default: return FA1InteractionInfo();
	}
}

void AA1StorageEntryBase::SetItemTransform(int32 ItemTemplateID, EItemRarity ItemRarity, EEquipmentSlotType SlotToCheck)
{
	const UA1ItemTemplate& ItemTemplate = UA1ItemData::Get().FindItemTemplateByID(ItemTemplateID);

	if (const UA1ItemFragment_Equipable_Attachment* AttachmentFragment = ItemTemplate.FindFragmentByClass<UA1ItemFragment_Equipable_Attachment>())
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		// 원하는 위치에 새로운 아이템 스폰
		FVector SpawnLocation = GetActorLocation();

		AA1EquipmentBase* NewCachedItem = GetWorld()->SpawnActor<AA1EquipmentBase>(
			AttachmentFragment->ItemAttachInfo.SpawnItemClass,
			SpawnLocation,
			FRotator::ZeroRotator,
			SpawnParams
		);

		if (NewCachedItem)
		{
			NewCachedItem->Init(ItemTemplateID, SlotToCheck, ItemRarity);
			NewCachedItem->OnItemPickupChanged.AddDynamic(this, &ThisClass::SetItemOutput);
			CachedItem = NewCachedItem;
			SetItemInput();
		}
	}
	
}

void AA1StorageEntryBase::SetItemInput()
{
	UE_LOG(LogTemp, Log, TEXT("[AA1StorageEntryBase] Item Detected!"));
	ItemState = EItemEntryState::Exist;
	UA1ScoreBlueprintFunctionLibrary::SetStorageItems(UA1ScoreBlueprintFunctionLibrary::GetStorageItems() + 1);
	OnItemEntryStateChanged.Broadcast(CachedItem);
}

void AA1StorageEntryBase::SetItemOutput()
{
	UE_LOG(LogTemp, Log, TEXT("[AA1StorageEntryBase] Remove Item"));
	if (UA1ScoreManager::Get()->GetDoTutorial() )
	{
		const UA1ItemTemplate& ItemTemplate = UA1ItemData::Get().FindItemTemplateByID(CachedItem->GetTemplateID());
		const UA1ItemFragment_Equipable_Utility* ItemFragment = Cast<UA1ItemFragment_Equipable_Utility>(ItemTemplate.FindFragmentByClass(UA1ItemFragment_Equipable_Utility::StaticClass()));
		if ( ItemFragment->UtilityType == EUtilityType::FoamGun )
		{
			FGameplayEventData EventData;
			UGameplayMessageSubsystem& MessageSubsystem = UGameplayMessageSubsystem::Get(GetWorld());
			MessageSubsystem.BroadcastMessage(A1GameplayTags::Tutorial_Condition_SprayObtained, EventData);
		}
	}
	CachedItem = nullptr;
	ItemState = EItemEntryState::None;
	UA1ScoreBlueprintFunctionLibrary::SetStorageItems(UA1ScoreBlueprintFunctionLibrary::GetStorageItems() - 1);
}

void AA1StorageEntryBase::SpawnItem(int32 TemplateID)
{
	const UA1ItemTemplate& ItemTemplateClass = UA1ItemData::Get().FindItemTemplateByID(TemplateID);
	UA1ItemInstance* AddedItemInstance = NewObject<UA1ItemInstance>();
	AddedItemInstance->Init(TemplateID, EItemRarity::Poor);
	const UA1ItemFragment_Equipable_Attachment* AttachmentFragment = AddedItemInstance->FindFragmentByClass<UA1ItemFragment_Equipable_Attachment>();

	SetItemTransform(TemplateID, EItemRarity::Poor, AttachmentFragment->ItemHandType);
}
