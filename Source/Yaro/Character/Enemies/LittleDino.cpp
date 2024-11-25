// Fill out your copyright notice in the Description page of Project Settings.


#include "LittleDino.h"

ALittleDino::ALittleDino()
{
	EnemyType = EEnemyType::LittleDino;

	hasSecondCollision = true;

	MaxHealth = 200.f;
	Damage = 30.f;
	EnemyExp = 30.f;
	AttackDelay = 0.2f;

	AgroSphereRadius = 200.f;
	CombatSphereRadius = 30.f;
}
