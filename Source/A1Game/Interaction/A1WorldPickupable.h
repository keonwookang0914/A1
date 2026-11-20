#pragma once

#include "A1Interactable.h"
#include "A1Pickupable.h"
#include "A1WorldPickupable.generated.h"

UCLASS(Abstract, Blueprintable)
class A1GAME_API AA1WorldPickupable : public AActor, public IA1Interactable, public IA1Pickupable
{
	GENERATED_BODY()
	
public:	
	AA1WorldPickupable(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

public:
	virtual FA1InteractionInfo GetPreInteractionInfo(const FA1InteractionQuery& InteractionQuery) const override { return InteractionInfo; }
	virtual void SetPickupInfo(const FA1PickupInfo& InPickupInfo);
	virtual FA1PickupInfo GetPickupInfo() const override { return PickupInfo; }

protected:
	UFUNCTION()
	virtual void OnRep_PickupInfo();

protected:
	UPROPERTY(EditAnywhere, Category = "Info")
	FA1InteractionInfo InteractionInfo;

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_PickupInfo, Category = "Info")
	FA1PickupInfo PickupInfo;
};
