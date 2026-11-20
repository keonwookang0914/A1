#include "A1EquipManagerComponent.h"

#include "AbilitySystemGlobals.h"
#include "A1CosmeticManagerComponent.h"
#include "A1EquipmentManagerComponent.h"
#include "A1GameplayTags.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/A1CombatSet.h"
#include "Actors/A1EquipmentBase.h"
#include "Character/LyraCharacter.h"
#include "Data/A1CharacterData.h"
#include "Engine/ActorChannel.h"
#include "Item/A1ItemInstance.h"
#include "Item/Fragments/A1ItemFragment_Equipable.h"
#include "Item/Fragments/A1ItemFragment_Equipable_Armor.h"
#include "Item/Fragments/A1ItemFragment_Equipable_Weapon.h"
#include "Net/UnrealNetwork.h"
#include "Player/LyraPlayerController.h"
#include "System/A1GameplayTagStack.h"
#include "System/LyraAssetManager.h"
#include "System/LyraGameData.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1EquipManagerComponent)

void FA1EquipEntry::Init(UA1ItemInstance* InItemInstance)
{
	if (ItemInstance == InItemInstance)
		return;

	ItemInstance = InItemInstance;
	ItemInstance ? Equip() : Unequip();
}

void FA1EquipEntry::Equip()
{
	if (ItemInstance == nullptr)
		return;

	ALyraCharacter* Character = EquipManager->GetCharacter();
	if (Character == nullptr)
		return;

	const UA1ItemFragment_Equipable* EquippableFragment = ItemInstance->FindFragmentByClass<UA1ItemFragment_Equipable>();
	if (EquippableFragment == nullptr)
		return;

	if (EquipManager->GetOwner()->HasAuthority())
	{
		if (ULyraAbilitySystemComponent* ASC = Cast<ULyraAbilitySystemComponent>(EquipManager->GetAbilitySystemComponent()))
		{
			// Remove Previous Ability
			BaseAbilitySetHandles.TakeFromAbilitySystem(ASC);
	
			// Add Current Ability
			if (const ULyraAbilitySet* BaseAbilitySet = EquippableFragment->BaseAbilitySet)
			{
				BaseAbilitySet->GiveToAbilitySystem(ASC, &BaseAbilitySetHandles, ItemInstance);
			}

			// Remove Previous Stat
			ASC->RemoveActiveGameplayEffect(BaseStatHandle);
			BaseStatHandle.Invalidate();
	
			// Add Current Stat
			/* TEMP Jerry
			*  ULyraGameData(GE)
			const TSubclassOf<UGameplayEffect> AttributeModifierGE = ULyraAssetManager::GetSubclassByPath(ULyraGameData::Get().AttributeModifierGameplayEffect);
			check(AttributeModifierGE);
			
			const FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
			const FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(AttributeModifierGE, 1.f, ContextHandle);
			const TSharedPtr<FGameplayEffectSpec>& SpecData = SpecHandle.Data;
			
			for (const FGameplayModifierInfo& ModifierInfo : SpecData->Def->Modifiers)
			{
				SpecData->SetSetByCallerMagnitude(ModifierInfo.ModifierMagnitude.GetSetByCallerFloat().DataTag, 0);
			}
			
			for (const FA1GameplayTagStack& Stack : ItemInstance->GetStatContainer().GetStacks())
			{
				SpecData->SetSetByCallerMagnitude(Stack.GetStackTag(), Stack.GetStackCount());
			}
			
			BaseStatHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);
			*/
		}

		if (EquippableFragment->EquipmentType == EEquipmentType::Weapon || EquippableFragment->EquipmentType == EEquipmentType::Utility)
		{
			// Despawn Previous Real Weapon
			if (IsValid(SpawnedEquipmentActor))
			{
				SpawnedEquipmentActor->Destroy();
			}

			// Spawn Current Real Weapon
			const UA1ItemFragment_Equipable_Attachment* AttachmentFragment = ItemInstance->FindFragmentByClass<UA1ItemFragment_Equipable_Attachment>();
			const FA1ItemAttachInfo& AttachInfo = AttachmentFragment->ItemAttachInfo;
			if (AttachInfo.SpawnItemClass)
			{
				UWorld* World = EquipManager->GetWorld();
				AA1EquipmentBase* NewWeaponActor = World->SpawnActorDeferred<AA1EquipmentBase>(AttachInfo.SpawnItemClass, FTransform::Identity, Character);
				NewWeaponActor->Init(ItemInstance->GetItemTemplateID(), EquipmentSlotType, ItemInstance->GetItemRarity());
				NewWeaponActor->SetActorRelativeTransform(AttachInfo.AttachTransform);
				NewWeaponActor->SetPickup(true);
				NewWeaponActor->AttachToComponent(Character->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, AttachInfo.AttachSocket);
				NewWeaponActor->SetActorHiddenInGame(EquipManager->ShouldHiddenEquipments());
				NewWeaponActor->FinishSpawning(FTransform::Identity, true);
			}
		}
	}
	else
	{
		if (EquippableFragment->EquipmentType == EEquipmentType::Weapon || EquippableFragment->EquipmentType == EEquipmentType::Utility)
		{
			if (Character->IsLocallyControlled())
			{
				// Despawn Previous Pocket Weapon
				if (IsValid(SpawnedPocketWorldActor))
				{
					SpawnedPocketWorldActor->Destroy();
				}

				/* TEMP
				// Spawn Current Pocket Weapon				
				const UA1ItemFragment_Equipable_Attachment* AttachmentFragment = ItemInstance->FindFragmentByClass<UA1ItemFragment_Equipable_Attachment>();
				if (UA1PocketWorldSubsystem* PocketWorldSubsystem = EquipManager->GetWorld()->GetSubsystem<UA1PocketWorldSubsystem>())
				{
					if (APlayerController* PC = Character->GetLyraPlayerController())
					{
						PocketWorldSubsystem->RegisterAndCallForGetPocketStage(PC->GetLocalPlayer(),
							FGetPocketStageDelegate::CreateLambda([this, AttachmentFragment](AA1PocketStage* PocketStage)
							{
								if (IsValid(PocketStage))
								{
									ACharacter* Character = PocketStage->GetCharacter();
									const FA1WeaponAttachInfo& AttachInfo = AttachmentFragment->WeaponAttachInfo;
									
									UWorld* World = EquipManager->GetWorld();
									SpawnedPocketWorldActor = World->SpawnActorDeferred<AA1EquipmentBase>(AttachInfo.SpawnWeaponClass, FTransform::Identity, Character);
									SpawnedPocketWorldActor->SetActorRelativeTransform(AttachInfo.AttachTransform);
									SpawnedPocketWorldActor->AttachToComponent(Character->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, AttachInfo.AttachSocket);
									SpawnedPocketWorldActor->bOnlyUseForLocal = true;
									SpawnedPocketWorldActor->FinishSpawning(FTransform::Identity, true);

									PocketStage->RefreshLightingChannelToActors();
									
									UAnimMontage* PocketWorldIdleMontage = ULyraAssetManager::GetAssetByPath<UAnimMontage>(AttachmentFragment->PocketWorldIdleMontage);
									Character->PlayAnimMontage(PocketWorldIdleMontage);
								}
							})
						);
					}
				}*/
			}
		}
		//else if (EquippableFragment->EquipmentType == EEquipmentType::Armor)
		//{
		//	// Refresh Real Armor Mesh
		//	const UA1ItemFragment_Equipable_Armor* ArmorFragment = ItemInstance->FindFragmentByClass<UA1ItemFragment_Equipable_Armor>();
		//	if (UA1CosmeticManagerComponent* CharacterCosmetics = Character->FindComponentByClass<UA1CosmeticManagerComponent>())
		//	{
		//		CharacterCosmetics->RefreshArmorMesh(ArmorFragment->ArmorType, ArmorFragment);
		//	}
		//
		//	// Refresh Pocket Armor Mesh
		//	if (Character->IsLocallyControlled())
		//	{
		//		/* TEMP				
		//		if (UA1PocketWorldSubsystem* PocketWorldSubsystem = EquipManager->GetWorld()->GetSubsystem<UA1PocketWorldSubsystem>())
		//		{
		//			if (APlayerController* PC = Character->GetLyraPlayerController())
		//			{
		//				PocketWorldSubsystem->RegisterAndCallForGetPocketStage(PC->GetLocalPlayer(),
		//					FGetPocketStageDelegate::CreateLambda([ArmorFragment](AA1PocketStage* PocketStage)
		//					{
		//						if (IsValid(PocketStage))
		//						{
		//							if (UA1CosmeticManagerComponent* CosmeticManager = PocketStage->GetCosmeticManager())
		//							{
		//								CosmeticManager->RefreshArmorMesh(ArmorFragment->ArmorType, ArmorFragment);
		//							}
		//						}
		//					})
		//				);
		//			}
		//		}*/
		//	}
		//}
	}

	if (EquippableFragment->EquipmentType == EEquipmentType::Weapon || EquippableFragment->EquipmentType == EEquipmentType::Utility)
	{
		const UA1ItemFragment_Equipable_Attachment* AttachmentFragment = ItemInstance->FindFragmentByClass<UA1ItemFragment_Equipable_Attachment>();
		if (USkeletalMeshComponent* MeshComponent = Character->GetMesh())
		{
			if (AttachmentFragment->AnimInstanceClass)
			{
				MeshComponent->LinkAnimClassLayers(AttachmentFragment->AnimInstanceClass);
			}

			UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent();
			if (ASC && ASC->HasMatchingGameplayTag(A1GameplayTags::Status_Interact) == false)
			{
				UAnimMontage* EquipMontage = ULyraAssetManager::GetAssetByPath<UAnimMontage>(AttachmentFragment->EquipMontage);
				if (UAnimInstance* AnimInstance = MeshComponent->GetAnimInstance())
				{
					if (AnimInstance->GetCurrentActiveMontage() != EquipMontage)
					{
						Character->PlayAnimMontage(EquipMontage);
					}
				}
			}
		}
	}
}

