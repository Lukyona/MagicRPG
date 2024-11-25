// Fill out your copyright notice in the Description page of Project Settings.


#include "Golem.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"


AGolem::AGolem()
{
	EnemyType = EEnemyType::Golem;

	hasSecondCollision = true;
	
	MaxHealth = 800.f;
	Damage = 60.f;
	EnemyExp = 80.f;
	AttackDelay = 0.7f;

	AgroSphereRadius = 700.f;
	CombatSphereRadius = 200.f;
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
	}
}