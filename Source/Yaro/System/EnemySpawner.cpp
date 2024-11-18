// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemySpawner.h"
#include "Yaro/Character/Enemies/Enemy.h"
#include "YaroSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Yaro/Character/Main.h"

// Sets default values
AEnemySpawner::AEnemySpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AEnemySpawner::BeginPlay()
{
	Super::BeginPlay();
	
	if (!bSpawnLater)
	{
		SpawnEnemies();
	}
}

// Called every frame
void AEnemySpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AEnemySpawner::SpawnEnemies()
{
	int32 count = 0;
	if (UGameplayStatics::DoesSaveGameExist("Default", 0)) // 저장 정보 존재하면
	{
		UYaroSaveGame* LoadGameInstance = Cast<UYaroSaveGame>(UGameplayStatics::LoadGameFromSlot("Default", 0));
		int32* deadCount = LoadGameInstance->DeadEnemyList.Find(EnemyType);
		if (deadCount != nullptr)
		{
			count = NumberOfEnemies - *deadCount;
			if (count <= 0) return;
		}
		else
			count = NumberOfEnemies;
	}
	else
	{
		count = NumberOfEnemies;
	}

	for (int i = 0; i < count; i++)
	{
		SpawnedEnemy = GetWorld()->SpawnActor<AEnemy>(EnemyClass, SpawnTransform[i]);
	}
}

