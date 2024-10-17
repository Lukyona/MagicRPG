// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "UIManager.generated.h"

/**
 * 
 */
UCLASS()
class YARO_API UUIManager : public UObject
{
	GENERATED_BODY()

	UPROPERTY()
	class UGameManager* GameManager;
	

public:
	void Init();
};
