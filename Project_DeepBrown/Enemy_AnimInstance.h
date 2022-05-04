// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Enemy_AnimInstance.generated.h"

UENUM()
enum class EEnemyAnimationStatus: uint8
{
	Normal,
	Strafe_Stand,
	Strafe_Forward,
	Strafe_Backward,
	Strafe_Right,
	Strafe_Left
};

UCLASS()
class PROJECT_DEEPBROWN_API UEnemy_AnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	void UpdateAnimationVariables();
	bool CheckObstacleToDirection(FVector StartPos, FVector EndPos) const;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class AEnemy* EnemyPtr;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float MovementSpeed;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float MovementDirection;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	EEnemyAnimationStatus AnimationStatus;

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
	void AnimNotify_HitEnd();
	UFUNCTION()
	void AnimNotify_EvadeEnd();
	UFUNCTION()
	void AnimNotify_DeathEnd();
};
