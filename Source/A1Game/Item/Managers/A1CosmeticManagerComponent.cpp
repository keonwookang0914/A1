#include "A1CosmeticManagerComponent.h"

#include "A1Define.h"
#include "Actors/A1ArmorBase.h"
#include "Character/LyraCharacter.h"
#include "Data/A1CharacterData.h"
//#include "Item/Fragments/A1ItemFragment_Equipable_Armor.h"
#include "Player/LyraPlayerController.h"
#include "System/LyraAssetManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1CosmeticManagerComponent)

UA1CosmeticManagerComponent::UA1CosmeticManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
}

void UA1CosmeticManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	InitializeManager();
}

void UA1CosmeticManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HeadSlot)
	{
		HeadSlot->DestroyComponent();
	}


	if (CosmeticSlots)
	{
		CosmeticSlots->DestroyComponent();
	}
	

	Super::EndPlay(EndPlayReason);
}

//void UA1CosmeticManagerComponent::RefreshArmorMesh(EArmorType ArmorType /*, const UD1ItemFragment_Equipable_Armor* ArmorFragment*/)
//{
//	if (ArmorType == EArmorType::Count)
//		return;
//
//	InitializeManager();
//
//	/*if (ArmorFragment)
//	{
//		if (ArmorFragment == nullptr || ArmorFragment->ArmorType != ArmorType)
//			return;
//
//		SetPrimaryArmorMesh(ArmorType, ArmorFragment->ArmorMesh);
//
//		if (ArmorType == EArmorType::Chest)
//		{
//			if (UChildActorComponent* CosmeticSlot = CosmeticSlots[(int32)EArmorType::Legs])
//			{
//				if (AA1ArmorBase* CosmeticActor = Cast<AA1ArmorBase>(CosmeticSlot->GetChildActor()))
//				{
//					CosmeticActor->ShouldUseSecondaryMesh(ArmorFragment->bIsFullBody);
//				}
//			}
//		}
//		else if (ArmorType == EArmorType::Helmet)
//		{
//			if (HeadSlot)
//			{
//				if (AA1ArmorBase* CosmeticActor = Cast<AA1ArmorBase>(HeadSlot->GetChildActor()))
//				{
//					CosmeticActor->ShouldUseSecondaryMesh(true);
//				}
//			}
//		}
//	}
//	else*/
//	{
//		//if (ArmorType == EArmorType::Chest)
//		//{
//		//	if (UChildActorComponent* CosmeticSlot = CosmeticSlots[(int32)EArmorType::Legs])
//		//	{
//		//		if (AA1ArmorBase* CosmeticActor = Cast<AA1ArmorBase>(CosmeticSlot->GetChildActor()))
//		//		{
//		//			CosmeticActor->ShouldUseSecondaryMesh(false);
//		//		}
//		//	}
//		//}
//		//else if (ArmorType == EArmorType::Helmet)
//		//{
//		//	if (HeadSlot)
//		//	{
//		//		if (AA1ArmorBase* CosmeticActor = Cast<AA1ArmorBase>(HeadSlot->GetChildActor()))
//		//		{
//		//			CosmeticActor->ShouldUseSecondaryMesh(false);
//		//		}
//		//	}
//		//}
//
//		//SetPrimaryArmorMesh(ArmorType, nullptr);
//	}
//}


//void UA1CosmeticManagerComponent::SetPrimaryArmorMesh(EArmorType ArmorType, TSoftObjectPtr<USkeletalMesh> ArmorMeshPtr)
//{
//	if (ArmorType == EArmorType::Count)
//		return;
//
//	InitializeManager();
//
//	if (UChildActorComponent* CosmeticSlot = CosmeticSlots[(int32)ArmorType])
//	{
//		if (AA1ArmorBase* CosmeticActor = Cast<AA1ArmorBase>(CosmeticSlot->GetChildActor()))
//		{
//			if (ArmorMeshPtr.IsNull())
//			{
//				CosmeticActor->SetPrimaryArmorMesh(nullptr);
//			}
//			else
//			{
//				USkeletalMesh* ArmorMesh = ULyraAssetManager::GetAsset<USkeletalMesh>(ArmorMeshPtr);
//				CosmeticActor->SetPrimaryArmorMesh(ArmorMesh);
//			}
//		}
//	}
//}

