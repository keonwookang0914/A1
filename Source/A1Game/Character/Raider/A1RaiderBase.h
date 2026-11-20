// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Character/A1CreatureBase.h"
#include "A1RaiderBase.generated.h"

class UA1CharacterAttributeSet;
class AA1EquipmentBase;

UCLASS()
class A1GAME_API AA1RaiderBase : public AA1CreatureBase
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AA1RaiderBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	UAnimMontage* GetHitMontage(AActor* InstigatorActor, const FVector& HitLocation, bool IsBlocked);

	void AddDropItems(TWeakObjectPtr<AA1EquipmentBase> Item);

	UFUNCTION(BlueprintImplementableEvent)
	void EnterAttackMode(bool OnOff);

protected:
	void BeAttacked(AActor* Instigator, float OldValue, float NewValue);
	void HandleOutOfHealth(AActor* InActor, float OldValue, float NewValue);
	
	UFUNCTION()
	virtual void SpawnDropItem();

public:
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UAnimMontage> FrontHitMontage;

	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UAnimMontage> BackHitMontage;

	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UAnimMontage> LeftHitMontage;

	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UAnimMontage> RightHitMontage;

	UPROPERTY(EditAnywhere)
	ERaiderType RaiderType = ERaiderType::Count;

protected:
	UPROPERTY()
	TObjectPtr<UA1CharacterAttributeSet> HealthSet;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	TArray<TSubclassOf<AA1EquipmentBase>> dropItems;

	bool warlike = true;

private:
	FTimerHandle TimerHandle;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	TArray<TSubclassOf<class UGameplayAbility>> Abilities;
};
