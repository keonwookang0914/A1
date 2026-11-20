#pragma once

#include "A1EquipmentBase.h"
#include "A1UtilityBase.generated.h"

UCLASS()
class AA1UtilityBase : public AA1EquipmentBase
{
	GENERATED_BODY()
	
public:
	AA1UtilityBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UFUNCTION()
	void ChangeState();

	UFUNCTION()
	void ChargeGauge() { Gauge = MaxGauge; };

	UFUNCTION(BlueprintImplementableEvent)
	void StateIsChanged();

private:
	/*UFUNCTION()
	void OnRep_OnOff();*/

protected:
	UPROPERTY(BlueprintReadOnly/*, ReplicatedUsing = OnRep_OnOff */ )
	bool OnOff = false;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
	float Gauge;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
	float MaxGauge;
};
