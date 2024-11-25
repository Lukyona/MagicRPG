// Fill out your copyright notice in the Description page of Project Settings.


#include "LittleMonster.h"

ALittleMonster::ALittleMonster()
{
	EnemyType = EEnemyType::LittleMonster;
	
	hasSecondCollision = true;

	MaxHealth = 700.f;
	Damage = 45.f;
	EnemyExp = 60.f;
	AttackDelay = 0.5f;

	AgroSphereRadius = 200.f;
	CombatSphereRadius = 75.f;
}
