// Fill out your copyright notice in the Description page of Project Settings.
#include "Goblin.h"

AGoblin::AGoblin()
{
	EnemyType = EEnemyType::Goblin;

	SetAgroSphere(500.f);
	SetCombatSphere(110.f);

	CreateFirstWeaponCollision();

	//This is the default value, each enemy has different health.
	InitHealth(180.f);

	Damage = 20.f;
	EnemyExp = 30.f;

	AttackDelay = 0.2f;

}

void AGoblin::BeginPlay()
{
	Super::BeginPlay();

	EnableFirstWeaponCollision();
}