// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "YaroCharacter.generated.h"


UCLASS(config=Game)
class AYaroCharacter : public ACharacter
{
	GENERATED_BODY()


public:
	AYaroCharacter();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	class AAIController* AIController;

	//virtual void Tick(float DeltaTime) override;

	void MoveToPlayer();

protected:

	virtual void BeginPlay() override;

	

protected:
	// APawn interface
	//virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

};

