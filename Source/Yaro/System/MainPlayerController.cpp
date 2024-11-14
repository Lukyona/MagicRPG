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
    if (GetWorld()->GetName().Contains("Start")) return;

    Super::BeginPlay();

    GameManager = Cast<UGameManager>(GetWorld()->GetGameInstance());
    if (GameManager)
    {
        UIManager = GameManager->GetUIManager();
        if (UIManager) UIManager->BeginPlay();

        DialogueManager = GameManager->GetDialogueManager();
        if (DialogueManager) DialogueManager->BeginPlay();

        NPCManager = GameManager->GetNPCManager();
        MainPlayer = GameManager->GetPlayer();
    }

    //카메라 회전 제한
    this->PlayerCameraManager->ViewPitchMin = -50.f; // 세로회전 위
    this->PlayerCameraManager->ViewPitchMax = 5.f; //아래
}


void AMainPlayerController::Tick(float DeltaTime)
{
    if (GetWorld()->GetName().Contains("Start")) return;

    Super::Tick(DeltaTime);

    DialogueManager->Tick();
    UIManager->Tick();
}

int AMainPlayerController::WhichKeyDown()
{
    int result = 0;
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


