// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Yaro/Character/Enemies/Enemy.h"
#include "Golem.generated.h"

/**
 * 
 */
UCLASS()
class YARO_API AGolem : public AEnemy
{
	GENERATED_BODY()

public:
	AGolem();

	UFUNCTION(BlueprintCallable)
	void HitGround(); 
	
};
