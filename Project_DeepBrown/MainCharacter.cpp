// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharacter.h"

#include "Enemy.h"
#include "FastAttackDamageType.h"
#include "MainCharacter_AnimInstance.h"
#include "Weapon.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Actor.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"


AMainCharacter::AMainCharacter()
{
	// Sets default values
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// set status
	MovementStatus = EMovementStatus::IdleMoving;
	CombatStatus = ECombatStatus::NormalIdle;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 450.0f; // The camera follows at this distance behind the character	
	CameraBoom->SocketOffset = FVector(0.f, 0.f, 50.f);
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	Weapon = CreateDefaultSubobject<UChildActorComponent>(TEXT("Weapon"));
	Weapon->SetChildActorClass(TSubclassOf<AWeapon>());
	Weapon->SetupAttachment(GetMesh(), TEXT("hand_rSocket"));

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)

	MaxHealth = 100.f;
	MaxStamina = 150.f;
	MaxHunger = 100.f;
	MaxThirst = 100.f;
	CurrentHealth = MaxHealth;
	CurrentStamina = MaxStamina;
	CurrentHunger = 0.f;
	CurrentThirst = 0.f;
	StaminaRecoveryRate = 30.f;
	HungerRatePerHour = 10.f;
	ThirstRatePerHour = 10.f;
	
	EvadeCost = 30.f;
	EvadeSpeedScaleFactor = 1.5f;
	bHasLockTarget = false;
	LockedOnEnemy = nullptr;
	CurrentAttackCombo = 1;
	TargetingRange = 5000.f;

	SetMovementStatus(EMovementStatus::IdleMoving);
}

void AMainCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (GetWeapon())
		GetWeapon()->HoldingActorPtr = this;
}

void AMainCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (MovementStatus == EMovementStatus::IdleMoving)
	{
		RecoverStamina(StaminaRecoveryRate * DeltaSeconds);
	}

	if (bHasLockTarget)
	{
		LookAtTarget(DeltaSeconds);
	}
}

// Called by GameState Tick()
void AMainCharacter::UpdateHungerAndThirst(float DeltaSecondsInGame)
{
	const float HungerRatePerSec = HungerRatePerHour / 3600.f;
	const float ThirstRatePerSec = ThirstRatePerHour / 3600.f;

	CurrentHunger = FMath::Clamp(CurrentHunger + HungerRatePerSec * DeltaSecondsInGame, 0.f, MaxHunger);
	CurrentThirst = FMath::Clamp(CurrentThirst + ThirstRatePerSec * DeltaSecondsInGame, 0.f, MaxThirst);
}


void AMainCharacter::LookAtTarget (float DeltaSeconds)
{
	if (LockedOnEnemy)
	{
		if (LockedOnEnemy->GetCurrentHealth() <= 0.f)
		{
			bHasLockTarget = false;
			LockedOnEnemy = nullptr;
			GetCharacterMovement()->bOrientRotationToMovement = 1;
			return;
		}

		FVector LockedOnLocation = LockedOnEnemy->GetActorLocation();
		LockedOnLocation.Z -= 150.f;
		const FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), LockedOnLocation);
		const FRotator InterpRotation = UKismetMathLibrary::RInterpTo(GetController()->GetControlRotation(), LookAtRotation, DeltaSeconds, 10.f);
		GetController()->SetControlRotation(FRotator(InterpRotation.Pitch, InterpRotation.Yaw, GetController()->GetControlRotation().Roll));
		const FRotator PawnInterpRotation = UKismetMathLibrary::RInterpTo(GetActorRotation(), FRotator(GetActorRotation().Pitch, GetControlRotation().Yaw, GetActorRotation().Roll), DeltaSeconds, 10.f);
		GetController()->GetPawn()->SetActorRotation(PawnInterpRotation);
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AMainCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("FastShortAttack", IE_Pressed, this, &AMainCharacter::Attack_FastShort);
	PlayerInputComponent->BindAction("SlowLongAttack", IE_Pressed, this, &AMainCharacter::Attack_SlowLong);
	PlayerInputComponent->BindAction("LockOn", IE_Pressed, this, &AMainCharacter::LockOn);
	PlayerInputComponent->BindAction("Evade", IE_Pressed, this, &AMainCharacter::Evade);
	PlayerInputComponent->BindAction("ToggleInventory", IE_Pressed, this, &AMainCharacter::ToggleInventory);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMainCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMainCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AMainCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AMainCharacter::LookUpAtRate);
}

void AMainCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AMainCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AMainCharacter::MoveForward(float Value)
{
	ForwardInputValue = Value;
	if ((Controller != nullptr) && (Value != 0.0f) && MovementStatus != EMovementStatus::Death && MovementStatus != EMovementStatus::Evading)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AMainCharacter::MoveRight(float Value)
{
	RightInputValue = Value;
	if ((Controller != nullptr) && (Value != 0.0f) && MovementStatus != EMovementStatus::Death && MovementStatus != EMovementStatus::Evading)
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void AMainCharacter::Evade(void)
{
	// 타겟락을 하고 있지 않으면, 앞 뒤로면 회피
	if (bHasLockTarget == false && MovementStatus == EMovementStatus::IdleMoving && EvadeForwardMontage && CurrentStamina - EvadeCost >= 0.f)
	{
		SetMovementStatus(EMovementStatus::Evading);
		ReduceStamina(EvadeCost);
		if (ForwardInputValue <= 0.01f && RightInputValue <= 0.01f)
		{
			PlayAnimMontage(EvadeForwardMontage, EvadeSpeedScaleFactor);
		}
		else
		{
			PlayAnimMontage(EvadeForwardMontage, EvadeSpeedScaleFactor, TEXT("Forward"));
		}
	}
	// 타겟락을 하고 있을 경우, 현재 입력값에 따라 회피
	else if (bHasLockTarget && MovementStatus == EMovementStatus::IdleMoving && EvadeForwardMontage && CurrentStamina - EvadeCost >= 0.f)
	{
		UMainCharacter_AnimInstance* AnimInstance = Cast<UMainCharacter_AnimInstance>(GetMesh()->GetAnimInstance());
		if (AnimInstance)
		{
			AnimInstance->SetEvadeValues(ForwardInputValue, RightInputValue); // 이 값은 BS에 전달된다.
			SetMovementStatus(EMovementStatus::Evading);
			ReduceStamina(EvadeCost);
			FTimerHandle TimerHandle;
			GetWorldTimerManager().SetTimer(TimerHandle, this, &AMainCharacter::EvadeEnd, 0.8f);
		}
	}
	// 죽어있을 경우, 리스타트
	else if (MovementStatus == EMovementStatus::Death)
	{
		UGameplayStatics::OpenLevel(GetWorld(), TEXT("TestDungeon"));
	}
}

void AMainCharacter::ToggleInventory()
{
	if (InventoryWidgetInstance == nullptr) return;

	// Close Inventory
	if (InventoryWidgetInstance->IsInViewport())
	{
		InventoryWidgetInstance->RemoveFromParent();
		APlayerController* PlayerController = Cast<APlayerController>(GetController());
		PlayerController->SetInputMode(FInputModeGameOnly());
		PlayerController->bShowMouseCursor = false;
	}
	// Open Inventory
	else
	{
		InventoryWidgetInstance->AddToViewport();
		APlayerController* PlayerController = Cast<APlayerController>(GetController());
		PlayerController->SetInputMode(FInputModeGameAndUI());
		PlayerController->bShowMouseCursor = true;
	}
}


void AMainCharacter::EvadeEnd(void)
{
	SetMovementStatus(EMovementStatus::IdleMoving);
}

void AMainCharacter::EatEnd(void)
{
	SetMovementStatus(EMovementStatus::IdleMoving);
}

void AMainCharacter::Death()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && DeathMontage)
	{
		MovementStatus = EMovementStatus::Death;
		PlayAnimMontage(DeathMontage, 1, TEXT("Death"));
		LockedOnEnemy = nullptr;
		bHasLockTarget = false;
	}
}

