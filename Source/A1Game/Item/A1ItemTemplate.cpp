#include "A1ItemTemplate.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif // WITH_EDITOR

#include "Fragments/A1ItemFragment_Equipable.h"
#include "Fragments/A1ItemFragment_Equipable_Armor.h"
#include "Fragments/A1ItemFragment_Equipable_Utility.h"
#include "Fragments/A1ItemFragment_Equipable_Weapon.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1ItemTemplate)

UA1ItemTemplate::UA1ItemTemplate(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
    
}

#if WITH_EDITOR
EDataValidationResult UA1ItemTemplate::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = UObject::IsDataValid(Context);

	if (SlotCount.X < 1 || SlotCount.Y < 1)
	{
		Context.AddError(FText::FromString(FString::Printf(TEXT("SlotCount is less than 1"))));
		Result = EDataValidationResult::Invalid;
	}
	
	if (MaxStackCount < 1)
	{
		Context.AddError(FText::FromString(FString::Printf(TEXT("MaxStackCount is less than 1"))));
		Result = EDataValidationResult::Invalid;
	}
	
	const UA1ItemFragment_Equipable* FoundEquipable = nullptr;
	for (UA1ItemFragment* Fragment : Fragments)
	{
		if (UA1ItemFragment_Equipable* CurrentEquippable = Cast<UA1ItemFragment_Equipable>(Fragment))
		{
			if (FoundEquipable)
			{
				Context.AddError(FText::FromString(FString::Printf(TEXT("Duplicated Equippable Fragment"))));
				return EDataValidationResult::Invalid;
			}

			FoundEquipable = CurrentEquippable;
		}
	}
	
	if (FoundEquipable)
	{
		if (FoundEquipable->EquipmentType == EEquipmentType::Count)
		{
			Context.AddError(FText::FromString(FString::Printf(TEXT("Equipment Type is Invalid : [EEquipmentType::Count]"))));
			return EDataValidationResult::Invalid;
		}
		
		if (FoundEquipable->EquipmentType == EEquipmentType::Weapon)
		{
			const UA1ItemFragment_Equipable_Weapon* WeaponFragment = Cast<UA1ItemFragment_Equipable_Weapon>(FoundEquipable);
			
			if (WeaponFragment->WeaponType == EWeaponType::Count)
			{
				Context.AddError(FText::FromString(FString::Printf(TEXT("Item Type is Invalid : [EItemType::Count]"))));
				Result = EDataValidationResult::Invalid;
			}
				
			if (WeaponFragment->ItemHandType == EEquipmentSlotType::Count)
			{
				Context.AddError(FText::FromString(FString::Printf(TEXT("Item Hand Type is Invalid : [EItemHandType::Count]"))));
				Result = EDataValidationResult::Invalid;
			}
		}
		else if (FoundEquipable->EquipmentType == EEquipmentType::Utility)
		{
			const UA1ItemFragment_Equipable_Utility* UtilityFragment = Cast<UA1ItemFragment_Equipable_Utility>(FoundEquipable);

			if (UtilityFragment->UtilityType == EUtilityType::Count)
			{
				Context.AddError(FText::FromString(FString::Printf(TEXT("Utility Type is Invalid : [EUtilityType::Count]"))));
				Result = EDataValidationResult::Invalid;
			}
		}

		if (FoundEquipable->EquipmentType == EEquipmentType::Armor || FoundEquipable->EquipmentType == EEquipmentType::Weapon)
		{
			if (MaxStackCount != 1)
			{
				Context.AddError(FText::FromString(FString::Printf(TEXT("Armor or Weapon Type must have MaxStackCount of 1: [MaxStackCount != 1]"))));
				Result = EDataValidationResult::Invalid;
			}
		}
	}
	
	return Result;
}
#endif // WITH_EDITOR

const UA1ItemFragment* UA1ItemTemplate::FindFragmentByClass(TSubclassOf<UA1ItemFragment> FragmentClass) const
{
	if (FragmentClass)
	{
		for (UA1ItemFragment* Fragment : Fragments)
		{
			if (Fragment && Fragment->IsA(FragmentClass))
			{
				return Fragment;
			}
		}
	}
	return nullptr;
}
