// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

UCLASS()
class PROJECT_DEEPBROWN_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackDamage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int FastShortAttackMaxCombo;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int SlowLongAttackMaxCombo;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UStaticMeshComponent* StaticMesh;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class UBoxComponent* AttackCollision;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class UAnimMontage* FastShortAttackAnimMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class UAnimMontage* SlowLongAttackAnimMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class UAnimMontage* SpecialAttackAnimMontage;
	UPROPERTY(BlueprintReadWrite)
	class AActor* HoldingActorPtr; // 무기를 쥐고 있는 액터
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float FastAttackStaminaCost;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float SlowAttackStaminaCost;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool IsSlowAttacking;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool IsEnemyWeapon;

	UFUNCTION(BlueprintCallable)
	void ActivateCollision() const { AttackCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly); };
	UFUNCTION(BlueprintCallable)
	void DeactivateCollision() const { AttackCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision); };
	UFUNCTION()
	void OnAttackCollisionOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	int GetFastShortAttackMaxCombo() const { return FastShortAttackMaxCombo; }
	int GetSlowLongAttackMaxCombo() const { return SlowLongAttackMaxCombo; }
	UAnimMontage* GetFastShortAttackAnimMontage() const { return FastShortAttackAnimMontage; };
	UAnimMontage* GetSlowLongAttackAnimMontage() const { return SlowLongAttackAnimMontage; };

	FORCEINLINE float GetAttackDamage() const { return AttackDamage; };
};
