// Fill out your copyright notice in the Description page of Project Settings.


#include "Spider.h"

ASpider::ASpider()
{
	EnemyType = EEnemyType::Spider;

	MaxHealth = 600.f;
	Damage = 45.f;
	EnemyExp = 45.f;
	AttackDelay = 0.4f;

	AgroSphereRadius = 450.f;
	CombatSphereRadius = 70.f;

	CreateSpheresAndCollisions();

}
