#include "A1GameplayAbility_Weapon_Gun_NormalShoot.h"

#include "A1GameplayTags.h"
#include "A1LogChannels.h"
#include "Actors/A1GunBase.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Character/Raider/A1RaiderBase.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Physics/LyraCollisionChannels.h"
#include "Actors/A1EquipmentBase.h"
#include "Player/LyraPlayerController.h"
#include "System/LyraAssetManager.h"
#include "System/LyraGameData.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1GameplayAbility_Weapon_Gun_NormalShoot)

UA1GameplayAbility_Weapon_Gun_NormalShoot::UA1GameplayAbility_Weapon_Gun_NormalShoot(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
    AbilityTags.AddTag(A1GameplayTags::Ability_Attack_Gun);
	ActivationOwnedTags.AddTag(A1GameplayTags::Status_Attack);
	//ActivationBlockedTags.AddTag(A1GameplayTags::Status_MainHand_Left);
	ActivationRequiredTags.AddTag(A1GameplayTags::Status_ADS_Ready);
}

void UA1GameplayAbility_Weapon_Gun_NormalShoot::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (K2_CheckAbilityCooldown() == false || K2_CheckAbilityCost() == false)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	if (CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo) == false)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	if (HasAuthority(&CurrentActivationInfo))
	{
		if (Shoot())
		{
			if (UAbilityTask_PlayMontageAndWait* ShootMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TEXT("ShootMontage"), ShootMontage, GetSnapshottedAttackRate(), NAME_None, true, 1.f, 0.f, false))
			{
				ShootMontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageFinished);
				ShootMontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageFinished);
				ShootMontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageFinished);
				ShootMontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageFinished);
				ShootMontageTask->ReadyForActivation();
			}
		}
	}

	//FGameplayTagContainer TagContainer;
	//TagContainer.AddTag(A1GameplayTags::Status_ADS_Ready);
	//UAbilitySystemBlueprintLibrary::RemoveLooseGameplayTags(GetAvatarActorFromActorInfo(), TagContainer, true);

	//UAnimMontage* SelectedMontage = K2_CheckAbilityCost() ? ReleaseReloadMontage : ShootMontage;
}

