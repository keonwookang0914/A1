#pragma once

#include "A1Define.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "Character/LyraCharacter.h"
#include "A1GameplayAbility_Equipment.generated.h"

class UA1ItemInstance;
class AA1EquipmentBase;

USTRUCT(BlueprintType)
struct FA1EquipmentInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category="A1|Equipment")
	EEquipmentType EquipmentType = EEquipmentType::Count;

public:
	UPROPERTY(EditAnywhere, Category="A1|Equipment", meta=(EditCondition="EquipmentType!=EEquipmentType::Armor", EditConditionHides))
	EEquipmentSlotType ItemSlotType = EEquipmentSlotType::Count;
	
	UPROPERTY(EditAnywhere, Category="A1|Equipment", meta=(EditCondition="EquipmentType==EEquipmentType::Weapon", EditConditionHides))
	EWeaponType RequiredWeaponType = EWeaponType::Count;

public:
	UPROPERTY(EditAnywhere, Category="A1|Equipment", meta=(EditCondition="EquipmentType==EEquipmentType::Utility", EditConditionHides))
	EUtilityType RequiredUtilityType = EUtilityType::Count;

public:
	UPROPERTY()
	TWeakObjectPtr<AA1EquipmentBase> EquipmentActor;
};

UCLASS(Blueprintable)
class UA1GameplayAbility_Equipment : public ULyraGameplayAbility
{
	GENERATED_BODY()
	
public:
	UA1GameplayAbility_Equipment(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;

public:
	AA1EquipmentBase* GetFirstEquipmentActor() const;
	UA1ItemInstance* GetEquipmentItemInstance(const AA1EquipmentBase* InEquipmentActor) const;
	
	int32 GetEquipmentStatValue(FGameplayTag InStatTag, const AA1EquipmentBase* InEquipmentActor) const;
	float GetSnapshottedAttackRate() const { return SnapshottedAttackRate; }
	
protected:
	UPROPERTY(EditDefaultsOnly, Category="A1|Equipment")
	TArray<FA1EquipmentInfo> EquipmentInfos;
	
	UPROPERTY(EditDefaultsOnly, Category="A1|Equipment")
	float DefaultAttackRate = 1.f;

	UPROPERTY(EditDefaultsOnly, Category="A1|Equipment")
	TSubclassOf<ULyraCameraMode> CameraModeClass;

private:
	float SnapshottedAttackRate = 0.f;
};
