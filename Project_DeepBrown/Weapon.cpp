// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Kismet/GameplayStatics.h"
#include "AIModule/Classes/Perception/AIPerceptionComponent.h"
#include "Enemy.h"
#include "FastAttackDamageType.h"
#include "SlowAttackDamageType.h"

// Sets default values
AWeapon::AWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMesh->SetupAttachment(RootComponent);

	AttackCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Attack Collision"));
	AttackCollision->SetupAttachment(StaticMesh);

	FastAttackStaminaCost = 20.f;
	SlowAttackStaminaCost = 40.f;

	IsSlowAttacking = false;
	IsEnemyWeapon = false;
}

// Called when the game starts or when spawned
void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	DeactivateCollision();
	AttackCollision->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnAttackCollisionOverlapBegin);
}

// Called every frame
void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWeapon::OnAttackCollisionOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UClass* DamageType;
	if (IsSlowAttacking)
		DamageType = USlowAttackDamageType::StaticClass();
	else
		DamageType = UFastAttackDamageType::StaticClass();

	if (IsSlowAttacking)
		UE_LOG(LogTemp, Warning, TEXT("Slow!"));
	
	UE_LOG(LogTemp, Warning, TEXT("weapon attack! %s %f"), *OverlappedComp->GetName());
	if (IsEnemyWeapon == false)
	{
		AEnemy* Enemy = Cast<AEnemy>(OtherActor);
		if (Enemy)
		{
			UGameplayStatics::ApplyDamage(Enemy, AttackDamage, Enemy->GetController(), HoldingActorPtr, DamageType);
		}
	}
	else
	{
		AMainCharacter* Main = Cast<AMainCharacter>(OtherActor);
		if (Main)
		{
			UE_LOG(LogTemp, Warning, TEXT("enemy weapon attack!"));
			UGameplayStatics::ApplyDamage(Main, AttackDamage, Main->GetController(), HoldingActorPtr, DamageType);
		}
	}
}