void UA1GameplayAbility_Weapon_Gun_NormalShoot::OnMontageFinished()
{
	//if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	//{
	//	FGameplayEventData Payload;
	//	ASC->HandleGameplayEvent(A1GameplayTags::GameplayEvent_Gun_ADS, &Payload);
	//}
	
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

bool UA1GameplayAbility_Weapon_Gun_NormalShoot::Shoot()
{
	if (HasAuthority(&CurrentActivationInfo) == false)
		return false;

	ALyraCharacter* LyraCharacter = GetLyraCharacterFromActorInfo();
	ALyraPlayerController* LyraPlayerController = GetLyraPlayerControllerFromActorInfo();

	AA1EquipmentBase* EquippedActor = Cast<AA1EquipmentBase>(GetFirstEquipmentActor());
	if (EquippedActor == nullptr)
		return false;

	ULyraAbilitySystemComponent* SourceASC = GetLyraAbilitySystemComponentFromActorInfo();
	if (SourceASC == nullptr)
		return false;

	if (LyraCharacter && LyraPlayerController)
	{
		if (LyraCharacter->bullets <= 0)
		{
			return false;
		}
		LyraCharacter->OnGunEquipped.Broadcast(--LyraCharacter->bullets);

		if(GunSound)
			UGameplayStatics::SpawnSoundAttached(GunSound, EquippedActor->GetRootComponent());

		FTransform SocketTransform = LyraCharacter->GetMesh()->GetSocketTransform(SpawnSocketName, RTS_World);
		FVector SocketLocation = SocketTransform.GetLocation();
		FRotator SocketRotation = SocketTransform.GetRotation().Rotator();

		FVector CameraLocation;
		FRotator CameraRotation;
		LyraPlayerController->GetPlayerViewPoint(CameraLocation, CameraRotation);

		float Distance = (SocketLocation - CameraLocation).Dot(CameraRotation.Vector());
		FVector StartLocation = CameraLocation + CameraRotation.Vector() * (Distance + AimAssistMinDistance);
		FVector EndLocation = StartLocation + (CameraRotation.Vector() * AimAssistMaxDistance);

		FHitResult HitResult;
		TArray<AActor*> ActorsToIgnore = { LyraCharacter, EquippedActor };

		bool bHit = UKismetSystemLibrary::LineTraceSingle(GetWorld(), StartLocation, EndLocation, UEngineTypes::ConvertToTraceType(A1_TraceChannel_AimAssist), false, ActorsToIgnore, EDrawDebugTrace::ForDuration, HitResult, true);
		
		// 가해자 정보
		FGameplayEffectContextHandle EffectContextHandle = SourceASC->MakeEffectContext();
		HitResult.bBlockingHit = true;

		if (EffectContextHandle.IsValid())
		{
			EffectContextHandle.AddHitResult(HitResult);
			EffectContextHandle.AddInstigator(SourceASC->AbilityActorInfo->AvatarActor.Get(), EquippedActor);

			// 맞았으면 데미지 적용
			if (bHit)
			{
				AA1RaiderBase* Target = Cast<AA1RaiderBase>(HitResult.GetActor());
				if (Target)
				{
					float Damage = GetEquipmentStatValue(A1GameplayTags::SetByCaller_BaseDamage, EquippedActor);
					FGameplayAbilityTargetDataHandle TargetDataHandle = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(Target);
					if (TargetDataHandle.IsValid(0))
					{
						const TSubclassOf<UGameplayEffect> DamageGE = ULyraAssetManager::GetSubclassByPath(ULyraGameData::Get().DamageGameplayEffect_SetByCaller);
						FGameplayEffectSpecHandle DamageEffectSpecHandle = MakeOutgoingGameplayEffectSpec(DamageGE);

						if (DamageEffectSpecHandle.IsValid())
						{
							DamageEffectSpecHandle.Data->SetContext(EffectContextHandle);
							// 무기에 희귀도에 따른 대미지 차별화
								DamageEffectSpecHandle.Data->SetSetByCallerMagnitude(A1GameplayTags::SetByCaller_BaseDamage, Damage);
								float DamageSet = DamageEffectSpecHandle.Data->GetSetByCallerMagnitude(A1GameplayTags::SetByCaller_BaseDamage, false);
							UE_LOG(LogA1, Warning, TEXT("Set Damage: %f"), DamageSet);
							ApplyGameplayEffectSpecToTarget(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, DamageEffectSpecHandle, TargetDataHandle);
						}
					}
				}
			}
		}
		
		if (LyraCharacter->IsOutSide())
		{
			// 산소 소모
			float Oxygen = GetEquipmentStatValue(A1GameplayTags::SetByCaller_BaseOxygen, EquippedActor);
			FGameplayAbilityTargetDataHandle TargetDataHandle = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(LyraCharacter);
			if (TargetDataHandle.IsValid(0))
			{
				const TSubclassOf<UGameplayEffect> OxygenGE = ULyraAssetManager::GetSubclassByPath(ULyraGameData::Get().ConsumeOxygenByWeapon_SetByCaller);
				FGameplayEffectSpecHandle OxygenEffectSpecHandle = MakeOutgoingGameplayEffectSpec(OxygenGE);
				if (OxygenEffectSpecHandle.IsValid())
				{
					OxygenEffectSpecHandle.Data->SetContext(EffectContextHandle);
					// 무기에 희귀도에 따른 대미지 차별화
					OxygenEffectSpecHandle.Data->SetSetByCallerMagnitude(A1GameplayTags::SetByCaller_BaseOxygen, Oxygen);
					float DamageSet = OxygenEffectSpecHandle.Data->GetSetByCallerMagnitude(A1GameplayTags::SetByCaller_BaseOxygen, false);
					ApplyGameplayEffectSpecToTarget(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, OxygenEffectSpecHandle, TargetDataHandle);
				}
			}
		}
		
	}

	return true;
}
