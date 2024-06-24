// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Yaro/Character/Enemies/Enemy.h"
#include "Spider.generated.h"

/**
 * 
 */
UCLASS()
class YARO_API ASpider : public AEnemy
{
	GENERATED_BODY()
	
public:
	ASpider();

	virtual void BeginPlay() override;

};
