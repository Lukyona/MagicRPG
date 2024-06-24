// Fill out your copyright notice in the Description page of Project Settings.


#include "Archer.h"

AArcher::AArcher()
{
	SetAgroSphere(430.f);
	SetCombatSphere(400.f);


	//This is the default value, each enemy has different health.
	InitHealth(450.f);

	Damage = 40.f;
	EnemyExp = 40.f;

	AttackDelay = 0.6f;
}
void AArcher::BeginPlay()
{
	Super::BeginPlay();
}

void AArcher::Disappear()
{
	Super::SphereCollisionDisabled();

	Super::Destroy();
}