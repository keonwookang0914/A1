// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#include "Actors/A1DoorBase.h"
#include "Components/ArrowComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1DoorBase)


AA1DoorBase::AA1DoorBase(const FObjectInitializer& objectInitializer) : Super(objectInitializer)
{
	ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComponent"));
	SetRootComponent(ArrowComponent);

	//Mesh
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMeshComponent"));
	MeshComponent->SetupAttachment(GetRootComponent());
	MeshComponent->SetCollisionProfileName(TEXT("Interactable"));
	MeshComponent->SetCanEverAffectNavigation(true);

}

void AA1DoorBase::BeginPlay()
{
	Super::BeginPlay();

	SetupTags();

	if (HasAuthority())
	{
		OwningSpaceship = FindSpaceshipOwner();

		if (OwningSpaceship.IsValid())
		{
			RegisterWithSpaceship(OwningSpaceship.Get());
		}
	}
}

void AA1DoorBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, DoorState);
}

FA1InteractionInfo AA1DoorBase::GetPreInteractionInfo(const FA1InteractionQuery& InteractionQuery) const
{
	switch (DoorState)
	{
	case EDoorState::Open:
		return OpenedInteractionInfo;
	case EDoorState::Close:
		return ClosedInteractionInfo;
	default:
		return FA1InteractionInfo();
	}
}

void AA1DoorBase::GetMeshComponents(TArray<UMeshComponent*>& OutMeshComponents) const
{
	if (MeshComponent->GetStaticMesh())
	{
		OutMeshComponents.Add(MeshComponent);
	}
}

void AA1DoorBase::RegisterWithSpaceship(AA1SpaceshipBase* Spaceship)
{
	if (Spaceship && HasAuthority())
	{
		OwningSpaceship = Spaceship;
		Spaceship->RegisterDoor(this);
	}
}

void AA1DoorBase::SetDoorState(EDoorState NewDoorState)
{
	if (HasAuthority() == false || NewDoorState == DoorState)
		return;

	DoorState = NewDoorState;
	OnRep_DoorState();
}

void AA1DoorBase::SetupTags()
{
	Tags.AddUnique("SpaceshipComponent");
	Tags.AddUnique("Door");
}

AA1SpaceshipBase* AA1DoorBase::FindSpaceshipOwner() const
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AA1SpaceshipBase::StaticClass(), FoundActors);

	if (FoundActors.Num() > 0)
	{
		return Cast<AA1SpaceshipBase>(FoundActors[0]);
	}

	return nullptr;
}

void AA1DoorBase::OnRep_DoorState()
{
	OnDoorStateChanged(DoorState);
}
