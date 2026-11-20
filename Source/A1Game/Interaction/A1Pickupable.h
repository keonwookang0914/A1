#pragma once

#include "A1Define.h"
#include "UObject/Interface.h"
#include "A1Pickupable.generated.h"

class UA1ItemTemplate;
class UA1ItemInstance;

USTRUCT(BlueprintType)
struct FA1PickupTemplate
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UA1ItemTemplate> ItemTemplateClass;

	UPROPERTY(EditAnywhere)
	EItemRarity ItemRarity = EItemRarity::Poor;

	UPROPERTY(EditAnywhere)
	int32 ItemCount = 1;
};

USTRUCT(BlueprintType)
struct FA1PickupInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UA1ItemInstance> ItemInstance;

	UPROPERTY(EditAnywhere)
	int32 ItemCount = 1;
};

USTRUCT(BlueprintType)
struct FA1PickupInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FA1PickupTemplate PickupTemplate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FA1PickupInstance PickupInstance;
};

UINTERFACE(MinimalAPI, BlueprintType, meta = (CannotImplementInterfaceInBlueprint))
class UA1Pickupable : public UInterface
{
	GENERATED_BODY()
};

class IA1Pickupable
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	virtual FA1PickupInfo GetPickupInfo() const = 0;
};
