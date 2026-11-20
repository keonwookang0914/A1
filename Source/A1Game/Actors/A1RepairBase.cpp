// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "Actors/A1RepairBase.h"

#include "A1EquipmentBase.h"
#include "A1SpaceshipBase.h"
#include "Character/LyraCharacter.h"
#include "Components/ArrowComponent.h"
#include "Item/Managers/A1EquipManagerComponent.h"
#include "Kismet/GameplayStatics.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1RepairBase)

AA1RepairBase::AA1RepairBase(const FObjectInitializer& objectInitializer) : Super(objectInitializer)
{
	ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComponent"));
	SetRootComponent(ArrowComponent);

	//Mesh
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMeshComponent"));
	MeshComponent->SetupAttachment(GetRootComponent());
	MeshComponent->SetCollisionProfileName(TEXT("Interactable"));
	MeshComponent->SetCanEverAffectNavigation(true);
}

void AA1RepairBase::BeginPlay()
{
	Super::BeginPlay();

	FindOwningSpaceship();
}

void AA1RepairBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

FA1InteractionInfo AA1RepairBase::GetPreInteractionInfo(const FA1InteractionQuery& InteractionQuery) const
{
	if (CurrentState == RepairState::NotBroken || CurrentState == RepairState::Complete)
	{
		return AlreadyRepairedInteractionInfo; //return null interaction info
	}

	if (CurrentState == RepairState::Break)
	{
		if (ALyraCharacter* Character = Cast<ALyraCharacter>(InteractionQuery.RequestingAvatar.Get()))
		{
			if (IsHoldingRepairKit(Character))
			{
				return InteractionInfo;
			}
			else
			{
				return NoRepairKitInteractionInfo; //return null interaction info
			}
		}
	}

	return FA1InteractionInfo();
}

void AA1RepairBase::GetMeshComponents(TArray<UMeshComponent*>& OutMeshComponents) const
{
	if (MeshComponent->GetStaticMesh())
	{
		OutMeshComponents.Add(MeshComponent);
	}
}

void AA1RepairBase::SetCurrentState(RepairState InState)
{
	CurrentState = InState;

	if (CurrentState == RepairState::Break)
	{
		SetSpriteBreak();
		bIsFoamed = false;
	}
	else if (CurrentState == RepairState::NotBroken)
	{
		SetSpriteNotBroken();
	}
	else if (CurrentState == RepairState::Foamed)
	{
		SetSpriteFoamed();
		if (!bIsFoamed)
		{
			bIsFoamed = true;
			CachedSpaceship->SetFuelConsumeAmount(CachedSpaceship->GetFuelConsumeAmount() - 1);
		}
	}
	else //RepairState::Complete
	{
		SetSpriteComplete();
		bIsFoamed = false;
	}
}

void AA1RepairBase::OnRepairChanged()
{
	//TODO eric1306
	/*
	 * Tutorial -> Complete 여야함 -> 이후 InnerMap 에서 해당 Foam 객체들을 RepairBase -> Foam으로 변경
	 * not Tutorial -> Foamed 여야함 -> 이후 
	 */
	if (CachedSpaceship)
	{
		if (CachedSpaceship->GetbTutorial())
		{
			SetCurrentState(RepairState::Complete);
		}
		else
		{
			SetCurrentState(RepairState::Foamed);
		}

		OnRepairStateChanged.Broadcast();
	}
}

bool AA1RepairBase::IsHoldingRepairKit(ALyraCharacter* Character) const
{
	if (!IsValid(Character))
	{
		return false;
	}

	UA1EquipManagerComponent* EquipManager = Character->FindComponentByClass<UA1EquipManagerComponent>();
	if (!EquipManager)
	{
		return false;
	}

	EMainHandState CurrentMainHand = EquipManager->GetCurrentMainHand();
	EEquipmentSlotType MainHandSlot = EquipManager->ConvertToEquipmentSlotType(CurrentMainHand);

	AA1EquipmentBase* EquippedItem = EquipManager->GetEquippedActor(EEquipmentSlotType::TwoHand);
	if (!EquippedItem)
	{
		EquippedItem = EquipManager->GetEquippedActor(MainHandSlot);
	}

	if (!IsValid(EquippedItem))
	{
		return false;
	}

	FString ClassName = EquippedItem->GetClass()->GetName();

	if (ClassName.Contains(TEXT("B_Utility_RepairKit")) ||
		ClassName.Contains(TEXT("RepairKit")))
	{
		return true;
	}

	return false;
}

void AA1RepairBase::FindOwningSpaceship()
{
	AActor* Actor = UGameplayStatics::GetActorOfClass(GetWorld(), AA1SpaceshipBase::StaticClass());
	if (Actor)
	{
		if (AA1SpaceshipBase* Spaceship = Cast<AA1SpaceshipBase>(Actor))
		{
			CachedSpaceship = Spaceship;
		}
	}
}