void FA1EquipEntry::Unequip()
{
	if (EquipmentSlotType == EEquipmentSlotType::TwoHand)
	{
		ALyraCharacter* Character = EquipManager->GetCharacter();
		if (Character == nullptr)
			return;

		if (USkeletalMeshComponent* MeshComponent = Character->GetMesh())
		{
			TSubclassOf<UAnimInstance> BaseAnim = UA1CharacterData::Get().BaseAnimLayers;
			if (BaseAnim != nullptr)
				MeshComponent->LinkAnimClassLayers(BaseAnim);
		}
	}
	if (EquipManager->GetOwner()->HasAuthority())
	{
		if (ULyraAbilitySystemComponent* ASC = Cast<ULyraAbilitySystemComponent>(EquipManager->GetAbilitySystemComponent()))
		{
			// Remove Ability
			BaseAbilitySetHandles.TakeFromAbilitySystem(ASC);

			// Remove Stat
			ASC->RemoveActiveGameplayEffect(BaseStatHandle);
			BaseStatHandle.Invalidate();
		}
		
		if (IsValid(SpawnedEquipmentActor))
		{
			SpawnedEquipmentActor->Destroy();
		}	
	}
	else
	{
		//if (ALyraCharacter* Character = EquipManager->GetCharacter())
		//{
		//	if (UA1EquipmentManagerComponent::IsWeaponSlot(EquipmentSlotType) || UA1EquipmentManagerComponent::IsUtilitySlot(EquipmentSlotType))
		//	{
		//		// Despawn Pocket Weapon
		//		if (Character->IsLocallyControlled())
		//		{
		//			/* TEMP Rookiss
		//			
		//			if (UA1PocketWorldSubsystem* PocketWorldSubsystem = EquipManager->GetWorld()->GetSubsystem<UA1PocketWorldSubsystem>())
		//			{
		//				if (APlayerController* PC = Character->GetLyraPlayerController())
		//				{
		//					PocketWorldSubsystem->RegisterAndCallForGetPocketStage(PC->GetLocalPlayer(),
		//						FGetPocketStageDelegate::CreateLambda([this](AA1PocketStage* PocketStage)
		//						{
		//							if (IsValid(PocketStage))
		//							{
		//								if (IsValid(SpawnedPocketWorldActor))
		//								{
		//									SpawnedPocketWorldActor->Destroy();
		//								}
		//							}
		//						})
		//					);
		//				}
		//			}*/
		//		}
		//	}
		//	else if (UA1EquipmentManagerComponent::IsArmorSlot(EquipmentSlotType))
		//	{
		//		// Refresh Real Armor Mesh
		//		EArmorType ArmorType = EquipManager->ConvertToArmorType(EquipmentSlotType);
		//	
		//		if (UA1CosmeticManagerComponent* CharacterCosmetics = Character->FindComponentByClass<UA1CosmeticManagerComponent>())
		//		{
		//			CharacterCosmetics->RefreshArmorMesh(ArmorType, nullptr);
		//		}
		//		
		//		// Refresh Pocket Armor Mesh
		//		if (Character->IsLocallyControlled())
		//		{
		//			/* TEMP
		//			
		//			if (UA1PocketWorldSubsystem* PocketWorldSubsystem = EquipManager->GetWorld()->GetSubsystem<UA1PocketWorldSubsystem>())
		//			{
		//				if (APlayerController* PC = Character->GetLyraPlayerController())
		//				{
		//					PocketWorldSubsystem->RegisterAndCallForGetPocketStage(PC->GetLocalPlayer(),
		//						FGetPocketStageDelegate::CreateLambda([ArmorType](AA1PocketStage* PocketStage)
		//						{
		//							if (IsValid(PocketStage))
		//							{
		//								if (UA1CosmeticManagerComponent* CosmeticManager = PocketStage->GetCosmeticManager())
		//								{
		//									CosmeticManager->RefreshArmorMesh(ArmorType, nullptr);
		//								}
		//							}
		//						})
		//					);
		//				}
		//			}*/
		//		}
		//	}
		//}
	}
}

