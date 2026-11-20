// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "Actors/A1BedBase.h"

#include "A1GameplayTags.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/A1CharacterAttributeSet.h"
#include "Character/LyraCharacter.h"
#include "Components/ArrowComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Player/LyraPlayerController.h"
#include "Score/A1ScoreManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1BedBase)

AA1BedBase::AA1BedBase(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComponent"));
	SetRootComponent(ArrowComponent);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BedMeshComponent"));
	MeshComponent->SetupAttachment(GetRootComponent());
	MeshComponent->SetCollisionProfileName(TEXT("Interactable"));
	MeshComponent->SetCanEverAffectNavigation(true);

	MaxLyingTimeRate = 300.f; //TODO eric1306 -> Original Value : 300.f(5 minute)
	DecreaseWeightTimeRate = 30.f; //TODO eric1306 -> Original Value : 30.f(30 second)
}

void AA1BedBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AA1BedBase, BedState);
	DOREPLIFETIME(AA1BedBase, StoredPlayerTransform);
	DOREPLIFETIME(AA1BedBase, OccupyingCharacter);
}

void AA1BedBase::BeginPlay()
{
	Super::BeginPlay();

	SetupTags();

	if (HasAuthority())
	{
		TArray<AActor*> FoundSpaceships;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AA1SpaceshipBase::StaticClass(), FoundSpaceships);

		for (AActor* Actor : FoundSpaceships)
		{
			if (AA1SpaceshipBase* Spaceship = Cast<AA1SpaceshipBase>(Actor))
			{
				RegisterWithSpaceship(Spaceship);
				break;
			}
		}
	}
}

FA1InteractionInfo AA1BedBase::GetPreInteractionInfo(const FA1InteractionQuery& InteractionQuery) const
{
	switch (BedState)
	{
	case EBedState::Empty: return EmptyInteractionInfo;
	case EBedState::Occupied: return OccupiedInteractionInfo;
	default: return FA1InteractionInfo();
	}
}

void AA1BedBase::GetMeshComponents(OUT TArray<UMeshComponent*>& OutMeshComponents) const
{

	if (MeshComponent->GetStaticMesh())
	{
		OutMeshComponents.Add(MeshComponent);
	}
}

void AA1BedBase::RegisterWithSpaceship(class AA1SpaceshipBase* Spaceship)
{
	if (Spaceship && HasAuthority())
	{
		Spaceship->RegisterBed(this);
	}
}

void AA1BedBase::SetBedState(EBedState NewBedState)
{
	if (HasAuthority() == false || NewBedState == BedState)
		return;

	//TODO eric1306 - change BedState After 10 minutes

	BedState = NewBedState;
	OnRep_BedState();
}

FTransform AA1BedBase::GetLayDownTransform() const
{
	return GetActorTransform() + LayDownOffset;
}

FRotator AA1BedBase::GetArrowComponentVector() const
{
	return ArrowComponent->GetComponentRotation();
}

void AA1BedBase::StorePlayerReturnTransform(const FTransform& PlayerTransform)
{
	if (HasAuthority())
	{
		StoredPlayerTransform = PlayerTransform;
	}
}

FTransform AA1BedBase::GetPlayerReturnTransform() const
{
	return StoredPlayerTransform;
}

void AA1BedBase::WakeUpOccupyingCharacter()
{
	if (!OccupyingCharacter)
		return;

	//Actor가 LyraCharacter인지 체크
	ALyraCharacter* LyraCharacter = Cast<ALyraCharacter>(OccupyingCharacter);
	if (!LyraCharacter)
		return;

	ALyraPlayerController* LyraPlayerController = LyraCharacter->GetLyraPlayerController();
	if (!LyraPlayerController)
		return;

	// AbilitySystemComponent 가져오기
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(LyraCharacter);
	if (!ASC)
		return;

	FA1InteractionQuery InteractionQuery;
	InteractionQuery.RequestingAvatar = LyraCharacter;
	InteractionQuery.RequestingController = LyraPlayerController;

	if (CanInteraction(InteractionQuery))
	{
		FGameplayEventData Payload;
		Payload.EventTag = A1GameplayTags::Ability_Interact_Active;
		Payload.Instigator = LyraCharacter;
		Payload.Target = this;

		ASC->HandleGameplayEvent(A1GameplayTags::Ability_Interact_Active, &Payload);
	}

	// 캐릭터 참조 제거
	OccupyingCharacter = nullptr;

	//Temp eric1306 (fuck)
	bCanUsed = true;
	bIsUsed = false;
}

