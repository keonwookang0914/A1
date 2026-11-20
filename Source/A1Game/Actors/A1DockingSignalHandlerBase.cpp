// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "Actors/A1DockingSignalHandlerBase.h"

#include "Components/ArrowComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Maps/A1RandomMapGenerator.h"
#include "Math/TransformCalculus3D.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(A1DockingSignalHandlerBase)

AA1DockingSignalHandlerBase::AA1DockingSignalHandlerBase(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;


	ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComponent"));
	SetRootComponent(ArrowComponent);

	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(GetRootComponent());
	MeshComponent->SetCollisionProfileName(TEXT("Interactable"));
	MeshComponent->SetCanEverAffectNavigation(true);
}

void AA1DockingSignalHandlerBase::BeginPlay()
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

void AA1DockingSignalHandlerBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, SignalState);
}

FA1InteractionInfo AA1DockingSignalHandlerBase::GetPreInteractionInfo(const FA1InteractionQuery& InteractionQuery) const
{
	switch (SignalState)
	{
	case ESignalState::Pressed:		return PressedInteractionInfo;
	case ESignalState::Released:	return ReleasedInteractionInfo;
	default:						return FA1InteractionInfo();
	}
}

void AA1DockingSignalHandlerBase::GetMeshComponents(TArray<UMeshComponent*>& OutMeshComponents) const
{
	OutMeshComponents.Add(MeshComponent);
}

void AA1DockingSignalHandlerBase::RegisterWithSpaceship(class AA1SpaceshipBase* Spaceship)
{
	if (Spaceship && HasAuthority())
	{
		OwningSpaceship = Spaceship;
		Spaceship->RegisterDockingSignalHandler(this);
	}
}

void AA1DockingSignalHandlerBase::SetSignalState(ESignalState NewSignalState)
{
	if (HasAuthority() == false || NewSignalState == SignalState)
		return;

	SignalState = NewSignalState;
	OnRep_SignalState();
}

void AA1DockingSignalHandlerBase::SetupTags()
{
	Tags.AddUnique("SpaceshipComponent");
	Tags.AddUnique("DockingSignalHandler");
}

AA1SpaceshipBase* AA1DockingSignalHandlerBase::FindSpaceshipOwner() const
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AA1SpaceshipBase::StaticClass(), FoundActors);

	if (FoundActors.Num() > 0)
	{
		return Cast<AA1SpaceshipBase>(FoundActors[0]);
	}

	return nullptr;
}

void AA1DockingSignalHandlerBase::OnRep_SignalState()
{
	OnSignalStateChanged(SignalState);
}
