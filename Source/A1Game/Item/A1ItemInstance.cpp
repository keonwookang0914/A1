#include "A1ItemInstance.h"

#if UE_WITH_IRIS
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#endif // UE_WITH_IRIS

#include "A1ItemTemplate.h"
#include "A1LogChannels.h"
#include "Data/A1ItemData.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1ItemInstance)

UA1ItemInstance::UA1ItemInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
    
}

void UA1ItemInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ItemTemplateID);
	DOREPLIFETIME(ThisClass, ItemRarity);
	DOREPLIFETIME(ThisClass, StatContainer);
}

float UA1ItemInstance::GetDistanceAttenuation(float Distance, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags) const
{
	return 0;
}

float UA1ItemInstance::GetPhysicalMaterialAttenuation(const UPhysicalMaterial* PhysicalMaterial, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags) const
{
	return 0;
}

void UA1ItemInstance::Init(int32 InItemTemplateID, EItemRarity InItemRarity)
{
	if (InItemTemplateID <= INDEX_NONE || InItemRarity == EItemRarity::Count)
		return;

	ItemTemplateID = InItemTemplateID;
	ItemRarity = InItemRarity;

	const UA1ItemTemplate& ItemTemplate = UA1ItemData::Get().FindItemTemplateByID(ItemTemplateID);
	for (const UA1ItemFragment* Fragment : ItemTemplate.Fragments)
	{
		if (Fragment)
		{
			Fragment->OnInstanceCreated(this);
		}
	}
}

#if UE_WITH_IRIS
void UA1ItemInstance::RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags)
{
	using namespace UE::Net;
	FReplicationFragmentUtil::CreateAndRegisterFragmentsForObject(this, Context, RegistrationFlags);
}
#endif // UE_WITH_IRIS

void UA1ItemInstance::AddStatTagStack(FGameplayTag StatTag, int32 StackCount)
{
	StatContainer.AddStack(StatTag, StackCount);
}

void UA1ItemInstance::RemoveStatTagStack(FGameplayTag StatTag)
{
	StatContainer.RemoveStack(StatTag);
}

EItemRarity UA1ItemInstance::DetermineItemRarity(const TArray<FA1ItemRarityProbability>& ItemProbabilities)
{
	float TotalProbability = 0.f;
	for (const FA1ItemRarityProbability& ItemProbability : ItemProbabilities)
	{
		TotalProbability += ItemProbability.Probability;
	}

	if (TotalProbability > 100.f)
		return EItemRarity::Count;
	
	float SumProbability = 0.f;
	float RandomValue = FMath::RandRange(0.f, 100.f);

	for (const FA1ItemRarityProbability& ItemProbability : ItemProbabilities)
	{
		SumProbability += ItemProbability.Probability;
		if (RandomValue < SumProbability)
		{
			return ItemProbability.Rarity;
		}
	}
	
	return EItemRarity::Count;
}

bool UA1ItemInstance::HasStatTag(FGameplayTag StatTag) const
{
	return StatContainer.ContainsTag(StatTag);
}

int32 UA1ItemInstance::GetStackCountByTag(FGameplayTag StatTag) const
{
	return StatContainer.GetStackCount(StatTag);
}

const UA1ItemFragment* UA1ItemInstance::FindFragmentByClass(TSubclassOf<UA1ItemFragment> FragmentClass) const
{
	if (ItemTemplateID > INDEX_NONE && FragmentClass)
	{
		const UA1ItemTemplate& ItemTemplate = UA1ItemData::Get().FindItemTemplateByID(ItemTemplateID);
		return ItemTemplate.FindFragmentByClass(FragmentClass);
	}
	return nullptr;
}
