// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "MainCharacter_AnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_DEEPBROWN_API UMainCharacter_AnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	void SetEvadeValues(float ForwardValue, float RightValue);
protected:
	void UpdateAnimationVariables();
	
#pragma region Variables
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class AMainCharacter* MainPtr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class AWeapon* WeaponPtr;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float MovementSpeed; // [0, 100] 최대 걷기 속도 대비 어느정도 속도인지 백분율로 표기
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float MovementDirection; // [-180, 180]
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float ForwardInputValue;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float RightInputValue;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsLockMode;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsEvading;
#pragma endregion

	// AnimNotify Funcs
	UFUNCTION()
	void AnimNotify_Anticipation();
	UFUNCTION()
	void AnimNotify_Contact();
	UFUNCTION()
	void AnimNotify_Recovery();
	UFUNCTION()
	void AnimNotify_AttackEnd();
	UFUNCTION()
	void AnimNotify_EvadeEnd();
	UFUNCTION()
	void AnimNotify_HitEnd();
	UFUNCTION()
	void AnimNotify_EatEnd();
	UFUNCTION()
	void AnimNotify_DeathEnd();
};
