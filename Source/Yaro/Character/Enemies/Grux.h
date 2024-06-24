// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Yaro/Character/Enemies/Enemy.h"
#include "Grux.generated.h"

/**
 * 
 */
UCLASS()
class YARO_API AGrux : public AEnemy
{
	GENERATED_BODY()
	
public:
	AGrux();

	virtual void BeginPlay() override;

};
