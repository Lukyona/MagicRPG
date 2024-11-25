// Fill out your copyright notice in the Description page of Project Settings.


#include "Archer.h"

AArcher::AArcher()
{
	EnemyType = EEnemyType::Archer;

	bIsRangedAttacker = true;

	//This is the default value, each enemy has different values.
	MaxHealth = 450.f;
	Damage = 40.f;
	EnemyExp = 40.f;
	AttackDelay = 0.6f;

	AgroSphereRadius = 430.f;
	CombatSphereRadius = 400.f;

	CreateSpheresAndCollisions();
}

void AArcher::Disappear()
{
	Super::DisableSphereCollisions();

	Super::Destroy();
}