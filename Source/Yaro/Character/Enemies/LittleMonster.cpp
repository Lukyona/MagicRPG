// Fill out your copyright notice in the Description page of Project Settings.


#include "LittleMonster.h"

ALittleMonster::ALittleMonster()
{
	SetAgroSphere(200.f);
	SetCombatSphere(75.f);

	CreateFirstWeaponCollision();
	CreateSecondWeaponCollision();

	//This is the default value, each enemy has different health.
	InitHealth(700.f);

	Damage = 45.f;
	EnemyExp = 60.f;

	AttackDelay = 0.5f;

}

void ALittleMonster::BeginPlay()
{
	Super::BeginPlay();

	EnableFirstWeaponCollision();
	EnableSecondWeaponCollision();
}