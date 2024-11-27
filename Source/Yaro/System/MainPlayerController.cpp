// Fill out your copyright notice in the Description page of Project Settings.


#include "MainPlayerController.h"
#include "Camera/CameraComponent.h"
#include "Yaro/System/GameManager.h"
#include "Yaro/System/DialogueManager.h"
#include "Yaro/System/UIManager.h"

void AMainPlayerController::BeginPlay()
{
    if (GetWorld()->GetName().Contains("Start"))
    {
        return;
    }

    Super::BeginPlay();

    GameManager = Cast<UGameManager>(GetWorld()->GetGameInstance());
    if (GameManager)
    {
        UIManager = GameManager->GetUIManager();
        if (UIManager)
        {
            UIManager->BeginPlay(GetWorld()->GetName());
        }

        DialogueManager = GameManager->GetDialogueManager();
        if (DialogueManager)
        {
            DialogueManager->BeginPlay();
        }

        NPCManager = GameManager->GetNPCManager();
        MainPlayer = GameManager->GetPlayer();
    }
    //카메라 회전 제한
    this->PlayerCameraManager->ViewPitchMin = -50.f; // 세로회전 위
    this->PlayerCameraManager->ViewPitchMax = 5.f; //아래
}

int32 AMainPlayerController::WhichKeyDown()
{
    static const TMap<FKey, int32> KeyMappings =
    {
        {EKeys::One, 1}, {EKeys::NumPadOne, 1},
        {EKeys::Two, 2}, {EKeys::NumPadTwo, 2},
        {EKeys::Three, 3}, {EKeys::NumPadThree, 3},
        {EKeys::Four, 4}, {EKeys::NumPadFour, 4},
        {EKeys::Five, 5}, {EKeys::NumPadFive, 5}
    };

    for (const auto& KeyMapping : KeyMappings)
    {
        if (WasInputKeyJustPressed(KeyMapping.Key))
        {
            return KeyMapping.Value;
        }
    }

    return 0;
}