bool FA1EquipList::NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
{
	return FFastArraySerializer::FastArrayDeltaSerialize<FA1EquipEntry, FA1EquipList>(Entries, DeltaParams,*this);
}

void FA1EquipList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (int32 AddedIndex : AddedIndices)
	{
		FA1EquipEntry& Entry = Entries[AddedIndex];
		Entry.EquipManager = EquipManager;
		Entry.EquipmentSlotType = (EEquipmentSlotType)AddedIndex;
		
		if (Entry.GetItemInstance())
		{
			Entry.Equip();
		}
	}
}

void FA1EquipList::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	for (int32 ChangedIndex : ChangedIndices)
	{
		FA1EquipEntry& Entry = Entries[ChangedIndex];
		Entry.GetItemInstance() ? Entry.Equip() : Entry.Unequip();
	}
}

void FA1EquipList::Equip(EEquipmentSlotType EquipmentSlotType, UA1ItemInstance* ItemInstance)
{
	FA1EquipEntry& Entry = Entries[(int32)(EquipmentSlotType)];
	Entry.Init(ItemInstance);
	MarkItemDirty(Entry);
}

void FA1EquipList::Unequip(EEquipmentSlotType EquipmentSlotType)
{
	FA1EquipEntry& Entry = Entries[(int32)(EquipmentSlotType)];
	Entry.Init(nullptr);
	MarkItemDirty(Entry);
}

