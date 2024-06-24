// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Yaro/Character/Enemies/Enemy.h"
#include "LizardShaman.generated.h"

/**
 * 
 */
UCLASS()
class YARO_API ALizardShaman : public AEnemy
{
	GENERATED_BODY()
	
public:
	ALizardShaman();

	virtual void BeginPlay() override;


	void Disappear();
};
