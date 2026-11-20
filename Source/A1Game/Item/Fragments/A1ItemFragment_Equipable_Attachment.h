#pragma once

#include "A1ItemFragment_Equipable.h"
#include "A1ItemFragment_Equipable_Attachment.generated.h"

class AA1EquipmentBase;

USTRUCT(BlueprintType)
struct FA1ItemAttachInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AA1EquipmentBase> SpawnItemClass;

	UPROPERTY(EditDefaultsOnly)
	FName AttachSocket;

	UPROPERTY(EditDefaultsOnly)
	FTransform AttachTransform;
};

UCLASS(Abstract, Const)
class UA1ItemFragment_Equipable_Attachment : public UA1ItemFragment_Equipable
{
	GENERATED_BODY()
	
public:
	UA1ItemFragment_Equipable_Attachment(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	UPROPERTY(EditDefaultsOnly)
	EEquipmentSlotType ItemHandType = EEquipmentSlotType::Count;

	UPROPERTY(EditDefaultsOnly)
	FA1ItemAttachInfo ItemAttachInfo;

	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UAnimMontage> EquipMontage;

	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UAnimMontage> FrontHitMontage;
	
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UAnimMontage> BackHitMontage;

	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UAnimMontage> LeftHitMontage;

	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UAnimMontage> RightHitMontage;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UAnimInstance> AnimInstanceClass;
};
