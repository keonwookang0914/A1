#include "A1FoamGunBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1FoamGunBase)

AA1FoamGunBase::AA1FoamGunBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	EquipmentSlotType = EEquipmentSlotType::LeftHand;
}

void AA1FoamGunBase::BeginPlay()
{
	Super::BeginPlay();

	if (bOnlyUseForLocal)
		return;
	
}