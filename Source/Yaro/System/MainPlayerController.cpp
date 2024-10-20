// Fill out your copyright notice in the Description page of Project Settings.


#include "MainPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Camera/CameraComponent.h"
#include "BrainComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Yaro/Character/Main.h"
#include "Yaro/Character/YaroCharacter.h"
#include "Yaro/System/GameManager.h"
#include "Yaro/System/DialogueManager.h"
#include "Yaro/System/UIManager.h"
#include "AIController.h"


void AMainPlayerController::BeginPlay()
{
    Super::BeginPlay();
    
    GameManager = Cast<UGameManager>(GetWorld()->GetGameInstance());
    if (GameManager)
    {
        DialogueManager = GameManager->GetDialogueManager();
        UIManager = GameManager->GetUIManager();
    }


    //카메라 회전 제한
    this->PlayerCameraManager->ViewPitchMin = -50.f; // 세로회전 위
    this->PlayerCameraManager->ViewPitchMax = 5.f; //아래

    
}


void AMainPlayerController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    DialogueManager->Tick();
    UIManager->Tick();

   if(Main == nullptr) Main = Cast<AMain>(UGameplayStatics::GetPlayerCharacter(this, 0));
}

int AMainPlayerController::WhichKeyDown()
{
    int result;
    if (WasInputKeyJustPressed(EKeys::One) || WasInputKeyJustPressed(EKeys::NumPadOne))
    {
        result = 1;
    }
    if (WasInputKeyJustPressed(EKeys::Two) || WasInputKeyJustPressed(EKeys::NumPadTwo))
    {
        result = 2;
    }
    if (WasInputKeyJustPressed(EKeys::Three) || WasInputKeyJustPressed(EKeys::NumPadThree))
    {
        result = 3;
    }
    if (WasInputKeyJustPressed(EKeys::Four) || WasInputKeyJustPressed(EKeys::NumPadFour))
    {
        result = 4;
    }
    if (WasInputKeyJustPressed(EKeys::Five) || WasInputKeyJustPressed(EKeys::NumPadFive))
    {
        result = 5;
    }

    return result;
}



void AMainPlayerController::FadeAndDialogue()
{
    if (WFadeInOut)
    {
        FadeInOut = CreateWidget<UUserWidget>(this, WFadeInOut);

        if (FadeInOut)
        {
            bFadeOn = true;

            if (bFallenPlayer && ((FallingCount == 0 || FallingCount >= 2)))
            {
                if(FallingCount != 6)
                    FallingCount += 1;
            }

            int dialogueNum = DialogueManager->GetDialogueNum();
            if (dialogueNum == 11 || dialogueNum == 18 || dialogueNum == 20) // second dungeon food trap
            {
                Main->SetCanMove(false);

                SetCinematicMode(true, true, true);

            }
            FadeOut();
        }
    }
    
}


