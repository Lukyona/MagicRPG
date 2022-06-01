// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MainPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class YARO_API AMainPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:

	//Player can Targeting, then TargetArrow appear on Targeted enemy
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<UUserWidget> WTargetArrow;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Widgets")
	UUserWidget* TargetArrow;


	bool bTargetArrowVisible;

	void DisplayTargetArrow();
	void RemoveTargetArrow();

	FVector EnemyLocation;
protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;
};