void AA1BedBase::SetOccupyingCharacter(AActor* Character)
{
	if (HasAuthority())
	{
		if (Character != nullptr)
		{
			OccupyingCharacter = Character;

			//Health Changed Delegate 등록
			UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OccupyingCharacter);
			if (ASC)
			{
				const UA1CharacterAttributeSet* Attribute = ASC->GetSet<UA1CharacterAttributeSet>();
				if (Attribute)
				{
					Attribute->OnHealthChanged.AddUObject(this, &ThisClass::OnHealthChanged);
				}
			}

			GetWorldTimerManager().SetTimer(MaxLyingTimerHandle, [&]()
				{
					//Multicast_Wakeup();
					WakeUpOccupyingCharacter(); //Temp Code eric1306 -> for standalone
				}, MaxLyingTimeRate, false);

			ActivateDecreaseWeight();
		}
		else
		{
			GetWorldTimerManager().ClearTimer(MaxLyingTimerHandle);

			DeactivateDecreaseWeight();

			//델리게이트 제거
			UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OccupyingCharacter);
			if (ASC)
			{
				const UA1CharacterAttributeSet* Attribute = ASC->GetSet<UA1CharacterAttributeSet>();
				Attribute->OnHealthChanged.RemoveAll(this);
			}

			//후 nullptr로 초기화
			OccupyingCharacter = Character;
		}
	}
}

void AA1BedBase::Multicast_Wakeup_Implementation()
{
	if (HasAuthority())
		return;

	WakeUpOccupyingCharacter();
}

void AA1BedBase::ActivateDecreaseWeight()
{
	// 이미 실행 중인 타이머가 있다면 제거
	if (GetWorldTimerManager().IsTimerActive(DecreaseWeightTimerHandle))
	{
		GetWorldTimerManager().ClearTimer(DecreaseWeightTimerHandle);
	}

	GetWorldTimerManager().SetTimer(DecreaseWeightTimerHandle, [this]()
		{
			if (!OccupyingCharacter || BedState != EBedState::Occupied)
				return;

			ALyraCharacter* LyraCharacter = Cast<ALyraCharacter>(OccupyingCharacter);
			if (!LyraCharacter)
				return;

			UAbilitySystemComponent* ASC = LyraCharacter->GetAbilitySystemComponent();
			if (!ASC)
				return;

			if (DecreaseWeightEffect)
			{
				FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
				EffectContext.AddSourceObject(this);

				FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(DecreaseWeightEffect, 1, EffectContext);
				if (SpecHandle.IsValid())
				{
					ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
				}
			}

		}, DecreaseWeightTimeRate, true);
}

void AA1BedBase::DeactivateDecreaseWeight()
{
	// 타이머 중지
	if (GetWorldTimerManager().IsTimerActive(DecreaseWeightTimerHandle))
	{
		GetWorldTimerManager().ClearTimer(DecreaseWeightTimerHandle);
	}
}

void AA1BedBase::OnRep_BedState()
{
	OnBedStateChanged(BedState);
}

void AA1BedBase::SetupTags()
{
	Tags.AddUnique("SpaceshipComponent");
	Tags.AddUnique("Bed");
}

void AA1BedBase::OnHealthChanged(AActor* InInstigator, float OldValue, float NewValue)
{
	//HP 감소 상관없이 플레이어 깨우기
	WakeUpOccupyingCharacter();
}
