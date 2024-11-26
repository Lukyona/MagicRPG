// Fill out your copyright notice in the Description page of Project Settings.


#include "Boss.h"

ABoss::ABoss()
{
	EnemyType = EEnemyType::Boss;

	bIsRangedAttacker = true;

	MaxHealth = 1000.f;
	Damage = 60.f;
	EnemyExp = 100.f;
	AttackDelay = 0.4f;

	AgroSphereRadius = 750.f;
	CombatSphereRadius = 600.f;

	CreateSpheresAndCollisions();

}

void ABoss::Disappear()
{
	Super::DisableSphereCollisions();
	Super::Destroy();
}
