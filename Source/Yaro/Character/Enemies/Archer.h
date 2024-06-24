// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Yaro/Character/Enemies/Enemy.h"
#include "Archer.generated.h"

/**
 * 
 */
UCLASS()
class YARO_API AArcher : public AEnemy
{
	GENERATED_BODY()
	
public:
	AArcher();

	virtual void BeginPlay() override;

	void Disappear();
};
