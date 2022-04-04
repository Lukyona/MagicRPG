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


protected:

	///** Resets HMD orientation in VR. */
	//void OnResetVR();

	///** Called for forwards/backward input */
	//void MoveForward(float Value);

	///** Called for side to side input */
	//void MoveRight(float Value);


	///** Handler for when a touch input begins. */
	//void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	///** Handler for when a touch input stops. */
	//void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

protected:
	// APawn interface
	//virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

};

