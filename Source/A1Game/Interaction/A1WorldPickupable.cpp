#include "A1WorldPickupable.h"

#include "Data/A1ItemData.h"
#include "Item/A1ItemInstance.h"
#include "Item/A1ItemTemplate.h"
#include "Net/UnrealNetwork.h"


AA1WorldPickupable::AA1WorldPickupable(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bReplicates = true;
}

void AA1WorldPickupable::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, PickupInfo);
}

void AA1WorldPickupable::SetPickupInfo(const FA1PickupInfo& InPickupInfo)
{
	if (HasAuthority() == false)
		return;

	if (InPickupInfo.PickupInstance.ItemInstance || InPickupInfo.PickupTemplate.ItemTemplateClass)
	{
		PickupInfo = InPickupInfo;
		OnRep_PickupInfo();
	}
	else
	{
		Destroy();
	}
}

void AA1WorldPickupable::OnRep_PickupInfo()
{
	if (const UA1ItemInstance* ItemInstance = PickupInfo.PickupInstance.ItemInstance)
	{
		const UA1ItemTemplate& ItemTemplate = UA1ItemData::Get().FindItemTemplateByID(ItemInstance->GetItemTemplateID());
	}
	else if (TSubclassOf<UA1ItemTemplate> ItemTemplateCalss = PickupInfo.PickupTemplate.ItemTemplateClass)
	{
		if (const UA1ItemTemplate* ItemTemplate = ItemTemplateCalss->GetDefaultObject<UA1ItemTemplate>())
		{
			InteractionInfo.Content = ItemTemplate->DisplayName;
		}
	}
}


