// Fill out your copyright notice in the Description page of Project Settings.


#include "Spider.h"

ASpider::ASpider()
{
	EnemyType = EEnemyType::Spider;

	SetAgroSphere(450.f);
	SetCombatSphere(70.f);

	CreateFirstWeaponCollision();

	//This is the default value, each enemy has different health.
	InitHealth(600.f);

	Damage = 45.f;
	EnemyExp = 45.f;

	AttackDelay = 0.4f;

}

void ASpider::BeginPlay()
{
	Super::BeginPlay();

	EnableFirstWeaponCollision();
}
