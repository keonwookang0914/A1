// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.


#include "AbilitySystem/Abilities/Utility/A1GameplayAbility_Utility_RemoveFoam.h"

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

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1GameplayAbility_Utility_RemoveFoam)

UA1GameplayAbility_Utility_RemoveFoam::UA1GameplayAbility_Utility_RemoveFoam(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bServerRespectsRemoteAbilityCancellation = false;
	NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ServerOnlyTermination;
	ActivationOwnedTags.AddTag(A1GameplayTags::Status_ActiveUtility);
	ActivationRequiredTags.AddTag(A1GameplayTags::Status_MainHand_Left);
}

void UA1GameplayAbility_Utility_RemoveFoam::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (UAbilityTask_WaitInputRelease* InputReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this, true))
	{
		InputReleaseTask->OnRelease.AddDynamic(this, &UA1GameplayAbility_Utility_RemoveFoam::OnInputReleased);
		InputReleaseTask->ReadyForActivation();
	}

	AA1FoamGunBase* EquippedFoamGun = Cast<AA1FoamGunBase>(GetFirstEquipmentActor());
	if (EquippedFoamGun)
	{
		EquippedFoamGun->ShowNiagara(true);
		EquippedFoamGun->ChangeMode(false);
	}

	if (RemoveSound)
	{
		LoopingAudioComponent = UGameplayStatics::SpawnSoundAttached(
			RemoveSound,
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
		GetWorld()->GetTimerManager().SetTimer(LoopHandle, this, &UA1GameplayAbility_Utility_RemoveFoam::TryRemoveFoam, 0.1f, true);
	}
}

void UA1GameplayAbility_Utility_RemoveFoam::OnInputReleased(float TimeHeld)
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

void UA1GameplayAbility_Utility_RemoveFoam::TryRemoveFoam()
{
	ALyraPlayerController* Controller = GetLyraPlayerControllerFromActorInfo();
	ALyraCharacter* Character = GetLyraCharacterFromActorInfo();
	if (Character == nullptr || Controller == nullptr)
		return;

	const float MaxDistance = 1000.f;
	const int32 MaxTryCount = 5.f;
	TArray<AActor*> ActorsToIgnore = { Character };

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;


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
		FVector SpawnLocation = HitResult.Location;
		FRotator SpawnRotation = FRotationMatrix::MakeFromZ(HitResult.Normal).Rotator();

		if (FoamClass == HitResult.GetActor()->GetClass())
		{
			HitResult.GetActor()->Destroy();
		}
	}

}