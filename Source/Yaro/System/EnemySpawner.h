// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "YaroSaveGame.h"
#include "EnemySpawner.generated.h"

class AEnemy;

UCLASS()
class YARO_API AEnemySpawner : public AActor
{
	GENERATED_BODY()

public:
	AEnemySpawner();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

protected:
	AEnemy* SpawnedEnemy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AEnemy> EnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EEnemyType EnemyType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NumberOfEnemies;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FTransform> SpawnTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSpawnLater = false;

public:
	UFUNCTION(BlueprintCallable)
	void SpawnEnemies();
};
