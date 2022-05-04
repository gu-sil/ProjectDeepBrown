// Fill out your copyright notice in the Description page of Project Settings.

#include "Enemy.h"
#include "Animation/AnimInstance.h"
#include "MainCharacter.h"
#include "EnemyAIController.h"
#include "FastAttackDamageType.h"
#include "Blueprint/UserWidget.h"
#include "Components/BoxComponent.h"
#include "Components/ProgressBar.h"
#include "Components/SphereComponent.h"
#include "Components/SplineComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Tasks/AITask.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TextBlock.h"

// Sets default values
AEnemy::AEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Name = "";
	MaxHealth = 100.f;
	CurrentHealth = MaxHealth;
	MaxPoise = 20.f;
	CurrentPoise = MaxPoise;
	PoiseRecoverPerSec = 2.f;
	AttackDamage = 20.f;
	AttackDelay = 2.f;
	AttackSpeedModifier = 1.f;
	AttackRange = 100.f;
	DeathDelay = 5.f;
	bIsAttacking = false;
	bIsAttackCollisionActivated = false;

	RunSpeed = 600.f;
	WalkSpeed = 300.f;
	StrafeSpeed = 150.f;
	TakeDamageInterval = 0.1f;

	WeaponComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("Weapon"));
	WeaponComponent->SetChildActorClass(TSubclassOf<AWeapon>());
	WeaponComponent->SetupAttachment(GetMesh(), TEXT("hand_rSocket"));

	// HUD
	HealthBarHolder = CreateDefaultSubobject<USceneComponent>(TEXT("Health Bar Holder"));
	HealthBarHolder->SetupAttachment(RootComponent);

	LockOnHolder = CreateDefaultSubobject<USceneComponent>(TEXT("LockOn Holder"));
	LockOnHolder->SetupAttachment(RootComponent);

	bIsShowingHealthBar = false;
	bIsShowingLockOnIcon = false;
	// End HUD

	// Set AI Variables
	PatrolDestinationIndex = 0;
	PatrolWaitTime = 2.f;
}

#pragma region Overriding Functions
// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	EnemyAIController = Cast<AEnemyAIController>(GetController());
	
	SetMovementStatus(EEnemyMovementStatus::Idle);
	SetCombatStatus(ECombatStatus::NormalIdle);

	WeaponPtr = Cast<AWeapon>(WeaponComponent->GetChildActor());

	for (FVector& point : PatrolPoints)
		point += GetActorLocation();

	TakeDamageMulticast.AddUFunction(this, TEXT("UpdateHealthBarPercent"));
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (TakeDamageIntervalRemained > 0.f)
		TakeDamageIntervalRemained -= DeltaTime;
	if (CurrentPoise < MaxPoise)
		CurrentPoise = FMath::Clamp(CurrentPoise + PoiseRecoverPerSec * DeltaTime, 0.f, MaxPoise);
	UpdateHUDPosition();
}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}
#pragma endregion Overriding Functions

#pragma region Combat Functions

float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (TakeDamageIntervalRemained > 0.f || CurrentHealth <= 0.f)
		return 0;

	DamageAmount = DamageEvent.DamageTypeClass == UFastAttackDamageType::StaticClass() ? DamageAmount : DamageAmount * 1.5f;
	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	// AI에게 피해를 입었음을 알린다.
	GetController()->TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.f, MaxHealth);
	CurrentPoise = FMath::Clamp(CurrentPoise - DamageAmount, 0.f, MaxPoise);

	if (CurrentHealth == 0.f)
	{
		Die();
	}
	else
	{
		if (HitAnimMontage && CurrentPoise == 0.f)
		{
			float YawAngle = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), DamageCauser->GetActorLocation()).Yaw - GetActorRotation().Yaw;
			if (YawAngle >= 50.f && YawAngle <= 180.f)
				PlayAnimMontage(HitAnimMontage, 1, TEXT("Right"));
			else if (YawAngle > -50.f && YawAngle <= 50.f)
				PlayAnimMontage(HitAnimMontage, 1, TEXT("Front"));
			else
				PlayAnimMontage(HitAnimMontage, 1, TEXT("Left"));
			UE_LOG(LogTemp, Log, TEXT("Angle: %f"), YawAngle);
			SetMovementStatus(EEnemyMovementStatus::Hit);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("%s took %f damage."), *GetName(), DamageAmount);
	TakeDamageIntervalRemained = TakeDamageInterval;

	TakeDamageMulticast.Broadcast();

	return DamageAmount;
}

void AEnemy::HitAnimationEnd()
{
	SetMovementStatus(EEnemyMovementStatus::Combat);
}

