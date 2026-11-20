#pragma once

#include "A1EquipmentBase.h"
#include "A1GunBase.generated.h"

UCLASS(BlueprintType, Abstract)
class AA1GunBase : public AA1EquipmentBase
{
	GENERATED_BODY()
	
public:
	AA1GunBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());


protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable)
	FORCEINLINE int GetBulletCount() { return bullet_count; }
	FORCEINLINE void SetBulletCount(int count) { bullet_count = count; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int bullet_count;
};
