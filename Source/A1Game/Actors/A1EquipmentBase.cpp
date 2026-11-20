#include "A1EquipmentBase.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "Character/LyraCharacter.h"
#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"
#include "Data/A1ItemData.h"
#include "Item/Fragments/A1ItemFragment_Equipable_Weapon.h"
#include "Item/Managers/A1EquipManagerComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Physics/LyraCollisionChannels.h"
#include "System/LyraAssetManager.h"
#include "A1LogChannels.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(A1EquipmentBase)

AA1EquipmentBase::AA1EquipmentBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;
	
    bReplicates = true;

	ArrowComponent = CreateDefaultSubobject<UArrowComponent>("ArrowComponent");
	ArrowComponent->PrimaryComponentTick.bStartWithTickEnabled = false;
	SetRootComponent(ArrowComponent);
	
	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>("ItemMesh");
	MeshComponent->SetCollisionProfileName("Weapon");
	MeshComponent->SetGenerateOverlapEvents(false);
	MeshComponent->SetupAttachment(GetRootComponent());
	MeshComponent->SetRenderCustomDepth(false);
	MeshComponent->PrimaryComponentTick.bStartWithTickEnabled = false;
	MeshComponent->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;

	// TODO 일단 최적화 없이 만들어서 성능 측정 부터
	//// Rendering 최적화
	//MeshComponent->bCastHiddenShadow = false;
	//MeshComponent->SetCullDistance(3000.f);
	//
	//// CullDistanceVolume을 맵에 깔아야 한다고 함 
	//MeshComponent->bAllowCullDistanceVolume = true;
	
	TraceDebugCollision = CreateDefaultSubobject<UBoxComponent>("TraceDebugCollision");
	TraceDebugCollision->SetCollisionProfileName("NoCollision");
	TraceDebugCollision->SetGenerateOverlapEvents(false);
	TraceDebugCollision->SetupAttachment(GetRootComponent());
	TraceDebugCollision->PrimaryComponentTick.bStartWithTickEnabled = false;
}

void AA1EquipmentBase::BeginPlay()
{
	Super::BeginPlay();

	if (bOnlyUseForLocal)
		return;
	
	if (HasAuthority())
	{
		OnRep_EquipmentSlotType();
	}
}

void AA1EquipmentBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	if (bOnlyUseForLocal)
		return;
	
	DOREPLIFETIME(ThisClass, TemplateID);
	DOREPLIFETIME(ThisClass, EquipmentSlotType);
	DOREPLIFETIME(ThisClass, bCanBlock);
}

void AA1EquipmentBase::Destroyed()
{
	if (bOnlyUseForLocal)
		return;
	
	if (ALyraCharacter* Character = Cast<ALyraCharacter>(GetOwner()))
	{
		if (ULyraAbilitySystemComponent* ASC = Cast<ULyraAbilitySystemComponent>(Character->GetAbilitySystemComponent()))
		{
			SkillAbilitySetHandles.TakeFromAbilitySystem(ASC);
		}

		if (UA1EquipManagerComponent* EquipManager = Character->FindComponentByClass<UA1EquipManagerComponent>())
		{
			TArray<FA1EquipEntry>& Entries = EquipManager->GetAllEntries();
			if (Entries[(int32)EquipmentSlotType].GetEquipmentActor() == this)
			{
				Entries[(int32)EquipmentSlotType].SetEquipmentActor(nullptr);
			}
		}
	}
	
	Super::Destroyed();
}

void AA1EquipmentBase::Init(int32 InTemplateID, EEquipmentSlotType InEquipmentSlotType, EItemRarity InItemRarity)
{
	if (bOnlyUseForLocal)
		return;
	
	TemplateID = InTemplateID;
	EquipmentSlotType = InEquipmentSlotType;
	ItemRarity = InItemRarity;
}

void AA1EquipmentBase::ChangeBlockState(bool bShouldBlock)
{
	if (bOnlyUseForLocal)
		return;
	
	if (HasAuthority())
	{
		bCanBlock = bShouldBlock;
		OnRep_CanBlock();
	}
}

void AA1EquipmentBase::OnRep_CanBlock()
{
	if (bOnlyUseForLocal)
		return;
	/* TEMP Jerry
	MeshComponent->SetCollisionResponseToChannel(A1_ObjectChannel_Projectile, bCanBlock ? ECR_Block : ECR_Ignore);
	*/
}

