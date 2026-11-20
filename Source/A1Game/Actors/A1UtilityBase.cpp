#include "A1UtilityBase.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "Character/LyraCharacter.h"
#include "Data/A1ItemData.h"
#include "Item/Fragments/A1ItemFragment_Equipable_Utility.h"
#include "Item/Managers/A1EquipManagerComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Physics/LyraCollisionChannels.h"
#include "System/LyraAssetManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1UtilityBase)

AA1UtilityBase::AA1UtilityBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
    bReplicates = true;
}

void AA1UtilityBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//DOREPLIFETIME(ThisClass, OnOff);
}

void AA1UtilityBase::ChangeState()
{
	OnOff = !OnOff;
	SetActorTickEnabled(OnOff);
	StateIsChanged();
}

//void AA1UtilityBase::OnRep_OnOff()
//{
//	if (bOnlyUseForLocal)
//		return;
//}
