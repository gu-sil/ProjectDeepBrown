// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "GameFramework/Character.h"
#include "MainCharacter.generated.h"


UENUM()
enum class EMovementStatus : uint8
{
	IdleMoving,
	Attacking,
	Evading,
	Hit,
	Eating,
	Death
};

UENUM()
enum class ECombatStatus : uint8
{
	NormalIdle,
	CombatIdle,
	Anticipation,
	Contact,
	Recovery
};

UCLASS()
class PROJECT_DEEPBROWN_API AMainCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UChildActorComponent* Weapon;

	UPROPERTY(VisibleAnywhere)
	EMovementStatus MovementStatus;
	UPROPERTY(VisibleAnywhere)
	ECombatStatus CombatStatus;

public:
	AMainCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseLookUpRate;

#pragma region UI
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class TSubclassOf<UUserWidget> RestartWidgetClass;
#pragma endregion

protected:
#pragma region Combat Functions & Variables
	bool bHasLockTarget;
	class AEnemy* LockedOnEnemy;
	int CurrentAttackCombo;
	float TargetingRange;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAnimMontage* DeathMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAnimMontage* EvadeForwardMontage;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAnimMontage* EvadeRightMontage;

	
	void Attack_FastShort();
	void Attack_SlowLong();
	
	void ResetCombo();
	void LockOn();
	void LookAtTarget(float DeltaSeconds);

	

#pragma  endregion

#pragma region Movement Functions

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	void Evade();

	void Death();

#pragma endregion Movement Fuctions

#pragma region Movement Variables
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float RightInputValue;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float ForwardInputValue;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float EvadeCost;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float EvadeSpeedScaleFactor;
	float EvadeForwardValue;
	float EvadeRightValue;

#pragma endregion Movement Variables

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	// End of APawn interface

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHealth;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxStamina;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MaxHunger;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MaxThirst;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float CurrentHealth;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float CurrentStamina;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float CurrentHunger;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float CurrentThirst;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float StaminaRecoveryRate; // 1초에 회복하는 스태미나량
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float HungerRatePerHour; // 게임시간 1시간에 따른 배고픔 상승치
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ThirstRatePerHour; // 게임시간 1시간에 따른 목마름 상승치

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UFUNCTION(BlueprintCallable)
	AWeapon* GetWeapon()
	{
		return Cast<AWeapon>(Weapon->GetChildActor());
	};
	UFUNCTION(BlueprintCallable)
	void SetMovementStatus(EMovementStatus Status) { MovementStatus = Status; }
	UFUNCTION(BlueprintCallable)
	EMovementStatus GetMovementStatus() { return MovementStatus; }
	UFUNCTION(BlueprintCallable)
	void SetCombatStatus(ECombatStatus Status) { CombatStatus = Status; }
	UFUNCTION(BlueprintCallable)
	bool GetHasLockTarget() { return bHasLockTarget; };
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	float GetForwardInputValue() { return ForwardInputValue; }
	float GetRightInputValue() { return RightInputValue; }
	void RecoverStamina(float Amount);
	void ReduceStamina(float Amount);
	UFUNCTION(BlueprintCallable)
	void RecoverHunger(float Amount);
	UFUNCTION(BlueprintCallable)
	void RecoverThirst(float Amount);
	UFUNCTION(BlueprintCallable)
	void UpdateHungerAndThirst(float DeltaSecondsInGame);
	UFUNCTION(BlueprintCallable)
	void AttackEnd();
	UFUNCTION(BlueprintCallable)
	void EatEnd();
	UFUNCTION(BlueprintCallable)
	void DeathEnd();
	UFUNCTION(BlueprintCallable)
	void HitEnd();
	UFUNCTION(BlueprintCallable)
	void EvadeEnd();

public:
#pragma region Inventory System
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Inventory")
	class UUserWidget* InventoryWidgetInstance;

	UFUNCTION()
	void ToggleInventory();
#pragma endregion


};