UA1EquipManagerComponent::UA1EquipManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, EquipList(this)
{
    SetIsReplicatedByDefault(true);
	bWantsInitializeComponent = true;
}

void UA1EquipManagerComponent::InitializeComponent()
{
	Super::InitializeComponent();

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		TArray<FA1EquipEntry>& Entries = EquipList.Entries;
		Entries.SetNum((int32)EEquipmentSlotType::Count);

		for (int32 i = 0; i < Entries.Num(); i++)
		{
			FA1EquipEntry& Entry = Entries[i];
			Entry.EquipManager = this;
			Entry.EquipmentSlotType = (EEquipmentSlotType)i;
			EquipList.MarkItemDirty(Entry);
		}
	}
}

void UA1EquipManagerComponent::UninitializeComponent()
{
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		for (int32 i = 0; i < (int32)EEquipmentSlotType::Count; i++)
		{
			Unequip((EEquipmentSlotType)i);
		}
	}
	
	Super::UninitializeComponent();
}

void UA1EquipManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, EquipList);
	DOREPLIFETIME(ThisClass, CurrentEquipState);
	DOREPLIFETIME(ThisClass, CurrentMainHand);
	DOREPLIFETIME(ThisClass, bShouldHiddenEquipments);
}

bool UA1EquipManagerComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bWroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	
	for (FA1EquipEntry& Entry : EquipList.Entries)
	{
		UA1ItemInstance* ItemInstance = Entry.GetItemInstance();
		if (IsValid(ItemInstance))
		{
			bWroteSomething |= Channel->ReplicateSubobject(ItemInstance, *Bunch, *RepFlags);
		}
	}
	
	return bWroteSomething;
}

