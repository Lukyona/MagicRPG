// Fill out your copyright notice in the Description page of Project Settings.


#include "MainPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

void AMainPlayerController::BeginPlay()
{
    Super::BeginPlay();
    
    //카메라 회전 제한
    this->PlayerCameraManager->ViewPitchMin = -50.f; // 세로회전 위
    this->PlayerCameraManager->ViewPitchMax = 10.f; //아래

    
    if (WTargetArrow)
    {
        TargetArrow = CreateWidget<UUserWidget>(this, WTargetArrow);
        if (TargetArrow)
        {
            TargetArrow->AddToViewport();
            TargetArrow->SetVisibility(ESlateVisibility::Hidden);
        }

        FVector2D Alignment(0.f, 0.f);
        TargetArrow->SetAlignmentInViewport(Alignment);
    }
}

void AMainPlayerController::DisplayTargetArrow()
{
    if (TargetArrow)
    {
        bTargetArrowVisible = true;
        TargetArrow->SetVisibility(ESlateVisibility::Visible);
    }
}

void AMainPlayerController::RemoveTargetArrow()
{
    if (TargetArrow)
    {
        bTargetArrowVisible = false;
        TargetArrow->SetVisibility(ESlateVisibility::Hidden);
    }
}

void AMainPlayerController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (TargetArrow)
    {
        FVector2D PositionInViewport;
        ProjectWorldLocationToScreen(EnemyLocation, PositionInViewport);
        PositionInViewport.Y -= 250.f;
        PositionInViewport.X -= 50.f;

        //FVector2D SizeInViewport = FVector2D(10.f, 10.f);

        TargetArrow->SetPositionInViewport(PositionInViewport);
        //TargetArrow->SetDesiredSizeInViewport(SizeInViewport);
    }
}

int AMainPlayerController::WhichKeyDown()
{
    int result;
    if (this->WasInputKeyJustPressed(EKeys::One) || this->WasInputKeyJustPressed(EKeys::NumPadOne))
    {
        result = 1;
    }
    if (this->WasInputKeyJustPressed(EKeys::Two) || this->WasInputKeyJustPressed(EKeys::NumPadTwo))
    {
        result = 2;
    }
    if (this->WasInputKeyJustPressed(EKeys::Three) || this->WasInputKeyJustPressed(EKeys::NumPadThree))
    {
        result = 3;
    }

    return result;
}
