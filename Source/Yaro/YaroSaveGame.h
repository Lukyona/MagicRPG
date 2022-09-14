// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "YaroSaveGame.generated.h"

USTRUCT(BlueprintType)
struct FCharacterStats
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category = "SaveGameData")
	float HP;
	UPROPERTY(VisibleAnywhere, Category = "SaveGameData")
	float MaxHP;

	UPROPERTY(VisibleAnywhere, Category = "SaveGameData")
	float MP;
	UPROPERTY(VisibleAnywhere, Category = "SaveGameData")
	float MaxMP;

	UPROPERTY(VisibleAnywhere, Category = "SaveGameData")
	float SP;
	UPROPERTY(VisibleAnywhere, Category = "SaveGameData")
	float MaxSP;

	UPROPERTY(VisibleAnywhere, Category = "SaveGameData")
	FVector Location;

	UPROPERTY(VisibleAnywhere, Category = "SaveGameData")
	FRotator Rotation;

	UPROPERTY(VisibleAnywhere, Category = "SaveGameData")
	FString WeaponName;
};

USTRUCT(BlueprintType)
struct FEnemyInfo
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category = "SaveGameData")
	FVector Location;

	UPROPERTY(VisibleAnywhere, Category = "SaveGameData")
	FRotator Rotation;

	/*UPROPERTY(VisibleAnywhere, Category = "SaveGameData")
	FString EnemyName;*/

	UPROPERTY(VisibleAnywhere, Category = "SaveGameData")
	int EnemyIndex;
};

/**
 * 
 */
UCLASS()
class YARO_API UYaroSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UYaroSaveGame();

	UPROPERTY(VisibleAnywhere, Category = Basic)
	FString SaveName;

	UPROPERTY(VisibleAnywhere, Category = Basic)
	uint32 UserIndex;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Basic)
	int32 PlayerGender = 0;

	UPROPERTY(VisibleAnywhere, Category = Basic)
	FCharacterStats CharacterStats;

	UPROPERTY(VisibleAnywhere, Category = Basic)
	TArray<FEnemyInfo> EnemyInfoArray;

	UPROPERTY(EditAnywhere)
	FEnemyInfo EnemyInfo;
};