void AA1EquipmentBase::OnRep_EquipmentSlotType()
{
	if (bOnlyUseForLocal)
		return;
	
	if (GetOwner() && GetOwner()->FindComponentByClass<UA1EquipManagerComponent>())
	{
		if (ALyraCharacter* Character = Cast<ALyraCharacter>(GetOwner()))
		{
			if (UA1EquipManagerComponent* EquipManager = Character->FindComponentByClass<UA1EquipManagerComponent>())
			{
				TArray<FA1EquipEntry>& Entries = EquipManager->GetAllEntries();
				Entries[(int32)EquipmentSlotType].SetEquipmentActor(this);
			}
		}
	}
	else
	{
		GetWorldTimerManager().SetTimerForNextTick(this, &ThisClass::OnRep_EquipmentSlotType);
	}
}

UAbilitySystemComponent* AA1EquipmentBase::GetAbilitySystemComponent() const
{
	UAbilitySystemComponent* ASC = nullptr;
	if (ALyraCharacter* LyraCharacter = Cast<ALyraCharacter>(GetOwner()))
	{
		ASC = LyraCharacter->GetAbilitySystemComponent();
	}
	return ASC;
}

UAnimMontage* AA1EquipmentBase::GetEquipMontage()
{
	UAnimMontage* EquipMontage = nullptr;
	
	if (TemplateID > 0)
	{
		const UA1ItemTemplate& ItemTemplate = UA1ItemData::Get().FindItemTemplateByID(TemplateID);
		if (const UA1ItemFragment_Equipable_Attachment* AttachmentFragment = ItemTemplate.FindFragmentByClass<UA1ItemFragment_Equipable_Attachment>())
		{
			EquipMontage = ULyraAssetManager::GetAssetByPath<UAnimMontage>(AttachmentFragment->EquipMontage);
		}
	}
	
	return EquipMontage;
}

UAnimMontage* AA1EquipmentBase::GetHitMontage(AActor* InstigatorActor, const FVector& HitLocation, bool IsBlocked)
{
	UAnimMontage* SelectedMontage = nullptr;

	if (InstigatorActor && TemplateID > 0)
	{
		const UA1ItemTemplate& ItemTemplate = UA1ItemData::Get().FindItemTemplateByID(TemplateID);
		if (const UA1ItemFragment_Equipable_Attachment* AttachmentFragment = ItemTemplate.FindFragmentByClass<UA1ItemFragment_Equipable_Attachment>())
		{
			AActor* CharacterActor = GetOwner();
			const FVector& CharacterLocation = CharacterActor->GetActorLocation();
			const FVector& CharacterDirection = CharacterActor->GetActorForwardVector();

			const FRotator& FacingRotator = UKismetMathLibrary::Conv_VectorToRotator(CharacterDirection);
			const FRotator& CharacterToHitRotator = UKismetMathLibrary::Conv_VectorToRotator((HitLocation - CharacterLocation).GetSafeNormal());

			const FRotator& DeltaRotator = UKismetMathLibrary::NormalizedDeltaRotator(CharacterToHitRotator, FacingRotator);
			float YawAbs = FMath::Abs(DeltaRotator.Yaw);

			if (YawAbs < 60.f)
			{
				SelectedMontage = ULyraAssetManager::GetAssetByPath<UAnimMontage>(AttachmentFragment->FrontHitMontage);
			}
			else if (YawAbs > 120.f)
			{
				SelectedMontage = ULyraAssetManager::GetAssetByPath<UAnimMontage>(AttachmentFragment->BackHitMontage);
			}
			else if (DeltaRotator.Yaw < 0.f)
			{
				SelectedMontage = ULyraAssetManager::GetAssetByPath<UAnimMontage>(AttachmentFragment->LeftHitMontage);
			}
			else
			{
				SelectedMontage = ULyraAssetManager::GetAssetByPath<UAnimMontage>(AttachmentFragment->RightHitMontage);
			}
		}
	}
	return SelectedMontage;
}
	
void AA1EquipmentBase::Highlight()
{
	// 이미 주워진 아이템임
	if (bPickedup)
		return;

	bHighlighted = true;
	if (MeshComponent)
	{
		MeshComponent->SetRenderCustomDepth(true);
		MeshComponent->SetCustomDepthStencilValue(250);
	}
}

void AA1EquipmentBase::UnHighlight()
{
	bHighlighted = false;
	if (MeshComponent)
		MeshComponent->SetRenderCustomDepth(false);
}
