#pragma once

#include "A1Interactable.h"
#include "A1WorldInteractable.generated.h"

UCLASS(Abstract, BlueprintType, Blueprintable)
class A1GAME_API AA1WorldInteractable : public AActor, public IA1Interactable
{
	GENERATED_BODY()
	
public:	
	AA1WorldInteractable(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
public:
	virtual bool CanInteraction(const FA1InteractionQuery& InteractionQuery) const override;

public:
	UFUNCTION(BlueprintCallable)
	virtual void OnInteractActiveStarted(AActor* Interactor);

	UFUNCTION(BlueprintImplementableEvent, DisplayName = "OnInteractActiveStarted")
	void K2_OnInteractActiveStarted(AActor* Interactor);

	UFUNCTION(BlueprintCallable)
	virtual void OnInteractActiveEnded(AActor* Interactor);

	UFUNCTION(BlueprintImplementableEvent, DisplayName = "OnInteractActiveEnded")
	void K2_OnInteractActiveEnded(AActor* Interactor);

	UFUNCTION(BlueprintCallable)
	virtual void OnInteractionSuccess(AActor* Interactor);

	UFUNCTION(BlueprintImplementableEvent, DisplayName = "OnInteractionSuccess")
	void K2_OnInteractionSuccess(AActor* Interactor);

protected:
	UPROPERTY(EditDefaultsOnly)
	bool bCanUsed = false;

	UPROPERTY(BlueprintReadWrite, Replicated)
	bool bIsUsed = false;

	UPROPERTY()
	TSet<TWeakObjectPtr<AActor>> CachedInteractors;

};