void AMainCharacter::DeathEnd()
{
	GetMesh()->bPauseAnims = true;
	if (RestartWidgetClass)
	{
		auto WidgetPtr = CreateWidget<UUserWidget>(GetWorld(), RestartWidgetClass);
		WidgetPtr->AddToViewport(0);
	}
}

void AMainCharacter::HitEnd()
{
	SetMovementStatus(EMovementStatus::IdleMoving);
}

float AMainCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	DamageAmount = DamageEvent.DamageTypeClass == TSubclassOf<UFastAttackDamageType>() ? DamageAmount : DamageAmount * 1.5f;
	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (MovementStatus != EMovementStatus::Death && MovementStatus != EMovementStatus::Evading)
	{
		
		CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.f, MaxHealth);

		if (CurrentHealth <= 0.f)
			Death();
		else if (DeathMontage)
		{
			PlayAnimMontage(DeathMontage, 1.f, TEXT("Hit"));
			SetMovementStatus(EMovementStatus::Hit);
		}

		UE_LOG(LogDamage, Log, TEXT("Main character took damage."));
	}

	return DamageAmount;
}

void AMainCharacter::RecoverStamina(float Amount)
{
	CurrentStamina = FMath::Clamp(CurrentStamina + Amount, 0.f, MaxStamina);
}


void AMainCharacter::ReduceStamina(float Amount)
{
	CurrentStamina = FMath::Clamp(CurrentStamina - Amount, 0.f, MaxStamina);
}

void AMainCharacter::RecoverHunger(float Amount)
{
	CurrentHunger = FMath::Clamp(CurrentHunger - Amount, 0.f, MaxHunger);
}

void AMainCharacter::RecoverThirst(float Amount)
{
	CurrentThirst = FMath::Clamp(CurrentThirst - Amount, 0.f, MaxThirst);
}


void AMainCharacter::Attack_FastShort()
{
	if ((MovementStatus == EMovementStatus::IdleMoving || (MovementStatus == EMovementStatus::Attacking && CombatStatus == ECombatStatus::Recovery))
		&& GetWeapon() && CurrentStamina - GetWeapon()->FastAttackStaminaCost >= 0.f)
	{
		EMovementStatus PostMovementStatus = GetMovementStatus();
		SetMovementStatus(EMovementStatus::Attacking);

		ReduceStamina(GetWeapon()->FastAttackStaminaCost);
		GetWeapon()->IsSlowAttacking = false;

		AWeapon* WeaponClass = Cast<AWeapon>(Weapon->GetChildActor());
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

		if (PostMovementStatus == EMovementStatus::IdleMoving || CurrentAttackCombo + 1 > WeaponClass->GetFastShortAttackMaxCombo())
			CurrentAttackCombo = 1;
		else
			CurrentAttackCombo++;
		UE_LOG(LogTemp, Warning, TEXT("%d - %d"), CurrentAttackCombo, WeaponClass->GetFastShortAttackMaxCombo());
		AnimInstance->Montage_Play(WeaponClass->FastShortAttackAnimMontage);
		if (WeaponClass && AnimInstance)
		{
			AnimInstance->Montage_JumpToSection(FName(FString::FromInt(CurrentAttackCombo)));
		}
	}
}

