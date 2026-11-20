// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "A1SpaceshipBase.h"
#include "Interaction/A1WorldInteractable.h"
#include "A1SignalDetectionBase.generated.h"

class UWidgetComponent;
class UArrowComponent;

UENUM(BlueprintType)
enum class ESignalDetectionState : uint8
{
	None,
	Survey,
	Find
};
/**
 * 
 */
UCLASS()
class AA1SignalDetectionBase : public AA1WorldInteractable, public IA1SpaceshipComponent
{
	GENERATED_BODY()
public:
	AA1SignalDetectionBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;

public:
	virtual FA1InteractionInfo GetPreInteractionInfo(const FA1InteractionQuery& InteractionQuery) const override;
	virtual void GetMeshComponents(TArray<UMeshComponent*>& OutMeshComponents) const override;

	UFUNCTION(BlueprintCallable)
	ESignalDetectionState GetSignalDetectionState() const { return SignalDetectionState; }

	UFUNCTION(BlueprintCallable)
	void SetSignalDetectionState(ESignalDetectionState InSignalDetectionState);

	//IA1SpaceshipInterface
	virtual void RegisterWithSpaceship(class AA1SpaceshipBase* Spaceship) override;
	virtual ESpaceshipComponentType GetComponentType() const override { return ESpaceshipComponentType::SignalDetection; }

	UFUNCTION(BlueprintImplementableEvent)
	void StartDetectSignal();

	UFUNCTION(BlueprintImplementableEvent)
	void StopDetectSignal();

	UFUNCTION(BlueprintCallable)
	AA1SpaceshipBase* FindSpaceshipOwner() const;

	FORCEINLINE	AA1SpaceshipBase* GetOwningSpaceship() const { return OwningSpaceship.IsValid() ? OwningSpaceship.Get() : nullptr; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SignalDetection")
	TObjectPtr<UArrowComponent> ArrowComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SignalDetection")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SignalDetection")
	TObjectPtr<UWidgetComponent> WidgetComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SignalDetection")
	TObjectPtr<UUserWidget> SignalWidget;

	UPROPERTY(EditDefaultsOnly, Category = "SignalDetection|Info")
	FA1InteractionInfo StartDetectionInfo;

	UPROPERTY(EditDefaultsOnly, Category = "SignalDetection|Info")
	FA1InteractionInfo StopDetectionInfo;

	UPROPERTY(VisibleAnywhere, Category = "SignalDetection", Replicated)
	ESignalDetectionState SignalDetectionState = ESignalDetectionState::None;

	UPROPERTY(VisibleAnywhere, Transient)
	TWeakObjectPtr<AA1SpaceshipBase> OwningSpaceship;

public:
	UPROPERTY(BlueprintReadWrite)
	float DelayTime = 0.0f;
};
