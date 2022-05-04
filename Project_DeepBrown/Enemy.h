// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MainCharacter.h"
#include "GameFramework/Character.h"
#include "Enemy.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnTakeDamageMulticast)

UENUM()
enum class EEnemyMovementStatus : uint8
{
	Idle,
	Combat,
	Attack,
	Hit,
	Evade,
	Die
};

UCLASS()
class PROJECT_DEEPBROWN_API AEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemy();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString Name;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MaxHealth;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float CurrentHealth;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MaxPoise;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float CurrentPoise;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float PoiseRecoverPerSec;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float AttackDamage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float AttackDelay;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float AttackSpeedModifier;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float AttackRange;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsAttacking;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsAttackCollisionActivated;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	EEnemyMovementStatus MovementStatus;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	ECombatStatus CombatStatus;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float DeathDelay; // 죽은 후 몇초후 시체가 사라질 것인지
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float RunSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float WalkSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float StrafeSpeed;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float TakeDamageInterval; // 데미지를 받는 간격 (한두프레임에 연속으로 공격 인식이 안되게 하기 위함)
	float TakeDamageIntervalRemained; // 현재 데미지를 받는 간격 (0이면 공격을 받을 수 있는 상태)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class UAnimMontage* EvadeAnimMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class UAnimMontage* HitAnimMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="UI")
	class TSubclassOf<UUserWidget> HealthBarWidgetClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	class TSubclassOf<UUserWidget> LockOnIconWidgetClass;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI")
	bool bIsShowingHealthBar;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "UI")
	bool bIsShowingLockOnIcon;
	class UUserWidget* HealthBarWidgetInstance;
	class UProgressBar* HealthBarProgressBar;
	class UTextBlock* HealthBarDamageText;
	class UUserWidget* LockOnIconWidgetInstance;

#pragma region Component References
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class USceneComponent* HealthBarHolder;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class USceneComponent* LockOnHolder;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class UChildActorComponent* WeaponComponent;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class AWeapon* WeaponPtr;
#pragma endregion

#pragma region AI
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI")
	TArray<FVector> PatrolPoints; // 패트롤 위치의 상대 위치
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	int32 PatrolDestinationIndex;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	float PatrolWaitTime;
	UFUNCTION(BlueprintCallable)
	void UpdatePatrolDestinationIndex();
#pragma endregion

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	class AEnemyAIController* EnemyAIController;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//UFUNCTION(BlueprintCallable)
	//void Attack(AActor* Target);
	UFUNCTION(BlueprintCallable)
	void Evade();
	void EvadeEnd();
	UFUNCTION(BlueprintCallable)
	void AttackEnd();
	UFUNCTION(BlueprintCallable)
	void HitAnimationEnd();
	FOnTakeDamageMulticast TakeDamageMulticast;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	float GetCurrentHealth() { return CurrentHealth; }

	// HUD
	void OnLockedOn();
	void OnUnlockedOn();
	void ShowHealthBar();
	void DestroyHealthBar();
	void ShowLockOnIcon();
	void DestroyLockOnIcon();
	void UpdateHUDPosition();
	UFUNCTION()
	void UpdateHealthBarPercent(); // Called by TakeDamageMulticast
	// End HUD

	UFUNCTION(BlueprintCallable)
	void SetMovementStatus(EEnemyMovementStatus Status) { MovementStatus = Status; };
	UFUNCTION(BlueprintCallable)
	void SetCombatStatus(ECombatStatus Status) { CombatStatus = Status; }
	UFUNCTION(BlueprintCallable)
	class AWeapon* GetWeapon();
	UFUNCTION(BlueprintCallable)
	void DeathEnd();

	UFUNCTION(BlueprintCallable)
	void PlaySpecialAttack();

protected:
	void Die(); // This function is called only by GetDamage()
	UFUNCTION(BlueprintImplementableEvent)
	void SpawnDropItems();
	void DestroySelf(); // This function is called only by Die()

	UFUNCTION(BlueprintCallable)
	EEnemyMovementStatus GetMovementStatus() const { return MovementStatus; }

	
};
