// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "AbilitySystem/Abilities/Utility/A1GameplayAbility_Utility_SprayFoam.h"

#include "A1GameplayTags.h"
#include "A1LogChannels.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Actors/A1FoamGunBase.h"
#include "Character/LyraCharacter.h"
#include "Player/LyraPlayerController.h"
#include "Physics/LyraCollisionChannels.h"
#include "System/LyraAssetManager.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1GameplayAbility_Utility_SprayFoam)

UA1GameplayAbility_Utility_SprayFoam::UA1GameplayAbility_Utility_SprayFoam(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bServerRespectsRemoteAbilityCancellation = false;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ServerOnlyTermination;
	ActivationOwnedTags.AddTag(A1GameplayTags::Status_ActiveUtility);
	ActivationRequiredTags.AddTag(A1GameplayTags::Status_MainHand_Left);
}

void UA1GameplayAbility_Utility_SprayFoam::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (UAbilityTask_WaitInputRelease* InputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this, true))
	{
		InputReleaseTask->OnRelease.AddDynamic(this, &UA1GameplayAbility_Utility_SprayFoam::OnInputReleased);
		InputReleaseTask->ReadyForActivation();
	}

	AA1FoamGunBase* EquippedFoamGun = Cast<AA1FoamGunBase>(GetFirstEquipmentActor());
	if (EquippedFoamGun)
	{
		EquippedFoamGun->ShowNiagara(true);
		EquippedFoamGun->ChangeMode(true);
	}

	if (SpraySound)
	{
		LoopingAudioComponent = UGameplayStatics::SpawnSoundAttached(
			SpraySound,
			GetAvatarActorFromActorInfo()->GetRootComponent(),
			NAME_None,
			FVector::ZeroVector,
			EAttachLocation::KeepRelativeOffset,
			true  // Looping
		);

		if (LoopingAudioComponent)
		{
			LoopingAudioComponent->bIsUISound = false;
		}
	}

	if (HasAuthority(&CurrentActivationInfo))
	{
		GetWorld()->GetTimerManager().SetTimer(LoopHandle, this, &UA1GameplayAbility_Utility_SprayFoam::TrySprayFoam, 0.1f, true);
	}
}

void UA1GameplayAbility_Utility_SprayFoam::OnInputReleased(float TimeHeld)
{
	AA1FoamGunBase* EquippedFoamGun = Cast<AA1FoamGunBase>(GetFirstEquipmentActor());
	if (EquippedFoamGun)
	{
		EquippedFoamGun->ShowNiagara(false);
	}

	GetWorld()->GetTimerManager().ClearTimer(LoopHandle);
	if (LoopingAudioComponent)
	{
		LoopingAudioComponent->Stop();
		LoopingAudioComponent = nullptr;
	}
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UA1GameplayAbility_Utility_SprayFoam::TrySprayFoam()
{
	ALyraPlayerController* Controller = GetLyraPlayerControllerFromActorInfo();
	ALyraCharacter* Character = GetLyraCharacterFromActorInfo();
	if (Character == nullptr || Controller == nullptr)
		return;

	const float MaxDistance = 1000.f;
	const int32 MaxTryCount = 5.f;
	TArray<AActor*> ActorsToIgnore = { Character };

	FHitResult HitResult;
	FVector2D RandPoint = FMath::RandPointInCircle(MaxDistance);

	FVector CameraLocation;
	FRotator CameraRotation;
	Controller->GetPlayerViewPoint(CameraLocation, CameraRotation);

	FVector StartLocation = CameraLocation;
	FVector EndLocation = StartLocation + (CameraRotation.Vector() * MaxDistance);


	bool bHit = UKismetSystemLibrary::LineTraceSingle(GetWorld(), StartLocation, EndLocation, UEngineTypes::ConvertToTraceType(A1_TraceChannel_AimAssist), false, ActorsToIgnore, EDrawDebugTrace::ForDuration, HitResult, true);

	TSubclassOf<AActor> FoamClass = ULyraAssetManager::Get().GetSubclassByName<AActor>("FoamBase");;


	if(bHit)
	{
		float rate = HitResult.GetActor()->GetActorScale3D().X;
		// 맞은 대상이 폼 액터이고 특정 크기 이하인 경우
		if ( FoamClass == HitResult.GetActor()->GetClass() && rate <= 0.5)
		{
			HitResult.GetActor()->SetActorScale3D(HitResult.GetActor()->GetActorScale3D() * 1.2f);	
		}
		else
		{
			// 액터 스폰
			FVector SpawnLocation = HitResult.Location + HitResult.Normal * 3.f;
			FRotator SpawnRotation = FRotationMatrix::MakeFromZ(HitResult.Normal).Rotator();

			FActorSpawnParameters SpawnParameters;
			SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		
			AActor* SpawnedFoam = GetWorld()->SpawnActor<AActor>(FoamClass, SpawnLocation, SpawnRotation, SpawnParameters);


			// skeletal mesh 대상인 경우
			USkeletalMeshComponent* MeshComp = HitResult.GetActor()->FindComponentByClass<USkeletalMeshComponent>();
			if (MeshComp)
			{
				// 1. 본 중에서 HitResult.Location에 가장 가까운 본 찾기
				FName ClosestBoneName = MeshComp->FindClosestBone(HitResult.Location);
				if (ClosestBoneName == NAME_None)
					return;

				// 2. 상대 위치 계산
				FVector BoneWorldLocation = MeshComp->GetBoneLocation(ClosestBoneName);
				FVector RelativeOffset = HitResult.Location - BoneWorldLocation;

				// 3. 붙이기: 상대 위치를 유지하며 해당 본에 붙이기
				if (SpawnedFoam)
				{
					SpawnedFoam->AttachToComponent(MeshComp, FAttachmentTransformRules::KeepRelativeTransform, ClosestBoneName);
					SpawnedFoam->SetActorRelativeLocation(RelativeOffset); // 본 기준 상대 위치로 맞춰줌
				}
			}

			// Static Mesh인 경우
			else               
			{
				if (HitResult.GetComponent() != nullptr)
				{
					if(SpawnedFoam)
					{
						SpawnedFoam->AttachToComponent(HitResult.GetComponent(), FAttachmentTransformRules::KeepWorldTransform);
					}
				}
			}
		}
	}
}