void UA1EquipManagerComponent::ReadyForReplication()
{
	Super::ReadyForReplication();

	if (IsUsingRegisteredSubObjectList())
	{
		for (const FA1EquipEntry& Entry : EquipList.Entries)
		{
			UA1ItemInstance* ItemInstance = Entry.GetItemInstance();
			if (IsValid(ItemInstance))
			{
				AddReplicatedSubObject(ItemInstance);
			}
		}
	}
}

void UA1EquipManagerComponent::Equip(EEquipmentSlotType EquipmentSlotType, UA1ItemInstance* ItemInstance)
{
	check(GetOwner()->HasAuthority());

	if (EquipmentSlotType == EEquipmentSlotType::Count || ItemInstance == nullptr)
		return;

	EquipList.Equip(EquipmentSlotType, ItemInstance);
	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && ItemInstance)
	{
		AddReplicatedSubObject(ItemInstance);
	}
}

void UA1EquipManagerComponent::Unequip(EEquipmentSlotType EquipmentSlotType)
{
	check(GetOwner()->HasAuthority())

	if (EquipmentSlotType == EEquipmentSlotType::Count)
		return;
	
	TArray<FA1EquipEntry>& Entries = EquipList.Entries;
	FA1EquipEntry& Entry = Entries[(int32)EquipmentSlotType];
	UA1ItemInstance* RemovedItemInstance = Entry.GetItemInstance();
	
	EquipList.Unequip(EquipmentSlotType);
	if (IsUsingRegisteredSubObjectList() && RemovedItemInstance)
	{
		RemoveReplicatedSubObject(RemovedItemInstance);
	}
}

void UA1EquipManagerComponent::ChangeEquipState(EEquipmentSlotType EquipSlotType, bool bWear)
{
	check(GetOwner()->HasAuthority());

	EEquipState NewEquipState = ConvertToEquipState(EquipSlotType);
	if (CanChangeEquipState(NewEquipState, bWear))
	{	
		if (bWear)													// 장비를 착용하려는 경우
		{
			if (CurrentEquipState != EEquipState::Unarmed)			// 한 손인 상태에서 추가 장착 시
				NewEquipState = EEquipState::Both;
		}
		else														// 장비를 해제하려는 경우
		{
			if (CurrentEquipState == EEquipState::Both)				// 두 손이라면	해당 장비의 반대 손으로 상태 변경
				NewEquipState = ConvertToAnotherHand(EquipSlotType);
			else
				NewEquipState = EEquipState::Unarmed;
		}
		CurrentEquipState = NewEquipState;
	}
}

void UA1EquipManagerComponent::ChangeMainHand()
{
	//check(GetOwner()->HasAuthority());

	switch (CurrentMainHand)
	{
	case EMainHandState::Left:
		CurrentMainHand = EMainHandState::Right;
		break;
	case EMainHandState::Right:
		CurrentMainHand = EMainHandState::Left;
		break;
	}

	// TODO Jerry 
	// 서버에서 호출되도록 구조 변경 예정
	BroadcastChangedMessage(CurrentMainHand);
}

