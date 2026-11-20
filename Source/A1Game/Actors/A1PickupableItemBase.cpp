#include "A1PickupableItemBase.h"

#include "Components/BoxComponent.h"
#include "Data/A1ItemData.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Item/A1ItemInstance.h"
#include "Item/Fragments/A1ItemFragment_Equipable.h"
#include "Kismet/KismetSystemLibrary.h"
#include "System/LyraAssetManager.h"

AA1PickupableItemBase::AA1PickupableItemBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	bAlwaysRelevant = true;
	AActor::SetReplicateMovement(true);

	PickupCollision = CreateDefaultSubobject<UBoxComponent>("PickupCollision");
	PickupCollision->SetCollisionProfileName("Pickupable");
	PickupCollision->SetupAttachment(GetRootComponent());

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("MeshComponent");
	MeshComponent->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	MeshComponent->SetCollisionProfileName("NoCollision");
	MeshComponent->SetupAttachment(GetRootComponent());
}

AA1PickupableItemBase::AA1PickupableItemBase(int ItemId)
{
}

void AA1PickupableItemBase::OnRep_PickupInfo()
{
	Super::OnRep_PickupInfo();

	TSoftObjectPtr<UStaticMesh> PickupableMeshPath = nullptr;

	if (const UA1ItemInstance* ItemInstance = PickupInfo.PickupInstance.ItemInstance)
	{
		const UA1ItemTemplate& ItemTemplate = UA1ItemData::Get().FindItemTemplateByID(ItemInstance->GetItemTemplateID());
		PickupableMeshPath = ItemTemplate.PickupableMesh;
	}
	else if (TSubclassOf<UA1ItemTemplate> ItemTemplateClass = PickupInfo.PickupTemplate.ItemTemplateClass)
	{
		const UA1ItemTemplate* ItemTemplate = ItemTemplateClass->GetDefaultObject<UA1ItemTemplate>();
		PickupableMeshPath = ItemTemplate->PickupableMesh;
	}

	if (PickupableMeshPath.IsNull() == false)
	{
		if (UStaticMesh* PickupableMesh = ULyraAssetManager::GetAssetByPath(PickupableMeshPath))
		{
			MeshComponent->SetStaticMesh(PickupableMesh);

			if (bAutoCollisionResize)
			{
				float Radius;
				FVector Origin, BoxExtent;
				UKismetSystemLibrary::GetComponentBounds(MeshComponent, Origin, BoxExtent, Radius);

				FVector PickupCollisionExtent = FVector(FMath::Max(MinPickupCollisionExtent.X, BoxExtent.X), FMath::Max(MinPickupCollisionExtent.Y, BoxExtent.Y), BoxExtent.Z);
				PickupCollision->SetBoxExtent(PickupCollisionExtent);
			}
		}
	}
}

void AA1PickupableItemBase::GetMeshComponents(TArray<UMeshComponent*>& OutMeshComponents) const
{
	OutMeshComponents.Add(MeshComponent);
}
