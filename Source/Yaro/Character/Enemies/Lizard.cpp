// Fill out your copyright notice in the Description page of Project Settings.


#include "Lizard.h"

ALizard::ALizard()
{
	EnemyType = EEnemyType::Lizard;

	bHasSecondCollision = true;
	
	MaxHealth = 400.f;
	Damage = 45.f;
	EnemyExp = 45.f;
	AttackDelay = 0.3f;

	AgroSphereRadius = 450.f;
	CombatSphereRadius = 110.f;

	CreateSpheresAndCollisions();

}
