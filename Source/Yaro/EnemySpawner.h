// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "YaroSaveGame.h"
#include "EnemySpawner.generated.h"

UCLASS()
class YARO_API AEnemySpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEnemySpawner();

	class AEnemy* Enemy;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class AEnemy> EnemyType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int NumberOfEnemies;

	FTransform tmp; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FTransform> SpawnTransform;

	FString tmp2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> EnemyName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSpawnLater = false;

	UYaroSaveGame* LoadGameInstance;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void SpawnEnemies();
};
