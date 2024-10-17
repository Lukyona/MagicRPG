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
#include "AIController.h"


void AMainPlayerController::BeginPlay()
{
    Super::BeginPlay();
    
    GameManager = Cast<UGameManager>(GetWorld()->GetGameInstance());
    if (GameManager)
    {
        DialogueManager = GameManager->GetDialogueManager();
    }


    //카메라 회전 제한
    this->PlayerCameraManager->ViewPitchMin = -50.f; // 세로회전 위
    this->PlayerCameraManager->ViewPitchMax = 5.f; //아래


    
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

    if (WEnemyHPBar)
    {
        EnemyHPBar = CreateWidget<UUserWidget>(this, WEnemyHPBar);
        if (EnemyHPBar)
        {
            EnemyHPBar->AddToViewport();
            EnemyHPBar->SetVisibility(ESlateVisibility::Hidden);
        }

        FVector2D Alignment(0.f, 0.f);
        EnemyHPBar->SetAlignmentInViewport(Alignment);
    }

    if (WSystemMessage)
    {
        SystemMessage = CreateWidget<UUserWidget>(this, WSystemMessage);
        if (SystemMessage)
        {
            SystemMessage->AddToViewport();
            SystemMessage->SetVisibility(ESlateVisibility::Hidden);
        }
    }
    
    if (WManual)
    {
        Manual = CreateWidget<UUserWidget>(this, WManual);
        if (Manual)
        {
            Manual->AddToViewport();
            Manual->SetVisibility(ESlateVisibility::Hidden);
        }
    }

    if (WMenu)
    {
        Menu = CreateWidget<UUserWidget>(this, WMenu);
        if (Menu)
        {
            Menu->AddToViewport();
            Menu->SetVisibility(ESlateVisibility::Hidden);
        }
    }

    bCalculateOn = true;


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

    DialogueManager->Tick();

    if (TargetArrow)
    {
        FVector2D PositionInViewport;
        ProjectWorldLocationToScreen(EnemyLocation, PositionInViewport);

        PositionInViewport.Y -= 140.f;
        PositionInViewport.X -= 130.f;
        EnemyHPBar->SetPositionInViewport(PositionInViewport);

        PositionInViewport.Y -= 120.f;
        PositionInViewport.X += 60.f;


        TargetArrow->SetPositionInViewport(PositionInViewport);

        FVector2D SizeInViewport = FVector2D(150.f, 15.f);
        EnemyHPBar->SetDesiredSizeInViewport(SizeInViewport);
    }



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

void AMainPlayerController::DisplayEnemyHPBar()
{
    if (EnemyHPBar)
    {
        bEnemyHPBarVisible = true;
        EnemyHPBar->SetVisibility(ESlateVisibility::Visible);
    }
}

void AMainPlayerController::RemoveEnemyHPBar()
{
    if (EnemyHPBar)
    {
        bEnemyHPBarVisible = false;
        EnemyHPBar->SetVisibility(ESlateVisibility::Hidden);
       // bCanDisplaySpeechBubble = false;
    }
}

void AMainPlayerController::DisplayMenu()
{
    if (Menu)
    {
        if (bManualVisible) RemoveManual();

        bMenuVisible = true;

        if (DialogueManager->GetDialogueNum() < 3 || DialogueManager->IsDialogueUIVisible()) DisplaySystemMessage();

        Menu->SetVisibility(ESlateVisibility::Visible);

        FInputModeGameAndUI InputMode;
        SetInputMode(InputMode);
        bShowMouseCursor = true;


    }
}

void AMainPlayerController::RemoveMenu()
{
    if (Menu)
    {      
        bMenuVisible = false;
        if (SystemMessageOn) SetSystemMessage();    
        else if (bSystemMessageVisible) RemoveSystemMessage();


        Menu->SetVisibility(ESlateVisibility::Hidden);

        if (!DialogueManager->IsDialogueUIVisible())
        {
            FInputModeGameOnly InputModeGameOnly;
            SetInputMode(InputModeGameOnly);

            bShowMouseCursor = false;
        }
    }
}

void AMainPlayerController::ToggleMenu()
{
    if (bMenuVisible)
    {
        RemoveMenu();
    }
    else
    {
        DisplayMenu();
    }
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

void AMainPlayerController::DisplaySystemMessage()
{
    if (SystemMessage)
    {
        if (bMenuVisible && DialogueManager->GetDialogueNum() < 3)
        {
            text = FString(TEXT("이 곳에선 저장되지 않습니다."));

        }
        if (bMenuVisible && DialogueManager->IsDialogueUIVisible())
        {
            text = FString(TEXT("대화 중엔 저장되지 않습니다."));
        }

        SystemText = FText::FromString(text);

        bSystemMessageVisible = true;

        SystemMessage->SetVisibility(ESlateVisibility::Visible);
        //UE_LOG(LogTemp, Log, TEXT("DisplaySystemMessage"));

    }
}

void AMainPlayerController::RemoveSystemMessage()
{
    if (SystemMessage)
    {
        SystemMessage->SetVisibility(ESlateVisibility::Hidden);
        SystemMessageOn = false;
        bSystemMessageVisible = false;

    }
}


void AMainPlayerController::SetSystemMessage()
{
    switch (SystemMessageNum)
    {
        case 1:
            text = FString(TEXT("F키를 누르거나 마우스 왼쪽 버튼을 클릭하여\n대화를 진행할 수 있습니다."));
            break;
        case 2:
            text = FString(TEXT("M키를 눌러 조작 매뉴얼을 확인하고\n다시 M키를 눌러 조작 매뉴얼을 닫으세요."));
            break;
        case 3:
            text = FString(TEXT("이제 지팡이 가까이에서 마우스 왼쪽 버튼을 클릭하여\n지팡이를 장비하세요."));
            break;
        case 4:
            text = FString(TEXT("포탈을 이용해 던전으로 이동하세요."));
            break;
        case 5:
            text = FString(TEXT("숫자키 1로 한 가지 마법을 쓸 수 있습니다.\n준비가 되었다면 G키를 누르세요."));
            break;
        case 6:
            text = FString(TEXT("레벨 2가 되었습니다!\n이제 숫자키 2로 두 번째 마법을 쓸 수 있습니다."));
            break;
        case 7:
            text = FString(TEXT("레벨 3이 되었습니다!\n이제 숫자키 3으로 세 번째 마법을 쓸 수 있습니다."));
            break;
        case 8:
            text = FString(TEXT("축하합니다!\n이제 숫자키 4로 네 번째 마법을 쓸 수 있습니다."));
            break;
        case 9:
            text = FString(TEXT("레벨 5를 달성했습니다!\n이제 숫자키 5로 다섯 번째 마법을 쓸 수 있습니다."));
            break;
        case 10:
            text = FString(TEXT("마우스 왼쪽 버튼을 클릭하여 돌을 줍고\n움직이는 돌 가까이서 마우스 왼쪽 버튼을 클릭하여\n돌을 놓으세요."));
            break;
        case 11:
            text = FString(TEXT("계단을 통해 다음 장소로 이동하세요."));
            break;
        case 12:
            text = FString(TEXT("지금부터 전투가 끝나는 시점까지\n저장이 되지 않습니다."));
            break;
        case 13:
            text = FString(TEXT("포탈을 이용해 던전 입구로 돌아가세요."));
            break;
        case 14:
            text = FString(TEXT("마우스 왼쪽 버튼을 클릭하여\n디비눔 프레시디움을 챙기세요."));
            break;
        case 15:
            text = FString(TEXT("레벨 5가 되었습니다!\n숫자키 1~5로 모든 마법을 쓸 수 있습니다."));
            break;
        case 16:
            text = FString(TEXT("전체 회복 포션을 받았습니다!\n필요할 때 P키를 눌러 사용하세요."));
            break;
    }
    SystemMessageOn = true;
    //UE_LOG(LogTemp, Log, TEXT("SetSystemMessage"));

    DisplaySystemMessage();

}

void AMainPlayerController::DisplayManual()
{
    if (Manual)
    {
        if (bMenuVisible || DialogueManager->IsDialogueUIVisible())
        {
            DisplaySystemMessage();
            text = FString(TEXT("대화 중이거나 메뉴가 활성화된 상태에서는\n조작 매뉴얼을 볼 수 없습니다."));
            SystemText = FText::FromString(text);
            GetWorld()->GetTimerManager().SetTimer(Timer, FTimerDelegate::CreateLambda([&]() {
                
                if (SystemMessageOn) SetSystemMessage();
                else if (bSystemMessageVisible) RemoveSystemMessage();
                
            }), 2.f, false);
        }
        else
        {
            bManualVisible = true;
            Manual->SetVisibility(ESlateVisibility::Visible);
        }
    }
}

void AMainPlayerController::RemoveManual()
{
    if (Manual)
    {
        bManualVisible = false;
        Manual->SetVisibility(ESlateVisibility::Hidden);
        if (DialogueManager->GetDialogueNum() == 2 && SystemMessageNum != 4)
        {
            Main->SetCanMove(true);
            SystemMessageNum = 3;
            DialogueManager->DialogueEvents();
        }
    }
}
