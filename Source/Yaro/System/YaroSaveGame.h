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
    int Level;

    UPROPERTY(VisibleAnywhere, Category = "SaveGameData")
    float Exp;

    UPROPERTY(VisibleAnywhere, Category = "SaveGameData")
    float MaxExp;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SaveGameData")
	FVector Location;

	UPROPERTY(VisibleAnywhere, Category = "SaveGameData")
	FRotator Rotation;

	UPROPERTY(VisibleAnywhere, Category = "SaveGameData")
	FString ItemName;

    UPROPERTY(VisibleAnywhere, Category = "SaveGameData")
	int FallCount;

	UPROPERTY(VisibleAnywhere, Category = "SaveGameData")
	int PotionNum;
};

USTRUCT(BlueprintType)
struct FNpcStats
{
    GENERATED_BODY()

    //UPROPERTY(VisibleAnywhere, Category = "SaveGameData")
    //float HP;


    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SaveGameData")
    FVector MomoLocation;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SaveGameData")
    FVector LukoLocation;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SaveGameData")
    FVector VovoLocation;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SaveGameData")
    FVector ViviLocation;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SaveGameData")
    FVector ZiziLocation;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SaveGameData")
	int TeamMoveIndex;
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

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Basic)
    int32 DialogueNum = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Basic)
	FCharacterStats CharacterStats;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Basic)
	TArray<FString> DeadEnemyList;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Basic)
	FNpcStats NpcInfo;
};
