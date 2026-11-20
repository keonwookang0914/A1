// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "Actors/A1StorageBase.h"

#include "A1EquipmentBase.h"
#include "A1StorageEntryBase.h"
#include "Character/LyraCharacter.h"
#include "Character/Raider/A1RaiderBase.h"
#include "Components/ArrowComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/OverlapResult.h"
#include "DrawDebugHelpers.h"
#include "Data/A1ItemData.h"
#include "Net/UnrealNetwork.h"
#include "Physics/LyraCollisionChannels.h"
#include "Score/A1ScoreManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1StorageBase)

AA1StorageBase::AA1StorageBase(const FObjectInitializer& ObjectInitializer)
    :Super(ObjectInitializer)
{
    ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComponent"));
    SetRootComponent(ArrowComponent);

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BedMeshComponent"));
    MeshComponent->SetupAttachment(GetRootComponent());
    MeshComponent->SetCollisionProfileName(TEXT("Interactable"));
    MeshComponent->SetCanEverAffectNavigation(true);
    MeshComponent->SetWorldScale3D(FVector(1.5f, 1.5f, 1.5f));

    // 4*4 = 16 Entries
    StorageWidthNum = 4;
    StorageHeightNum = 4;
}

void AA1StorageBase::BeginPlay()
{
    Super::BeginPlay();

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    SpawnStartLocation = GetActorLocation() + FVector(0.f, 0.f, 0.f);
    for (int32 row = 0; row < StorageHeightNum ; row++) for (int32 col=0; col < StorageWidthNum ; col++)
    {
        FVector FinalSpawnLocation = SpawnStartLocation + FVector(-60.f + 50.f * col, 0.f, 50.f + 60.f * row);
        FRotator SpawnRotation = GetActorRotation();
        AA1StorageEntryBase* Entry = GetWorld()->SpawnActor<AA1StorageEntryBase>(EntryClass, FinalSpawnLocation, SpawnRotation, Params);
        StorageEntries.Add(Entry);
    }

	if(UA1ScoreManager::Get()->GetDoTutorial())
	{
		SpawnDefaultItems();
	}
}

void AA1StorageBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AA1StorageBase, StorageEntries);
}

void AA1StorageBase::RegisterWithSpaceship(AA1SpaceshipBase* Spaceship)
{
    if (Spaceship && HasAuthority())
    {
        OwningSpaceship = Spaceship;
        Spaceship->RegisterStorage(this);
    }
}

bool AA1StorageBase::RemoveItem(AA1RaiderBase* InstigatorActor)
{
    if (EntryIsEmpty())
        return true;
    else
    {
        int num = StorageWidthNum * StorageHeightNum;
        for (int i = 0; i < num; i++)
        {
            AA1StorageEntryBase* ItemEntry = StorageEntries[i];
            if (ItemEntry->GetItemEntryState() == EItemEntryState::None)
            {
                continue;
            }
            else
            {
                TWeakObjectPtr<AA1EquipmentBase> Item = ItemEntry->GetItem();
                ItemEntry->SetItemOutput();
                InstigatorActor->AddDropItems(Item);

                if (Item.IsValid())
                {
                    Item->Destroy();
                }
                break;
            }
        }

        if (EntryIsEmpty())
            return true;
    }

    return false;
}

bool AA1StorageBase::EntryIsEmpty()
{
    for (AA1StorageEntryBase* Item : StorageEntries)
    {
        if (Item->GetItemEntryState() == EItemEntryState::Exist)
        {
            return false;
        }
    }
    return true;
}

void AA1StorageBase::SpawnDefaultItems()
{
    // TODO eric1306: Fix HardCoding
    StorageEntries[14].Get()->SpawnItem(2004);
    //StorageEntries[15].Get()->SpawnItem(1000);
}

void AA1StorageBase::SetupTags()
{
    Tags.AddUnique("SpaceshipComponent");
    Tags.AddUnique("Storage");
}

AA1SpaceshipBase* AA1StorageBase::FindSpaceshipOwner() const
{
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AA1SpaceshipBase::StaticClass(), FoundActors);

    if (FoundActors.Num() > 0)
    {
        return Cast<AA1SpaceshipBase>(FoundActors[0]);
    }

    return nullptr;
}
