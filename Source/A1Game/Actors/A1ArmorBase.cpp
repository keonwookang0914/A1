#include "A1ArmorBase.h"
#include "A1Define.h"
#include "System/LyraAssetManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1ArmorBase)

AA1ArmorBase::AA1ArmorBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	ArmorMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>("ArmorMeshComponent");
	ArmorMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetRootComponent(ArmorMeshComponent);
}

void AA1ArmorBase::InitializeActor(TSoftObjectPtr<USkeletalMesh> InDefaultArmorMesh, TArray<FName> InSkinMaterialSlotName, TArray<TSoftObjectPtr<UMaterialInterface>> InSkinMaterial)
{
	DefaultArmorMesh = InDefaultArmorMesh;

	if (InSkinMaterialSlotName.Num() == (int32)EBodyType::Count && InSkinMaterial.Num() == (int32)EBodyType::Count)
	{
		SkinMaterialSlotName = InSkinMaterialSlotName;
		SkinMaterial = InSkinMaterial;
	}

	SetArmorMesh(DefaultArmorMesh);
}


void AA1ArmorBase::SetArmorMesh(TSoftObjectPtr<USkeletalMesh> InArmorMesh)
{
	USkeletalMesh* LoadedArmorMesh = nullptr;
	if (InArmorMesh.IsNull() == false)
	{
		LoadedArmorMesh = ULyraAssetManager::GetAssetByPath<USkeletalMesh>(InArmorMesh);
	}

	ArmorMeshComponent->SetSkeletalMesh(LoadedArmorMesh);
	ArmorMeshComponent->EmptyOverrideMaterials();

	if (SkinMaterialSlotName.Num() == (int32)EBodyType::Count && SkinMaterial.Num() == (int32)EBodyType::Count)
	{
		for (int32 i = 0; i < (int32)EBodyType::Count; i++)
		{
			UMaterialInterface* LoadedMaterial = ULyraAssetManager::GetAssetByPath<UMaterialInterface>(SkinMaterial[i]);
			ArmorMeshComponent->SetMaterialByName(SkinMaterialSlotName[i], LoadedMaterial);
		}
	}
}