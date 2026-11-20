// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "Actors/A1SignalDetectionBase.h"

#include "Components/ArrowComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1SignalDetectionBase)

AA1SignalDetectionBase::AA1SignalDetectionBase(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	SetRootComponent(ArrowComponent);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MeshComponent->SetupAttachment(GetRootComponent());
	MeshComponent->SetCollisionProfileName(TEXT("Interactable"));
	MeshComponent->SetCanEverAffectNavigation(true);
}

void AA1SignalDetectionBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AA1SignalDetectionBase, SignalDetectionState);
}

void AA1SignalDetectionBase::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		OwningSpaceship = FindSpaceshipOwner();

		if (OwningSpaceship.IsValid())
		{
			RegisterWithSpaceship(OwningSpaceship.Get());
		}
	}
}

FA1InteractionInfo AA1SignalDetectionBase::GetPreInteractionInfo(const FA1InteractionQuery& InteractionQuery) const
{
	switch (SignalDetectionState)
	{
	case ESignalDetectionState::None: return StartDetectionInfo;
	case ESignalDetectionState::Survey: return StopDetectionInfo;
	case ESignalDetectionState::Find: return StopDetectionInfo;
	default: return FA1InteractionInfo();
	}
}

void AA1SignalDetectionBase::GetMeshComponents(TArray<UMeshComponent*>& OutMeshComponents) const
{
	if (MeshComponent->GetStaticMesh())
	{
		OutMeshComponents.Add(MeshComponent);
	}
}

void AA1SignalDetectionBase::SetSignalDetectionState(ESignalDetectionState InSignalDetectionState)
{
	SignalDetectionState = InSignalDetectionState;

	switch (SignalDetectionState)
	{
	case ESignalDetectionState::None:
	case ESignalDetectionState::Survey:
		GetOwningSpaceship()->SetCanUseDockingSignalHandler(false);
		break;
	case ESignalDetectionState::Find:
		GetOwningSpaceship()->SetCanUseDockingSignalHandler(true);
		break;
	}
}

void AA1SignalDetectionBase::RegisterWithSpaceship(class AA1SpaceshipBase* Spaceship)
{
	if (Spaceship && HasAuthority())
	{
		Spaceship->RegisterSignalDetection(this);
	}
}

AA1SpaceshipBase* AA1SignalDetectionBase::FindSpaceshipOwner() const
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AA1SpaceshipBase::StaticClass(), FoundActors);

	if (FoundActors.Num() > 0)
	{
		return Cast<AA1SpaceshipBase>(FoundActors[0]);
	}

	return nullptr;
}
