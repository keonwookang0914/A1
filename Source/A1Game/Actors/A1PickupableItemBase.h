#pragma once

#include "Interaction/A1WorldPickupable.h"
#include "A1PickupableItemBase.generated.h"

class UBoxComponent;
class UProjectileMovementComponent;

UCLASS()
class AA1PickupableItemBase : public AA1WorldPickupable
{
	GENERATED_BODY()

public:
	AA1PickupableItemBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	AA1PickupableItemBase(int ItemId);

protected:
	virtual void OnRep_PickupInfo() override;
	virtual void GetMeshComponents(TArray<UMeshComponent*>& OutMeshComponents) const override;

protected:
	UPROPERTY(EditDefaultsOnly)
	bool bAutoCollisionResize = false;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UBoxComponent> PickupCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> MeshComponent;

protected:
	UPROPERTY(EditDefaultsOnly)
	FVector2D MaxMovementCollisionExtent = FVector2D(16.f, 16.f);

	UPROPERTY(EditDefaultsOnly)
	FVector2D MinPickupCollisionExtent = FVector2D(32.f, 32.f);

};
