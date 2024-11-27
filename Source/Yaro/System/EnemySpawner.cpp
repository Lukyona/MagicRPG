// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemySpawner.h"
#include "Kismet/GameplayStatics.h"
#include "YaroSaveGame.h"
#include "Yaro/Character/Enemies/Enemy.h"
#include "Yaro/Character/Main.h"

AEnemySpawner::AEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AEnemySpawner::BeginPlay()
{
	Super::BeginPlay();
	
	if (!bSpawnLater)
	{
		SpawnEnemies();
	}
}

void AEnemySpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AEnemySpawner::SpawnEnemies()
{
	int32 AliveCount = 0;
	if (UGameplayStatics::DoesSaveGameExist("Default", 0)) // 저장 정보 존재하면
	{
		UYaroSaveGame* LoadGameInstance = Cast<UYaroSaveGame>(UGameplayStatics::LoadGameFromSlot("Default", 0));
		int32* DeadCount = LoadGameInstance->DeadEnemyList.Find(EnemyType);
		if (DeadCount != nullptr)
		{
			AliveCount = NumberOfEnemies - *DeadCount;
			if (AliveCount <= 0) return;
		}
		else
			AliveCount = NumberOfEnemies;
	}
	else
	{
		AliveCount = NumberOfEnemies;
	}

	for (int i = 0; i < AliveCount; i++)
	{
		SpawnedEnemy = GetWorld()->SpawnActor<AEnemy>(EnemyClass, SpawnTransform[i]);
	}
}
