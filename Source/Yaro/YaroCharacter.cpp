// Copyright Epic Games, Inc. All Rights Reserved.

#include "YaroCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"

//////////////////////////////////////////////////////////////////////////
// AYaroCharacter

AYaroCharacter::AYaroCharacter()
{

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;



	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

//////////////////////////////////////////////////////////////////////////
// Input

//void AYaroCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
//{
//	// Set up gameplay key bindings
//	//check(PlayerInputComponent);
//	//PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
//	//PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
//
//	//PlayerInputComponent->BindAxis("MoveForward", this, &AYaroCharacter::MoveForward);
//	//PlayerInputComponent->BindAxis("MoveRight", this, &AYaroCharacter::MoveRight);
//
//
//	//// handle touch devices
//	//PlayerInputComponent->BindTouch(IE_Pressed, this, &AYaroCharacter::TouchStarted);
//	//PlayerInputComponent->BindTouch(IE_Released, this, &AYaroCharacter::TouchStopped);
//
//	//// VR headset functionality
//	//PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AYaroCharacter::OnResetVR);
//}


//void AYaroCharacter::OnResetVR()
//{
//	// If Yaro is added to a project via 'Add Feature' in the Unreal Editor the dependency on HeadMountedDisplay in Yaro.Build.cs is not automatically propagated
//	// and a linker error will result.
//	// You will need to either:
//	//		Add "HeadMountedDisplay" to [YourProject].Build.cs PublicDependencyModuleNames in order to build successfully (appropriate if supporting VR).
//	// or:
//	//		Comment or delete the call to ResetOrientationAndPosition below (appropriate if not supporting VR)
//	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
//}
//
//void AYaroCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
//{
//		Jump();
//}
//
//void AYaroCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
//{
//		StopJumping();
//}
//
//void AYaroCharacter::MoveForward(float Value)
//{
//	if ((Controller != nullptr) && (Value != 0.0f))
//	{
//		// find out which way is forward
//		const FRotator Rotation = Controller->GetControlRotation();
//		const FRotator YawRotation(0, Rotation.Yaw, 0);
//
//		// get forward vector
//		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
//		AddMovementInput(Direction, Value);
//	}
//}
//
//void AYaroCharacter::MoveRight(float Value)
//{
//	if ( (Controller != nullptr) && (Value != 0.0f) )
//	{
//		// find out which way is right
//		const FRotator Rotation = Controller->GetControlRotation();
//		const FRotator YawRotation(0, Rotation.Yaw, 0);
//	
//		// get right vector 
//		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
//		// add movement in that direction
//		AddMovementInput(Direction, Value);
//	}
//}
