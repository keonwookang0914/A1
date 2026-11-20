#pragma once

#include "A1EquipmentBase.h"
#include "A1FoamGunBase.generated.h"

UCLASS(BlueprintType, Abstract)
class AA1FoamGunBase : public AA1EquipmentBase
{
	GENERATED_BODY()
	
public:
	AA1FoamGunBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintImplementableEvent)
	void ShowNiagara(bool bShow);

	UFUNCTION(BlueprintImplementableEvent)
	void ChangeMode(bool bSprayMode);
};