void AMainCharacter::Attack_SlowLong()
{
	if ((MovementStatus == EMovementStatus::IdleMoving || (MovementStatus == EMovementStatus::Attacking && CombatStatus == ECombatStatus::Recovery))
		&& GetWeapon() && CurrentStamina - GetWeapon()->SlowAttackStaminaCost >= 0.f)
	{
		EMovementStatus PostMovementStatus = GetMovementStatus();
		SetMovementStatus(EMovementStatus::Attacking);

		ReduceStamina(GetWeapon()->SlowAttackStaminaCost);
		GetWeapon()->IsSlowAttacking = true;

		AWeapon* WeaponClass = Cast<AWeapon>(Weapon->GetChildActor());
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

		if (MovementStatus == EMovementStatus::IdleMoving || CurrentAttackCombo + 1 > WeaponClass->GetSlowLongAttackMaxCombo())
			CurrentAttackCombo = 1;
		else
			CurrentAttackCombo++;
		
		AnimInstance->Montage_Play(WeaponClass->SlowLongAttackAnimMontage);
		if (WeaponClass && AnimInstance)
		{
			AnimInstance->Montage_JumpToSection(FName(FString::FromInt(CurrentAttackCombo)));
		}
	}
}

void AMainCharacter::LockOn()
{
	if (bHasLockTarget == false)
	{
		// 120도의 부채꼴 모양으로 레이트레이싱을 실행한다.
		FVector CameraForwardVector = GetFollowCamera()->GetForwardVector().RotateAngleAxis(15.f, FVector::LeftVector);
		FVector LeftEndVector = CameraForwardVector.RotateAngleAxis(-60.f, FVector::UpVector);
		TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypeToLock;
		ObjectTypeToLock.Add(EObjectTypeQuery::ObjectTypeQuery3);
		FCollisionQueryParams QueryParams(NAME_None);
		TArray<AActor*> ActorsToNotTargeting;
		ActorsToNotTargeting.Add(this);
		FVector StartPoint = GetFollowCamera()->GetComponentLocation();
		FHitResult HitResult;
		float ClosestDist = TargetingRange;
		AActor* ClosestHitActor = nullptr;
		for (int i = 0; i < 120; i += 5)
		{
			FVector Direction = LeftEndVector.RotateAngleAxis(i, FVector::UpVector);
			FVector EndPoint = StartPoint + Direction * TargetingRange;
			bool bIsHit = UKismetSystemLibrary::SphereTraceSingleForObjects(
				GetWorld(), StartPoint, EndPoint, 200.f,
				ObjectTypeToLock, false, ActorsToNotTargeting, EDrawDebugTrace::None,
				HitResult, true,
				FLinearColor::Red, FLinearColor::Green, 2.f);
			if (bIsHit && HitResult.Distance < ClosestDist)
			{
				ClosestDist = HitResult.Distance;
				ClosestHitActor = HitResult.GetActor();
			}
		}
		if (ClosestHitActor)
		{
			AEnemy* NewLockedOnEnemy = Cast<AEnemy>(ClosestHitActor);
			if (LockedOnEnemy)
			{
				LockedOnEnemy->OnUnlockedOn();
			}
			if (NewLockedOnEnemy)
			{
				LockedOnEnemy = NewLockedOnEnemy;
				bHasLockTarget = true;
				LockedOnEnemy->OnLockedOn();
				GetCharacterMovement()->bOrientRotationToMovement = 0;
			}
		}
	}
	else
	{
		bHasLockTarget = false;
		if (LockedOnEnemy)
		{
			LockedOnEnemy->OnUnlockedOn();
			LockedOnEnemy = nullptr;
		}
		
		GetCharacterMovement()->bOrientRotationToMovement = 1;
	}
}



// 공격 애니메이션의 공격 부분이 끝났을 때 발동
// 다음 공격 콤보를 넣거나 구를 수 있다.
void AMainCharacter::AttackEnd()
{
	SetMovementStatus(EMovementStatus::IdleMoving);
	if (GetWeapon())
		GetWeapon()->DeactivateCollision();
}

// 공격 애니메이션이 완전히 끝났을 때 발동
// 콤보를 초기화한다.
void AMainCharacter::ResetCombo()
{
	CurrentAttackCombo = 0;
}


