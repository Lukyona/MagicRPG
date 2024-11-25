// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Yaro/Character/Enemies/Enemy.h"
#include "Boss.generated.h"

/**
 * 
 */
UCLASS()
class YARO_API ABoss : public AEnemy
{
	GENERATED_BODY()

public:
	ABoss();

	void Disappear();
};
