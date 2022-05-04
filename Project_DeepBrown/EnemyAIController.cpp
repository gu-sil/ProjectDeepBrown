// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAIController.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"

void AEnemyAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AEnemyAIController::SetTarget(AActor* Target)
{
	GetBlackboardComponent()->SetValueAsObject("Player", Target);
	CurrentTarget = Target;
}

void AEnemyAIController::SetAttackRangeInBB(float AttackRange)
{
	GetBlackboardComponent()->SetValueAsFloat("AttackRange", AttackRange);
}

float AEnemyAIController::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	RegisterDamageOnPerception(DamageCauser);
	return DamageAmount;
}


float AEnemyAIController::GetDistToTarget(AActor* Target) const
{
	return FVector::Dist(this->GetPawn()->GetActorLocation(), Target->GetActorLocation());
}