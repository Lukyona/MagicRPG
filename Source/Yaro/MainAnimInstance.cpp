// Fill out your copyright notice in the Description page of Project Settings.


#include "MainAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"

void UMainAnimInstance::NativeInitializeAnimation()
{
    if (Pawn == nullptr)
    {
        Pawn = TryGetPawnOwner();
    }

}

void UMainAnimInstance::UpdateAnimationProperties()
{
    if (Pawn == nullptr)
    {
        Pawn = TryGetPawnOwner();
    }

    if (Pawn)
    {
        FVector Speed = Pawn->GetVelocity();
        FVector LateraSpeed = FVector(Speed.X, Speed.Y, 0.f); //Lateral means ¿·¸é, È¾¹æÇâ
        MovementSpeed = LateraSpeed.Size(); // º¤ÅÍ Å©±â·Î float ¹ÝÈ¯

        bIsInAir = Pawn->GetMovementComponent()->IsFalling();
    }
}