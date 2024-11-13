// Fill out your copyright notice in the Description page of Project Settings.


#include "Lizard.h"

ALizard::ALizard()
{
	EnemyType = EEnemyType::Lizard;

	SetAgroSphere(450.f);
	SetCombatSphere(110.f);

	CreateFirstWeaponCollision();
	CreateSecondWeaponCollision();

	//This is the default value, each enemy has different health.
	InitHealth(400.f);

	Damage = 45.f;
	EnemyExp = 45.f;

	AttackDelay = 0.3f;

}

void ALizard::BeginPlay()
{
	Super::BeginPlay();

	EnableFirstWeaponCollision();
	EnableSecondWeaponCollision();
}