void AEnemy::AttackEnd()
{
	
}

#pragma endregion Combat Functions
AWeapon* AEnemy::GetWeapon()
{
	return WeaponPtr;
}

// Called by BTT
void AEnemy::PlaySpecialAttack()
{
	if (WeaponPtr && WeaponPtr->SpecialAttackAnimMontage)
	{
		PlayAnimMontage(WeaponPtr->SpecialAttackAnimMontage);
	}
}

void AEnemy::Evade()
{
	if (EvadeAnimMontage)
	{
		SetMovementStatus(EEnemyMovementStatus::Evade);
		PlayAnimMontage(EvadeAnimMontage);
	}
}

void AEnemy::EvadeEnd()
{
	SetMovementStatus(EEnemyMovementStatus::Combat);
}

void AEnemy::ShowHealthBar()
{
	if (HealthBarWidgetClass)
	{
		HealthBarWidgetInstance = CreateWidget(GetWorld(), HealthBarWidgetClass, TEXT("Health Bar"));
		HealthBarWidgetInstance->AddToViewport();
		HealthBarProgressBar = Cast<UProgressBar>(HealthBarWidgetInstance->GetWidgetFromName(TEXT("HealthBar_0")));
		bIsShowingHealthBar = true;
	}
}

void AEnemy::ShowLockOnIcon()
{
	if (LockOnIconWidgetClass)
	{
		LockOnIconWidgetInstance = CreateWidget(GetWorld(), LockOnIconWidgetClass, TEXT("Lock On Icon"));
		LockOnIconWidgetInstance->AddToViewport();
		bIsShowingLockOnIcon = true;
	}
}

void AEnemy::DestroyHealthBar()
{
	if (bIsShowingHealthBar && HealthBarWidgetInstance)
	{
		HealthBarWidgetInstance->RemoveFromParent();
		HealthBarWidgetInstance = nullptr;
		HealthBarProgressBar = nullptr;
	}
}

void AEnemy::DestroyLockOnIcon()
{
	if (bIsShowingLockOnIcon && LockOnIconWidgetInstance)
	{
		LockOnIconWidgetInstance->RemoveFromParent();
		LockOnIconWidgetInstance = nullptr;
	}
}

void AEnemy::OnLockedOn()
{
	ShowHealthBar();
	UpdateHealthBarPercent();
	ShowLockOnIcon();
}

void AEnemy::OnUnlockedOn()
{
	DestroyHealthBar();
	DestroyLockOnIcon();
}

void AEnemy::UpdateHUDPosition()
{
	if (bIsShowingHealthBar && HealthBarWidgetInstance)
	{
		auto PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		FVector2D ScreenPos;
		UGameplayStatics::ProjectWorldToScreen(PlayerController, HealthBarHolder->GetComponentLocation(), ScreenPos);
		HealthBarWidgetInstance->SetPositionInViewport(ScreenPos);
	}

	if (bIsShowingLockOnIcon && LockOnIconWidgetInstance)
	{
		auto PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		FVector2D ScreenPos;
		UGameplayStatics::ProjectWorldToScreen(PlayerController, LockOnHolder->GetComponentLocation(), ScreenPos);
		LockOnIconWidgetInstance->SetPositionInViewport(ScreenPos);
	}
}

void AEnemy::UpdateHealthBarPercent()
{
	if (bIsShowingHealthBar && HealthBarProgressBar)
	{
		float HealthBarPercent = CurrentHealth / MaxHealth;
		HealthBarProgressBar->SetPercent(HealthBarPercent);
	}
}

void AEnemy::Die()
{
	auto AnimInstance = GetMesh()->GetAnimInstance();
	SpawnDropItems();
	if (AnimInstance)
	{
		SetMovementStatus(EEnemyMovementStatus::Die);
		EnemyAIController->StopMovement();
		EnemyAIController->UnPossess();
		if (HitAnimMontage)
		{
			PlayAnimMontage(HitAnimMontage, 1, TEXT("Die"));
		}

		// Make Death Timer
		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle, this, &AEnemy::DestroySelf, DeathDelay, false);

		
		DestroyHealthBar();
		DestroyLockOnIcon();
	}
}

void AEnemy::DestroySelf()
{
	Destroy();
}

void AEnemy::DeathEnd()
{
	GetMesh()->bPauseAnims = true;
}

#pragma region AI

void AEnemy::UpdatePatrolDestinationIndex()
{
	PatrolDestinationIndex++;
	if (PatrolDestinationIndex >= PatrolPoints.Num())
		PatrolDestinationIndex = 0;
}

#pragma endregion

