// Copyright (c) 2025 THIS-ACCENT. All Rights Reserved.

#pragma once

#include "Camera/LyraCameraMode.h"
#include "LyraCameraMode_FirstPerson.generated.h"

/**
 * 
 */
UCLASS(Abstract, Blueprintable)
class ULyraCameraMode_FirstPerson : public ULyraCameraMode
{
	GENERATED_BODY()
public:
	ULyraCameraMode_FirstPerson();

	FORCEINLINE bool GetCanRotate() const { return bCanRotate; }
	FORCEINLINE void SetCanRotate(bool InCanRotate) { bCanRotate = InCanRotate; }
protected:
	virtual void UpdateView(float DeltaTime) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "First Person")
	FVector EyeHeightOffset = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, Category = "First Person")
	bool bApplyHeadBob = false;

	UPROPERTY(EditDefaultsOnly, Category = "First Person", Meta = (EditCondition = "bApplyHeadBob"))
	float HeadBobIntensity = 0.1f;

	UPROPERTY(EditDefaultsOnly, Category = "First Person", Meta = (EditCondition = "bApplyHeadBob"))
	float HeadBobFrequency = 2.0f;

	UPROPERTY(EditDefaultsOnly, Category = "First Person|Sprint")
	float SprintEyeHeightOffsetX = 10.0f;


	float CurrentSprintOffset = 0.0f;

	UPROPERTY(EditDefaultsOnly, Category = "First Person|Sprint")
	float SprintOffsetInterpSpeed = 10.0f;
private:
	bool bCanRotate = true;
	float HeadBobTime = 0.0f;
	FVector CalculateHeadBob(float DeltaTime);
};
