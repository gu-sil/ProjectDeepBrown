// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy_AnimInstance.h"

#include "Enemy.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"

void UEnemy_AnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();
	RootMotionMode = ERootMotionMode::RootMotionFromEverything;
	EnemyPtr = Cast<AEnemy>(TryGetPawnOwner());
	AnimationStatus = EEnemyAnimationStatus::Normal;
}

void UEnemy_AnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	UpdateAnimationVariables();
}

void UEnemy_AnimInstance::UpdateAnimationVariables()
{
	

	if (EnemyPtr && AnimationStatus == EEnemyAnimationStatus::Normal)
	{
		MovementSpeed = EnemyPtr->GetVelocity().Size() / EnemyPtr->RunSpeed * 100;
		MovementDirection = this->CalculateDirection(EnemyPtr->GetVelocity(), EnemyPtr->GetActorRotation());
	}
	else if (EnemyPtr && AnimationStatus == EEnemyAnimationStatus::Strafe_Left)
	{
		MovementSpeed = EnemyPtr->StrafeSpeed;
		MovementDirection = 90.f;
		if (CheckObstacleToDirection(TryGetPawnOwner()->GetActorLocation(), TryGetPawnOwner()->GetActorLocation() + TryGetPawnOwner()->GetActorRightVector() * 200))
			AnimationStatus = EEnemyAnimationStatus::Strafe_Stand;
	}
	else if (EnemyPtr && AnimationStatus == EEnemyAnimationStatus::Strafe_Right)
	{
		MovementSpeed = EnemyPtr->StrafeSpeed;
		MovementDirection = -90.f;
		if (CheckObstacleToDirection(TryGetPawnOwner()->GetActorLocation(), TryGetPawnOwner()->GetActorLocation() + TryGetPawnOwner()->GetActorRightVector() * -200))
			AnimationStatus = EEnemyAnimationStatus::Strafe_Stand;
	}
	else if (EnemyPtr && AnimationStatus == EEnemyAnimationStatus::Strafe_Stand)
	{
		MovementSpeed = 0.f;
		MovementDirection = 0.f;
	}
	else if (EnemyPtr && AnimationStatus == EEnemyAnimationStatus::Strafe_Forward)
	{
		MovementSpeed = EnemyPtr->StrafeSpeed;
		MovementDirection = 0.f;
		if (CheckObstacleToDirection(TryGetPawnOwner()->GetActorLocation(), TryGetPawnOwner()->GetActorLocation() + TryGetPawnOwner()->GetActorForwardVector() * 200))
			AnimationStatus = EEnemyAnimationStatus::Strafe_Stand;
	}
	else if (EnemyPtr && AnimationStatus == EEnemyAnimationStatus::Strafe_Backward)
	{
		MovementSpeed = EnemyPtr->StrafeSpeed;
		MovementDirection = 180.f;
		if (CheckObstacleToDirection(TryGetPawnOwner()->GetActorLocation(), TryGetPawnOwner()->GetActorLocation() + TryGetPawnOwner()->GetActorForwardVector() * -200))
			AnimationStatus = EEnemyAnimationStatus::Strafe_Stand;
	}
}

bool UEnemy_AnimInstance::CheckObstacleToDirection(FVector StartPos, FVector EndPos) const
{
	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(TryGetPawnOwner());
	bool HasHitResult = GetWorld()->LineTraceSingleByChannel(HitResult, StartPos, EndPos, ECollisionChannel::ECC_GameTraceChannel4, Params);
	DrawDebugLine(GetWorld(), StartPos, EndPos, FColor::Red);
	return HasHitResult;
}

void UEnemy_AnimInstance::AnimNotify_Anticipation()
{
	if (EnemyPtr && EnemyPtr->WeaponPtr)
	{
		EnemyPtr->SetCombatStatus(ECombatStatus::Anticipation);
	}
}

void UEnemy_AnimInstance::AnimNotify_Contact()
{
	if (EnemyPtr && EnemyPtr->WeaponPtr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Enemy_Anim: Contact!"))
		EnemyPtr->SetCombatStatus(ECombatStatus::Contact);
		EnemyPtr->WeaponPtr->ActivateCollision();
	}
}

void UEnemy_AnimInstance::AnimNotify_Recovery()
{
	if (EnemyPtr && EnemyPtr->WeaponPtr)
	{
		EnemyPtr->SetCombatStatus(ECombatStatus::Recovery);
		EnemyPtr->WeaponPtr->DeactivateCollision();
	}
}

void UEnemy_AnimInstance::AnimNotify_AttackEnd()
{
	if (EnemyPtr)
	{
		EnemyPtr->SetCombatStatus(ECombatStatus::CombatIdle);
		EnemyPtr->AttackEnd();
		EnemyPtr->WeaponPtr->DeactivateCollision();
	}
}

void UEnemy_AnimInstance::AnimNotify_HitEnd()
{
	if (EnemyPtr)
	{
		EnemyPtr->HitAnimationEnd();
	}
}

void UEnemy_AnimInstance::AnimNotify_EvadeEnd()
{
	if (EnemyPtr)
	{
		EnemyPtr->EvadeEnd();
	}
}

void UEnemy_AnimInstance::AnimNotify_DeathEnd()
{
	if (EnemyPtr)
	{
		EnemyPtr->DeathEnd();
	}
}
