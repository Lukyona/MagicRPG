// Fill out your copyright notice in the Description page of Project Settings.
#include "Goblin.h"

AGoblin::AGoblin()
{
	EnemyType = EEnemyType::Goblin;

	MaxHealth = 180.f;
	Damage = 20.f;
	EnemyExp = 30.f;
	AttackDelay = 0.2f;

	AgroSphereRadius = 500.f;
	CombatSphereRadius = 110.f;

	CreateSpheresAndCollisions();

}
