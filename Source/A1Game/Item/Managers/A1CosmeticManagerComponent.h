#pragma once

#include "A1Define.h"
#include "Components/PawnComponent.h"
#include "A1CosmeticManagerComponent.generated.h"

class AA1ArmorBase;
//class UA1ItemFragment_Equipable_Armor;

UCLASS(BlueprintType, Blueprintable)
class UA1CosmeticManagerComponent : public UPawnComponent
{
	GENERATED_BODY()

public:
	UA1CosmeticManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	//void RefreshArmorMesh(EArmorType ArmorType /*, const UA1ItemFragment_Equipable_Armor* ArmorFragment*/);
	void GetMeshComponents(TArray<UMeshComponent*>& OutMeshComponents) const;

private:
	void InitializeManager();

	UChildActorComponent* SpawnCosmeticSlotActor(TSoftObjectPtr<USkeletalMesh> InDefaultMesh, TArray<FName> InSkinMaterialSlotName, TArray<TSoftObjectPtr<UMaterialInterface>> InSkinMaterial);
	//void SetPrimaryArmorMesh(EArmorType ArmorType, TSoftObjectPtr<USkeletalMesh> ArmorMeshPtr);

protected:
	UPROPERTY(EditDefaultsOnly)
	ECharacterSkinType CharacterSkinType = ECharacterSkinType::Red;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AA1ArmorBase> CosmeticSlotClass;

private:
	UPROPERTY()
	TObjectPtr<UChildActorComponent> HeadSlot;

	UPROPERTY()
	TObjectPtr<UChildActorComponent> CosmeticSlots;

	bool bInitialized = false;
};