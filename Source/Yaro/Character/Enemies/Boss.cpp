// Fill out your copyright notice in the Description page of Project Settings.


#include "Boss.h"

ABoss::ABoss()
{
	EnemyType = EEnemyType::Boss;

	SetAgroSphere(750.f);
	SetCombatSphere(600.f);

	//This is the default value, each enemy has different health.
	InitHealth(1000.f);

	Damage = 60.f;
	EnemyExp = 100.f;

	AttackDelay = 0.4f;
}

void ABoss::BeginPlay()
{
	Super::BeginPlay();
}

void ABoss::Disappear()
{
	Super::SphereCollisionDisabled();
	Super::Destroy();
}