void UA1CosmeticManagerComponent::GetMeshComponents(TArray<UMeshComponent*>& OutMeshComponents) const
{
	if (HeadSlot)
	{
		if (AA1ArmorBase* CosmeticActor = Cast<AA1ArmorBase>(HeadSlot->GetChildActor()))
		{
			OutMeshComponents.Add(CosmeticActor->GetMeshComponent());
		}
	}

	if (CosmeticSlots)
	{
		if (AA1ArmorBase* CosmeticActor = Cast<AA1ArmorBase>(CosmeticSlots->GetChildActor()))
		{
			OutMeshComponents.Add(CosmeticActor->GetMeshComponent());
		}
	}
	
}

void UA1CosmeticManagerComponent::InitializeManager()
{
	if (bInitialized)
		return;

	bInitialized = true;

	check(CosmeticSlotClass);
	check(CharacterSkinType != ECharacterSkinType::Count);

	if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		if (Character->IsNetMode(NM_DedicatedServer) == false)
		{
			const UA1CharacterData& CharacterData = ULyraAssetManager::Get().GetCharacterData();
			const FA1DefaultArmorMeshSet& DefaultArmorMeshSet = CharacterData.GetDefaultArmorMeshSet(CharacterSkinType);

			//HeadSlot = SpawnCosmeticSlotActor(DefaultArmorMeshSet.DefaultMesh, NAME_None, NULLPTR);

			TArray<FName> SkinMaterialSlotName;
			TArray<TSoftObjectPtr<UMaterialInterface>> SkinMaterial;

			SkinMaterialSlotName.SetNum((int32)EBodyType::Count);
			SkinMaterial.SetNum((int32)EBodyType::Count);

			UEnum* Enum = FindObject<UEnum>(ANY_PACKAGE, TEXT("EBodyType"), true);
			check(Enum);
			
			for (int32 i = 0; i < (int32)EBodyType::Count; i++)
			{
				auto na = (int32)EBodyType::Count;
				SkinMaterialSlotName[i] = FName(*Enum->GetNameStringByIndex(i));
				SkinMaterial[i] = DefaultArmorMeshSet.BodySkinMaterial[i];
			}
			/*if (HasAuthority())
				return;*/

			if (Character->GetLocalRole() == ROLE_AutonomousProxy || Character->GetNetMode() == NM_Standalone)
			{
				CosmeticSlots = SpawnCosmeticSlotActor(DefaultArmorMeshSet.DefaultArmMesh, SkinMaterialSlotName, SkinMaterial);
			}
			else
			{
				CosmeticSlots = SpawnCosmeticSlotActor(DefaultArmorMeshSet.DefaultMesh, SkinMaterialSlotName, SkinMaterial);
			}
			
		}
	}
}

UChildActorComponent* UA1CosmeticManagerComponent::SpawnCosmeticSlotActor(TSoftObjectPtr<USkeletalMesh> InDefaultMesh, TArray<FName> InSkinMaterialSlotName, TArray<TSoftObjectPtr<UMaterialInterface>> InSkinMaterial)
{
	UChildActorComponent* CosmeticComponent = nullptr;

	if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		USceneComponent* ComponentToAttachTo = Character->GetMesh();
		CosmeticComponent = NewObject<UChildActorComponent>(Character);
		CosmeticComponent->SetupAttachment(ComponentToAttachTo);
		CosmeticComponent->SetChildActorClass(CosmeticSlotClass);
		CosmeticComponent->RegisterComponent();

		if (AA1ArmorBase* SpawnedActor = Cast<AA1ArmorBase>(CosmeticComponent->GetChildActor()))
		{
			if (USceneComponent* SpawnedRootComponent = SpawnedActor->GetRootComponent())
			{
				SpawnedRootComponent->AddTickPrerequisiteComponent(ComponentToAttachTo);
			}

			SpawnedActor->InitializeActor(InDefaultMesh, InSkinMaterialSlotName, InSkinMaterial);
		}
	}

	return CosmeticComponent;
}
