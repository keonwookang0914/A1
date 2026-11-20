#pragma once

#include "A1ArmorBase.generated.h"

UCLASS(BlueprintType, Blueprintable)
class AA1ArmorBase : public AActor
{
	GENERATED_BODY()

public:
	AA1ArmorBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	void InitializeActor(TSoftObjectPtr<USkeletalMesh> InDefaultArmorMesh, TArray<FName> InSkinMaterialSlotName, TArray<TSoftObjectPtr<UMaterialInterface>> InSkinMaterial);

private:
	void SetArmorMesh(TSoftObjectPtr<USkeletalMesh> InArmorMesh);

public:
	USkeletalMeshComponent* GetMeshComponent() const { return ArmorMeshComponent; }

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USkeletalMeshComponent> ArmorMeshComponent;

protected:
	UPROPERTY()
	TSoftObjectPtr<USkeletalMesh> DefaultArmorMesh;

protected:
	UPROPERTY()
	TArray<FName> SkinMaterialSlotName;

	UPROPERTY()
	TArray<TSoftObjectPtr<UMaterialInterface>> SkinMaterial;
};