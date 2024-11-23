// Fill out your copyright notice in the Description page of Project Settings.


#include "LizardShaman.h"

ALizardShaman::ALizardShaman()
{
	EnemyType = EEnemyType::LizardShaman;

	SetAgroSphere(600.f);
	SetCombatSphere(400.f);


	//This is the default value, each enemy has different health.
	InitHealth(500.f);

	Damage = 0.f;
	EnemyExp = 50.f;

	AttackDelay = 0.5f;

}

void ALizardShaman::BeginPlay()
{
	Super::BeginPlay();
}

void ALizardShaman::Disappear()
{
	Super::DisableSphereCollisions();

	Super::Destroy();
}