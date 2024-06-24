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
	
	LoadGameInstance = Cast<UYaroSaveGame>(UGameplayStatics::LoadGameFromSlot("Defalut", 0));

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
	if (!LoadGameInstance)
	{
		LoadGameInstance = Cast<UYaroSaveGame>(UGameplayStatics::CreateSaveGameObject(UYaroSaveGame::StaticClass()));
		LoadGameInstance = Cast<UYaroSaveGame>(UGameplayStatics::LoadGameFromSlot("Defalut", 0));
	}

	for (int i = 0; i < NumberOfEnemies; i++)
	{
		Enemy = GetWorld()->SpawnActor<AEnemy>(EnemyType, SpawnTransform[i]);
		//Enemy->Name = EnemyName[i];

		if (!LoadGameInstance)
		{
			UE_LOG(LogTemp, Log, TEXT("nope"));

		}

		/*AMain* Main = Cast<AMain>(UGameplayStatics::GetPlayerCharacter(this, 0));
		
		for (int j = 0; j < Main->Enemies.Num(); j++)
		{
			if (Main->Enemies[i] == Enemy->Name)
			{
				Enemy->Destroy();
				UE_LOG(LogTemp, Log, TEXT("des"));

				break;
			}
		}*/
		/*for (auto& DeadEnemyName : LoadGameInstance->DeadEnemyList)
		{
			if (DeadEnemyName == Enemy->Name)
			{
				Enemy->Destroy();
				break;
			}
		}*/
	}
}

