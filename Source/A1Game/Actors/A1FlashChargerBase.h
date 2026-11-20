// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "Interaction/A1WorldInteractable.h"
#include "A1FlashChargerBase.generated.h"

class USphereComponent;
class UArrowComponent;
/**
 * 
 */
UCLASS()
class AA1FlashChargerBase : public AA1WorldInteractable
{
	GENERATED_BODY()
public:
	AA1FlashChargerBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void BeginPlay() override;

public:
	virtual FA1InteractionInfo GetPreInteractionInfo(const FA1InteractionQuery& InteractionQuery) const override;

	virtual void GetMeshComponents(TArray<UMeshComponent*>& OutMeshComponents) const override;

	UFUNCTION()
	void OnChargeZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnChargeZoneEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex);

	void AddChargingTag(class ALyraCharacter* Character);

	void RemoveChargingTag(class ALyraCharacter* Character);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Info")
	FA1InteractionInfo ChargerInteractionInfo;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UArrowComponent> ArrowComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USphereComponent> ChargeZone;

	UPROPERTY(EditDefaultsOnly, Category = "Charging")
	float ChargeRadius = 200.0f;

private:
	UPROPERTY()
	TSet<TWeakObjectPtr<class ALyraCharacter>> CharactersInZone;
};
