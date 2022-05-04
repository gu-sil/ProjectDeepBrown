// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharacter_AnimInstance.h"

#include "MainCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UMainCharacter_AnimInstance::NativeBeginPlay()
{
	RootMotionMode = ERootMotionMode::RootMotionFromEverything;
	MainPtr = Cast<AMainCharacter>(TryGetPawnOwner());
	WeaponPtr = MainPtr->GetWeapon();
}

void UMainCharacter_AnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	UpdateAnimationVariables();
}

void UMainCharacter_AnimInstance::UpdateAnimationVariables()
{
	if (MainPtr) 
	{
		MovementSpeed = MainPtr->GetVelocity().Size() / MainPtr->GetCharacterMovement()->MaxWalkSpeed * 100;
		MovementDirection = this->CalculateDirection(MainPtr->GetVelocity(), MainPtr->GetActorRotation());
		bIsLockMode = MainPtr->GetHasLockTarget();
		bIsEvading = MainPtr->GetMovementStatus() == EMovementStatus::Evading;
	}
}

void UMainCharacter_AnimInstance::SetEvadeValues(float ForwardValue, float RightValue)
{
	ForwardInputValue = ForwardValue;
	RightInputValue = RightValue;
}


void UMainCharacter_AnimInstance::AnimNotify_Anticipation()
{
	WeaponPtr = MainPtr->GetWeapon();
	if(MainPtr && WeaponPtr)
	{
		MainPtr->SetCombatStatus(ECombatStatus::Anticipation);
	}
}

void UMainCharacter_AnimInstance::AnimNotify_Contact()
{
	WeaponPtr = MainPtr->GetWeapon();
	if(MainPtr && WeaponPtr)
	{
		MainPtr->SetCombatStatus(ECombatStatus::Contact);
		WeaponPtr->ActivateCollision();
	}
}

void UMainCharacter_AnimInstance::AnimNotify_Recovery()
{
	if (MainPtr && WeaponPtr)
	{
		WeaponPtr = MainPtr->GetWeapon();
		MainPtr->SetCombatStatus(ECombatStatus::Recovery);
		WeaponPtr->DeactivateCollision();
	}
}

void UMainCharacter_AnimInstance::AnimNotify_AttackEnd()
{
	if (MainPtr)
	{
		WeaponPtr = MainPtr->GetWeapon();
		WeaponPtr->DeactivateCollision();
		MainPtr->SetCombatStatus(ECombatStatus::CombatIdle);
		MainPtr->AttackEnd();
	}
}

void UMainCharacter_AnimInstance::AnimNotify_EvadeEnd()
{
	if (MainPtr)
	{
		MainPtr->EvadeEnd();
	}
}

void UMainCharacter_AnimInstance::AnimNotify_HitEnd()
{
	if (MainPtr)
	{
		MainPtr->HitEnd();
	}
}

void UMainCharacter_AnimInstance::AnimNotify_EatEnd()
{
	if (MainPtr)
	{
		MainPtr->EatEnd();
	}
}

void UMainCharacter_AnimInstance::AnimNotify_DeathEnd()
{
	if (MainPtr)
	{
		MainPtr->DeathEnd();
	}
}








