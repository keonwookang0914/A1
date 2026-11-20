// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "Interaction/A1WorldInteractable.h"
#include "A1CMDBase.generated.h"

UENUM()
enum class ECMDState : uint8
{
	None,
	Use
};
class UArrowComponent;
/**
 * 
 */
UCLASS()
class AA1CMDBase : public AA1WorldInteractable
{
	GENERATED_BODY()
public:
	AA1CMDBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

public:
	virtual FA1InteractionInfo GetPreInteractionInfo(const FA1InteractionQuery& InteractionQuery) const override;
	virtual void GetMeshComponents(TArray<UMeshComponent*>& OutMeshComponents) const override;


protected:

	UPROPERTY(BlueprintReadOnly, Replicated)
	ECMDState CMDState = ECMDState::None;

	UPROPERTY(EditDefaultsOnly, Category = "Info")
	FA1InteractionInfo CmdInteractionInfo;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UArrowComponent> ArrowComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> MeshComponent;

};