void UA1EquipManagerComponent::CanInteract()
{
	FGameplayTag TagToCheck = FGameplayTag::RequestGameplayTag(FName("Status.TryInteract"));
	bool HasTag = GetAbilitySystemComponent()->HasMatchingGameplayTag(TagToCheck);
	bool HasItem = GetEquippedActor(ConvertToEquipmentSlotType(CurrentMainHand)) != nullptr;
	bool HasTwoHandItem = GetEquippedActor(EEquipmentSlotType::TwoHand) != nullptr;

	FGameplayTagContainer TagContainer;
	TagContainer.AddTag(TagToCheck);


	// Tag를 보유했으며 (TwoHand가 찼다면 || MainHand가 비어있지 않다면) 불가능 -> Tag 제거
	if (HasTag && (HasItem || HasTwoHandItem))
		UAbilitySystemBlueprintLibrary::RemoveLooseGameplayTags(GetOwner(), TagContainer, true);
	
	// Tag가 없으며 MainHand & TwoHand 모두 Item이 비었다면 -> Interact 가능 태그 부여
	if(!HasTag && !(HasItem || HasTwoHandItem))
		UAbilitySystemBlueprintLibrary::AddLooseGameplayTags(GetOwner(), TagContainer, true);
}

bool UA1EquipManagerComponent::CanChangeEquipState(EEquipState NewEquipState, bool bWear) const
{
	if (NewEquipState == EEquipState::Count)
		return false;
																// 착용 시도 시
	if (bWear && CurrentEquipState == NewEquipState)			// 이미 착용 중인 상태로 변경 불가
		return false;
									
	if (!bWear && NewEquipState == ConvertToAnotherHand(ConvertToEquipmentSlotType(CurrentEquipState)))
		return false;											// 해제 시도 시 반대 상태를 해체하려 하면 변경 불가

	if (NewEquipState == EEquipState::Both && CurrentEquipState != EEquipState::Unarmed)
		return false;

	return true;
}

AA1EquipmentBase* UA1EquipManagerComponent::GetEquippedActor(EEquipmentSlotType ItemSlotType) const
{
	if (ItemSlotType == EEquipmentSlotType::Count)
		return nullptr;
	
	const TArray<FA1EquipEntry>& Entries = EquipList.Entries;
	const int32 EntryIndex = (int32)ItemSlotType;
	return Entries.IsValidIndex(EntryIndex) ? Entries[EntryIndex].GetEquipmentActor() : nullptr;
}

UA1ItemInstance* UA1EquipManagerComponent::GetEquippedItemInstance(EEquipmentSlotType EquipmentSlotType) const
{
	if (EquipmentSlotType == EEquipmentSlotType::Count)
		return nullptr;

	const TArray<FA1EquipEntry>& Entries = EquipList.Entries;
	const int32 EntryIndex = (int32)EquipmentSlotType;
	return Entries.IsValidIndex(EntryIndex) ? Entries[EntryIndex].GetItemInstance() : nullptr;
}

void UA1EquipManagerComponent::OnRep_CurrentEquipState(EEquipState PrevEquipState)
{
	BroadcastChangedMessage(PrevEquipState, CurrentEquipState);
}

void UA1EquipManagerComponent::OnRep_CurrentMainHand(EMainHandState NewState)
{
	BroadcastChangedMessage(NewState);
}

void UA1EquipManagerComponent::BroadcastChangedMessage(EEquipState PrevEquipState, EEquipState NewEquipState)
{
	if (OnEquipStateChanged.IsBound())
	{
		OnEquipStateChanged.Broadcast(PrevEquipState, NewEquipState);
	}
}

void UA1EquipManagerComponent::BroadcastChangedMessage(EMainHandState NewState)
{
	if (OnMainHandChanged.IsBound())
	{
		OnMainHandChanged.Broadcast(NewState);
	}
}

