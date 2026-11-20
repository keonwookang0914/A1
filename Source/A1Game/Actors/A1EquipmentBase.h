#pragma once

#include "AbilitySystemInterface.h"
#include "A1Define.h"
#include "AbilitySystem/LyraAbilitySet.h"
#include "Interaction/A1HighlightInterface.h"
#include "A1EquipmentBase.generated.h"

class UAbilitySystemComponent;
class USkeletalMeshComponent;
class UArrowComponent;
class UBoxComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnItemPickupChanged);

UCLASS(BlueprintType, Abstract)
class AA1EquipmentBase : public AActor, public IAbilitySystemInterface, public IA1HighlightInterface
{
	GENERATED_BODY()
	
public:
	AA1EquipmentBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Destroyed() override;

public:
	void Init(int32 InTemplateID, EEquipmentSlotType InEquipmentSlotType, EItemRarity InItemRarity);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void ChangeBlockState(bool bShouldBlock);

protected:
	UFUNCTION()
	void OnRep_EquipmentSlotType();

	UFUNCTION()
	void OnRep_CanBlock();

public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	int32 GetTemplateID() const { return TemplateID; }
	EEquipmentSlotType GetEquipmentSlotType() const { return EquipmentSlotType; }
	EItemRarity GetItemRarity() const { return ItemRarity; }
	bool GetPickup() const { return bPickedup; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	UAnimMontage* GetEquipMontage();
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	UAnimMontage* GetHitMontage(AActor* InstigatorActor, const FVector& HitLocation, bool IsBlocked);

public:
	virtual void Highlight() override;
	virtual void UnHighlight() override;

	void SetPickup(bool InbPickedup) { bPickedup = InbPickedup; };

protected:
	bool bPickedup = false;
	bool bHighlighted = false;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UArrowComponent> ArrowComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UBoxComponent> TraceDebugCollision;

	FOnItemPickupChanged OnItemPickupChanged;
	
protected:
	UPROPERTY(EditAnyWhere, Replicated)
	int32 TemplateID;
	
	UPROPERTY(ReplicatedUsing=OnRep_EquipmentSlotType)
	EEquipmentSlotType EquipmentSlotType = EEquipmentSlotType::Count;

	UPROPERTY(EditAnyWhere)
	EItemRarity ItemRarity = EItemRarity::Count;
	
public:
	UPROPERTY(ReplicatedUsing=OnRep_CanBlock)
	bool bCanBlock = false;

public:
	bool bOnlyUseForLocal = false;
	
protected:
	FLyraAbilitySet_GrantedHandles SkillAbilitySetHandles;
};
