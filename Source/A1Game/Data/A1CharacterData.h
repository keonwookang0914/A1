#pragma once

#include "A1Define.h"
#include "Engine/DataAsset.h"
#include "A1CharacterData.generated.h"

class UA1ItemTemplate;

USTRUCT(BlueprintType)
struct FA1DefaultItemEntry
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
	EEquipmentSlotType EquipmentSlotType = EEquipmentSlotType::Count;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UA1ItemTemplate> ItemTemplateClass;

	UPROPERTY(EditDefaultsOnly)
	EItemRarity ItemRarity = EItemRarity::Poor;

	UPROPERTY(EditDefaultsOnly)
	int32 ItemCount = 1;
};

USTRUCT(BlueprintType)
struct FA1DefaultArmorMeshSet
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
	TArray<TSoftObjectPtr<UMaterialInterface>> BodySkinMaterial;

public:
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<USkeletalMesh> DefaultMesh;
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<USkeletalMesh> DefaultArmMesh;

};

UCLASS(BlueprintType, Const, meta = (DisplayName = "A1 Character Data"))
class A1GAME_API UA1CharacterData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	static const UA1CharacterData& Get();

public:
#if WITH_EDITOR
	virtual void PreSave(FObjectPreSaveContext ObjectSaveContext) override;
#endif // WITH_EDITOR

public:
	const FA1DefaultArmorMeshSet& GetDefaultArmorMeshSet(ECharacterSkinType CharacterSkinType) const;

private:
	UPROPERTY(EditDefaultsOnly)
	TMap<ECharacterSkinType, FA1DefaultArmorMeshSet> DefaultArmorMeshMap;

	// TODO 제거할 예정
public:
	UPROPERTY(EditDefaultsOnly)
	TArray<FA1DefaultItemEntry> DefaultItemEntries;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAnimInstance> BaseAnimLayers;
};