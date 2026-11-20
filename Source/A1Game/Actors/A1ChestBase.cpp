// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "Actors/A1ChestBase.h"

#include "A1EquipmentBase.h"
#include "Components/ArrowComponent.h"
#include "Data/A1ItemData.h"
#include "Item/A1ItemInstance.h"
#include "Item/A1ItemTemplate.h"
#include "Item/Fragments/A1ItemFragment_Equipable_Attachment.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1ChestBase)

AA1ChestBase::AA1ChestBase(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComponent"));
	SetRootComponent(ArrowComponent);

	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BedMeshComponent"));
	MeshComponent->SetupAttachment(GetRootComponent());
	MeshComponent->SetCollisionProfileName(TEXT("Interactable"));
	MeshComponent->SetCanEverAffectNavigation(true);

	ItemLocation = CreateDefaultSubobject<USceneComponent>(TEXT("ItemLocation"));
	ItemLocation->SetupAttachment(GetRootComponent());
}

void AA1ChestBase::BeginPlay()
{
	Super::BeginPlay();

	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, [this]()
		{
			SpawnItems();
		}, 2.f, false);
}

void AA1ChestBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AA1ChestBase, ChestState);
}

FA1InteractionInfo AA1ChestBase::GetPreInteractionInfo(const FA1InteractionQuery& InteractionQuery) const
{
	if (ChestState == EChestState::Close)
		return OpenInteractionInfo;

	return FA1InteractionInfo();
}

void AA1ChestBase::GetMeshComponents(TArray<UMeshComponent*>& OutMeshComponents) const
{
	Super::GetMeshComponents(OutMeshComponents);

	if (MeshComponent->GetSkeletalMeshAsset())
	{
		OutMeshComponents.Add(MeshComponent);
	}
}

void AA1ChestBase::OnRep_ChestState()
{
	OnChestStateChanged(ChestState);
}

void AA1ChestBase::SpawnItems()
{
	TArray<USceneComponent*> FoundComponents;
	ItemLocation->GetChildrenComponents(false, FoundComponents);
	ItemLocations.Append(FoundComponents);

	int32 ItemAmount = FMath::RandRange(1, 3);

	UA1ItemData::Get().GetAllItemTemplateClasses(OUT CachedItemTemplates);

	for (int i = 0; i < ItemAmount; i++)
	{
		FTimerHandle ItemSpawnHandle;
		FTimerDelegate ItemDelegate;
		ItemDelegate.BindUFunction(this, FName("SpawnItem"), i);

		GetWorldTimerManager().SetTimer(ItemSpawnHandle, ItemDelegate, 0.1f, false);
	}
}

void AA1ChestBase::SpawnItem(int32 idx)
{
	int32 ItemtoSpawn = FMath::RandRange(2, CachedItemTemplates.Num() - 6);
	TSubclassOf<UA1ItemTemplate> ItemTemplateClass = CachedItemTemplates[ItemtoSpawn];
	int32 ItemTemplateId = UA1ItemData::Get().FindItemTemplateIDByClass(ItemTemplateClass);
	UA1ItemInstance* AddedItemInstance = NewObject<UA1ItemInstance>();
	AddedItemInstance->Init(ItemTemplateId, EItemRarity::Poor);
	const UA1ItemFragment_Equipable_Attachment* AttachmentFragment = AddedItemInstance->FindFragmentByClass<UA1ItemFragment_Equipable_Attachment>();
	const FA1ItemAttachInfo& AttachInfo = AttachmentFragment->ItemAttachInfo;
	if (AttachInfo.SpawnItemClass)
	{
		AA1EquipmentBase* NewSpawnedItem = GetWorld()->SpawnActorDeferred<AA1EquipmentBase>(AttachInfo.SpawnItemClass, FTransform::Identity, this);
		NewSpawnedItem->Init(AddedItemInstance->GetItemTemplateID(), EEquipmentSlotType::Count, AddedItemInstance->GetItemRarity());
		NewSpawnedItem->SetActorRelativeTransform(ItemLocations[idx]->GetComponentTransform());
		NewSpawnedItem->SetActorScale3D(FVector(1.f, 1.f, 1.f));
		NewSpawnedItem->SetPickup(false);
		NewSpawnedItem->SetActorHiddenInGame(false);
		NewSpawnedItem->FinishSpawning(FTransform::Identity, true);
	}
}