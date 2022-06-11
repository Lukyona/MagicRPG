// Fill out your copyright notice in the Description page of Project Settings.


#include "YaroAnimInstance.h"
#include "YaroCharacter.h"

void UYaroAnimInstance::NativeInitializeAnimation()
{
    if (Pawn == nullptr)
    {
        Pawn = TryGetPawnOwner();
        if (Pawn)
        {
            YaroCharacter = Cast<AYaroCharacter>(Pawn);
        }
    }
}

void UYaroAnimInstance::UpdateAnimationProperties()
{
    if (Pawn == nullptr)
    {
        Pawn = TryGetPawnOwner();
        if (Pawn)
        {
            YaroCharacter = Cast<AYaroCharacter>(Pawn);
        }
    }

    if (Pawn)
    {
        FVector Speed = Pawn->GetVelocity();
        FVector LateraSpeed = FVector(Speed.X, Speed.Y, 0.f); //Lateral means ø∑∏È, »æπÊ«‚
        MovementSpeed = LateraSpeed.Size(); // ∫§≈Õ ≈©±‚∑Œ float π›»Ø
    }
}