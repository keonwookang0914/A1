#include "A1GunBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1GunBase)

AA1GunBase::AA1GunBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bullet_count = 0;
	EquipmentSlotType = EEquipmentSlotType::TwoHand;
}

void AA1GunBase::BeginPlay()
{
	Super::BeginPlay();

	if (bOnlyUseForLocal)
		return;
	
}