ALyraCharacter* UA1EquipManagerComponent::GetCharacter() const
{
	return Cast<ALyraCharacter>(GetOwner());
}

ALyraPlayerController* UA1EquipManagerComponent::GetPlayerController() const
{
	if (ALyraCharacter* LyraCharacter = GetCharacter())
	{
		return LyraCharacter->GetLyraPlayerController();
	}
	return nullptr;
}

TArray<FA1EquipEntry>& UA1EquipManagerComponent::GetAllEntries()
{
	return EquipList.GetAllEntries();
}

UAbilitySystemComponent* UA1EquipManagerComponent::GetAbilitySystemComponent() const
{
	return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());
}

UA1EquipmentManagerComponent* UA1EquipManagerComponent::GetEquipmentManager() const
{
	if (ALyraCharacter* LyraCharacter = GetCharacter())
	{
		return LyraCharacter->FindComponentByClass<UA1EquipmentManagerComponent>();
	}
	return nullptr;
}

EEquipmentSlotType UA1EquipManagerComponent::ConvertToEquipmentSlotType(EEquipState EquipState)
{
	EEquipmentSlotType EquipmentSlotType = EEquipmentSlotType::Count;

	switch (EquipState)
	{
	case EEquipState::Left:  EquipmentSlotType = EEquipmentSlotType::LeftHand;  break;
	case EEquipState::Right: EquipmentSlotType = EEquipmentSlotType::RightHand; break;
	case EEquipState::Both:   EquipmentSlotType = EEquipmentSlotType::TwoHand;   break;
	}

	return EquipmentSlotType;
}

EEquipmentSlotType UA1EquipManagerComponent::ConvertToEquipmentSlotType(EMainHandState EquipState)
{
	EEquipmentSlotType EquipmentSlotType = EEquipmentSlotType::Count;

	switch (EquipState)
	{
	case EMainHandState::Left:  EquipmentSlotType = EEquipmentSlotType::LeftHand;  break;
	case EMainHandState::Right: EquipmentSlotType = EEquipmentSlotType::RightHand; break;
	}

	return EquipmentSlotType;
}

EEquipState UA1EquipManagerComponent::ConvertToEquipState(EEquipmentSlotType ItemSlotType)
{
	EEquipState EquipmentSlotType = EEquipState::Count;

	switch (ItemSlotType)
	{
	case EEquipmentSlotType::LeftHand:  EquipmentSlotType = EEquipState::Left;  break;
	case EEquipmentSlotType::RightHand: EquipmentSlotType = EEquipState::Right; break;
	case EEquipmentSlotType::TwoHand:   EquipmentSlotType = EEquipState::Both;   break;
	}

	return EquipmentSlotType;
}

EEquipState UA1EquipManagerComponent::ConvertToAnotherHand(EEquipmentSlotType EquipmentSlotType)
{
	EEquipState EquipState = EEquipState::Count;

	switch (EquipmentSlotType)
	{
	case EEquipmentSlotType::LeftHand:  EquipState = EEquipState::Right;  break;
	case EEquipmentSlotType::RightHand: EquipState = EEquipState::Left; break;
	case EEquipmentSlotType::TwoHand:   EquipState = EEquipState::Unarmed;   break;
	}

	return EquipState;
}

void UA1EquipManagerComponent::ChangeShouldHiddenEquipments(bool bNewShouldHiddenEquipments)
{
	bShouldHiddenEquipments = bNewShouldHiddenEquipments;

	TArray<AA1EquipmentBase*> OutEquippedActors;
	//GetAllEquippedActors(OutEquippedActors);

	const TArray<FA1EquipEntry>& Entries = EquipList.Entries;
	if (CurrentEquipState != EEquipState::Unarmed)
	{
		AA1EquipmentBase* ItemActor = Entries[(int32)ConvertToEquipmentSlotType(CurrentEquipState)].GetEquipmentActor();
		if (IsValid(ItemActor))
		{
			ItemActor->SetActorHiddenInGame(bShouldHiddenEquipments);
		}
	}
}
