// Fill out your copyright notice in the Description page of Project Settings.


#include "LittleDino.h"

ALittleDino::ALittleDino()
{
	SetAgroSphere(200.f);
	SetCombatSphere(30.f);

	CreateFirstWeaponCollision();
	CreateSecondWeaponCollision();

	//This is the default value, each enemy has different health.
	InitHealth(200.f);

	Damage = 30.f;
	EnemyExp = 30.f;

	AttackDelay = 0.2f;

}

void ALittleDino::BeginPlay()
{
	Super::BeginPlay();

	EnableFirstWeaponCollision();
	EnableSecondWeaponCollision();
}
