// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#include "A1RaiderBase.h"
#include "A1LogChannels.h"
#include "AbilitySystem/Attributes/A1CharacterAttributeSet.h"
#include "Actors/A1EquipmentBase.h"
#include "Actors/A1EquipmentBase.h"
#include "Actors/A1PickupableItemBase.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Raider/A1RaiderBase.h"
#include "Controller/Raider/A1RaiderController.h"
#include "Data/A1RaiderData.h"
#include "GameplayEffect.h"
#include "Kismet/KismetMathLibrary.h"
#include "System/LyraAssetManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1RaiderBase)

// Sets default values
AA1RaiderBase::AA1RaiderBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AIControllerClass = AA1RaiderController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("ASC"));
	HealthSet = CreateDefaultSubobject<UA1CharacterAttributeSet>(TEXT("AttributeSet"));

	// Register to listen for attribute changes.
	HealthSet->OnHealthChanged.AddUObject(this, &AA1RaiderBase::BeAttacked);
	HealthSet->OnOutOfHealth.AddUObject(this, &AA1RaiderBase::HandleOutOfHealth);

	USkeletalMeshComponent* MeshComp = GetMesh();
	check(MeshComp);

	MeshComp->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	MeshComp->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f));
}

// Called when the game starts or when spawned
void AA1RaiderBase::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogA1Raider, Log, TEXT("RaiderBase: Beginplay Call"));

	const UA1RaiderData& RaiderData = ULyraAssetManager::Get().GetRaiderData();
	const FA1RaiderBaseSet& RaiderBase = RaiderData.GetRaiderDataSet(RaiderType);

	if (RaiderBase.DefaultMesh != nullptr)
	{
		USkeletalMeshComponent* MeshComp = GetMesh();
		check(MeshComp);

		MeshComp->SetSkeletalMesh(RaiderBase.DefaultMesh.LoadSynchronous());
	}

	int i = 0;
	for (auto Ability : Abilities)
	{
		FGameplayAbilitySpec StartSpec(Ability);
		StartSpec.InputID = i++;
		ASC->GiveAbility(StartSpec);
	}
}

// Called every frame
void AA1RaiderBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

UAnimMontage* AA1RaiderBase::GetHitMontage(AActor* InstigatorActor, const FVector& HitLocation, bool IsBlocked)
{
	UAnimMontage* SelectedMontage = nullptr;

	if (InstigatorActor)
	{
		const FVector& CharacterLocation = GetActorLocation();
		const FVector& CharacterDirection = GetActorForwardVector();

		const FRotator& FacingRotator = UKismetMathLibrary::Conv_VectorToRotator(CharacterDirection);
		const FRotator& CharacterToHitRotator = UKismetMathLibrary::Conv_VectorToRotator((HitLocation - CharacterLocation).GetSafeNormal());

		const FRotator& DeltaRotator = UKismetMathLibrary::NormalizedDeltaRotator(CharacterToHitRotator, FacingRotator);
		float YawAbs = FMath::Abs(DeltaRotator.Yaw);

		if (YawAbs < 60.f)
		{
			SelectedMontage = ULyraAssetManager::GetAssetByPath<UAnimMontage>(FrontHitMontage);
		}
		else if (YawAbs > 120.f)
		{
			SelectedMontage = ULyraAssetManager::GetAssetByPath<UAnimMontage>(BackHitMontage);
		}
		else if (DeltaRotator.Yaw < 0.f)
		{
			SelectedMontage = ULyraAssetManager::GetAssetByPath<UAnimMontage>(LeftHitMontage);
		}
		else
		{
			SelectedMontage = ULyraAssetManager::GetAssetByPath<UAnimMontage>(RightHitMontage);
		}
	}

	return SelectedMontage;
}

void AA1RaiderBase::AddDropItems(TWeakObjectPtr<AA1EquipmentBase> Item)
{
	if (Item.IsValid())
	{
		TSubclassOf<AA1EquipmentBase> EquipmentClass = TSubclassOf<AA1EquipmentBase>(Item->GetClass());
		dropItems.Add(EquipmentClass);
	}
}

void AA1RaiderBase::BeAttacked(AActor* InInstigator, float OldValue, float NewValue)
{
	if (warlike)
	{
		AAIController* AIController = Cast<AAIController>(GetController());
		if (AIController && AIController->GetBlackboardComponent())
		{
			UBlackboardComponent* BlackBoard = AIController->GetBlackboardComponent();
			if (BlackBoard->GetValueAsBool(AA1RaiderController::CanAttackKey) == false)
			{
				BlackBoard->SetValueAsBool(AA1RaiderController::CanAttackKey, true);
				BlackBoard->SetValueAsObject(AA1RaiderController::AggroTargetKey, InInstigator);

				// 기본 Idle 음성 멈추기
				EnterAttackMode(true);
			}
		}
	}
}

void AA1RaiderBase::HandleOutOfHealth(AActor* InActor, float OldValue, float NewValue)
{
	
	// 애니메이션이 처리될 시간 기다리기
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AA1RaiderBase::SpawnDropItem, 1.5f, false);


	GetController()->UnPossess();  // AI가 더 이상 캐릭터를 제어하지 않도록 함

	// 기본 Idle 음성 멈추기
	EnterAttackMode(false);

	SetDead();
}

void AA1RaiderBase::SpawnDropItem()
{
	// 아이템 스폰
	int ItemNum = dropItems.Num();
	if (ItemNum > 0)
	{
		FVector ItemSpawnLocation = GetActorLocation();
		ItemSpawnLocation.Z = 0;

		//확률에 따라 아이템 설정(일단 무조건 스폰)
		//확률 정해진게 없어 하드코딩
		int32 index = FMath::RandRange(0, 100);
		if (index >= 50)
			GetWorld()->SpawnActor<AA1EquipmentBase>(dropItems[ItemNum - 1], ItemSpawnLocation, GetActorRotation());
		else
			GetWorld()->SpawnActor<AA1EquipmentBase>(dropItems[0], ItemSpawnLocation, GetActorRotation());
	}

	DeatState = EA1DeathState::DeathFinished;
}