// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "Actors/A1FlashChargerBase.h"

#include "A1GameplayTags.h"
#include "Character/LyraCharacter.h"
#include "Components/ArrowComponent.h"
#include "Components/SphereComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1FlashChargerBase)

AA1FlashChargerBase::AA1FlashChargerBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComponent"));
	SetRootComponent(ArrowComponent);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ChargerMeshComponent"));
	MeshComponent->SetupAttachment(GetRootComponent());
	MeshComponent->SetCollisionProfileName(TEXT("Interactable"));
	MeshComponent->SetCanEverAffectNavigation(true);

	ChargeZone = CreateDefaultSubobject<USphereComponent>(TEXT("ChargeZone"));
	ChargeZone->SetupAttachment(GetRootComponent());
	ChargeZone->SetSphereRadius(ChargeRadius);
	ChargeZone->SetCollisionProfileName(TEXT("Trigger"));
	ChargeZone->SetGenerateOverlapEvents(true);
}

void AA1FlashChargerBase::BeginPlay()
{
	Super::BeginPlay();

	ChargeZone->SetSphereRadius(ChargeRadius);
	ChargeZone->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnChargeZoneBeginOverlap);
	ChargeZone->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnChargeZoneEndOverlap);
}

FA1InteractionInfo AA1FlashChargerBase::GetPreInteractionInfo(const FA1InteractionQuery& InteractionQuery) const
{
	return ChargerInteractionInfo;
}

void AA1FlashChargerBase::GetMeshComponents(TArray<UMeshComponent*>& OutMeshComponents) const
{
	if (MeshComponent->GetStaticMesh())
	{
		OutMeshComponents.Add(MeshComponent);
	}
}

void AA1FlashChargerBase::OnChargeZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ALyraCharacter* Character = Cast<ALyraCharacter>(OtherActor))
	{
		CharactersInZone.Add(Character);
		AddChargingTag(Character);
	}
}

void AA1FlashChargerBase::OnChargeZoneEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex)
{
	if (ALyraCharacter* Character = Cast<ALyraCharacter>(OtherActor))
	{
		CharactersInZone.Remove(Character);
		RemoveChargingTag(Character);
	}
}

void AA1FlashChargerBase::AddChargingTag(class ALyraCharacter* Character)
{
	if (!IsValid(Character))
		return;

	if (UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent())
	{
		ASC->AddLooseGameplayTag(A1GameplayTags::Status_FlashLight_ChargingZone);
	}
}

void AA1FlashChargerBase::RemoveChargingTag(class ALyraCharacter* Character)
{
	if (!IsValid(Character))
		return;

	if (UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent())
	{
		ASC->RemoveLooseGameplayTag(A1GameplayTags::Status_FlashLight_ChargingZone);
	}
}
