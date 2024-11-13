// Fill out your copyright notice in the Description page of Project Settings.


#include "Golem.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"


AGolem::AGolem()
{
	EnemyType = EEnemyType::Golem;

	SetAgroSphere(700.f);
	SetCombatSphere(200.f);

	CreateFirstWeaponCollision();
	CreateSecondWeaponCollision();

	//This is the default value, each enemy has different health.
	InitHealth(800.f);

	Damage = 60.f;
	EnemyExp = 80.f;

	AttackDelay = 0.7f;

}

void AGolem::BeginPlay()
{
	Super::BeginPlay();

	EnableFirstWeaponCollision();
	EnableSecondWeaponCollision();
}

void AGolem::HitGround() //Golem's third skill
{
	if (SkillSound) UGameplayStatics::PlaySound2D(this, SkillSound);

	if (CombatTarget)
	{
		TArray<AActor*> ignoredActors;
		UGameplayStatics::ApplyRadialDamage(GetWorld(), 80.f, GetActorLocation(), 300.f, UDamageType::StaticClass(),
			ignoredActors,
			this,
			nullptr,
			false,
			ECollisionChannel::ECC_Visibility);
			//Damage(CombatTarget, Damage, AIController, this, DamageTypeClass);
	}
}