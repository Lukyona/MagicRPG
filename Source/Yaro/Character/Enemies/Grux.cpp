// Fill out your copyright notice in the Description page of Project Settings.


#include "Grux.h"

AGrux::AGrux()
{
	EnemyType = EEnemyType::Grux;

	bHasSecondCollision = true;
	
	MaxHealth = 300.f;
	Damage = 40.f;
	EnemyExp = 70.f;
	AttackDelay = 0.2f;

	AgroSphereRadius = 600.f;
	CombatSphereRadius = 200.f;

	CreateSpheresAndCollisions();

}

