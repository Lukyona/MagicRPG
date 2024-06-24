// Fill out your copyright notice in the Description page of Project Settings.


#include "Grux.h"

AGrux::AGrux()
{
	SetAgroSphere(600.f);
	SetCombatSphere(200.f);

	CreateFirstWeaponCollision();
	CreateSecondWeaponCollision();

	//This is the default value, each enemy has different health.
	InitHealth(300.f);

	Damage = 40.f;
	EnemyExp = 70.f;

	AttackDelay = 0.2f;


}

void AGrux::BeginPlay()
{
	Super::BeginPlay();

	EnableFirstWeaponCollision();
	EnableSecondWeaponCollision();
}

