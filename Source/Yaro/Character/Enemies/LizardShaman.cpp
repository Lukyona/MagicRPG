// Fill out your copyright notice in the Description page of Project Settings.


#include "LizardShaman.h"

ALizardShaman::ALizardShaman()
{
	EnemyType = EEnemyType::LizardShaman;
	
	bIsRangedAttacker = true;

	MaxHealth = 500.f;
	Damage = 0.f;
	EnemyExp = 50.f;
	AttackDelay = 0.5f;

	AgroSphereRadius = 600.f;
	CombatSphereRadius = 400.f;

	CreateSpheresAndCollisions();

}

void ALizardShaman::Disappear()
{
	Super::DisableSphereCollisions();
	Super::Destroy();
}
