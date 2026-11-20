// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "Actors/A1CMDBase.h"

#include "Components/ArrowComponent.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1CMDBase)

AA1CMDBase::AA1CMDBase(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComponent"));
	SetRootComponent(ArrowComponent);

	//Mesh
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMeshComponent"));
	MeshComponent->SetupAttachment(GetRootComponent());
	MeshComponent->SetCollisionProfileName(TEXT("Interactable"));
	MeshComponent->SetCanEverAffectNavigation(true);
}

void AA1CMDBase::BeginPlay()
{
	Super::BeginPlay();
}

void AA1CMDBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AA1CMDBase, CMDState);
}

FA1InteractionInfo AA1CMDBase::GetPreInteractionInfo(const FA1InteractionQuery& InteractionQuery) const
{
	switch (CMDState)
	{
	case ECMDState::None: return CmdInteractionInfo;
	default: return FA1InteractionInfo();
	}
}

void AA1CMDBase::GetMeshComponents(TArray<UMeshComponent*>& OutMeshComponents) const
{
	if (MeshComponent->GetStaticMesh())
	{
		OutMeshComponents.Add(MeshComponent);
	}
}
