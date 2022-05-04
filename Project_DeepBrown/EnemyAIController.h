// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_DEEPBROWN_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:

	virtual void Tick(float DeltaSeconds) override;

	void SetTarget(AActor* Target);
	void SetAttackRangeInBB(float AttackRange);
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable)
	float GetDistToTarget(AActor* Target) const;

	UFUNCTION(BlueprintImplementableEvent)
	void RegisterDamageOnPerception(AActor* DamageCauser);

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class AActor* CurrentTarget;

protected:
	UPROPERTY(VisibleAnywhere, BLueprintReadOnly)
	float DistToTarget;
};
