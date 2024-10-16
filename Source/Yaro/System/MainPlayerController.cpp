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
    
    UGameManager* GameManager = Cast<UGameManager>(GetWorld()->GetGameInstance());
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

void AMainPlayerController::SetPositions()
{
    for (AYaroCharacter* npc : Main->GetNPCList()) 
    {
        npc->GetAIController()->StopMovement();
        GetWorldTimerManager().ClearTimer(npc->GetMoveTimer());
        GetWorldTimerManager().ClearTimer(npc->GetTeamMoveTimer());
    }

    int dialogueNum = DialogueManager->GetDialogueNum();

    if (dialogueNum == 3) // after golem battle
    {
        Main->SetActorLocation(FVector(646.f, -1747.f, 2578.f));
        Main->SetActorRotation(FRotator(0.f, 57.f, 0.f)); // y(pitch), z(yaw), x(roll)

        Main->Momo->SetActorLocation(FVector(594.f, -1543.f, 2531.f));
        Main->Momo->SetActorRotation(FRotator(0.f, 280.f, 0.f));
         
        Main->Luko->SetActorLocation(FVector(494.f, -1629.f, 2561.f));
        Main->Luko->SetActorRotation(FRotator(0.f, 6.f, 0.f));
         
        Main->Vovo->SetActorLocation(FVector(903.f, -1767.f, 2574.f));
        Main->Vovo->SetActorRotation(FRotator(0.f, 165.f, 0.f));   
    
        Main->Vivi->SetActorLocation(FVector(790.f, -1636.f, 2566.f));
        Main->Vivi->SetActorRotation(FRotator(00.f, 180.f, 0.f));
        
        Main->Zizi->SetActorLocation(FVector(978.f, -1650.f, 2553.f));
        Main->Zizi->SetActorRotation(FRotator(0.f, 187.f, 0.f));   
    }

    if (dialogueNum == 11)
    {
        Main->SetActorLocation(FVector(-137.f, -2833.f, -117.f));
        Main->SetActorRotation(FRotator(0.f, 170.f, 0.f)); // y(pitch), z(yaw), x(roll)

        Main->Momo->SetActorLocation(FVector(-329.f, -2512.f, -122.f));
        Main->Momo->SetActorRotation(FRotator(0.f, 80.f, 0.f));

        Main->Luko->SetActorLocation(FVector(-242.f, -2692.f, -117.f));
        Main->Luko->SetActorRotation(FRotator(0.f, 85.f, 0.f));

        Main->Vovo->SetActorLocation(FVector(-313.f, -2755.f, -117.5f));
        Main->Vovo->SetActorRotation(FRotator(0.f, 83.f, 0.f));

        Main->Vivi->SetActorLocation(FVector(-343.f, -2849.f, -117.5f));
        Main->Vivi->SetActorRotation(FRotator(0.f, 77.f, 0.f));

        Main->Zizi->SetActorLocation(FVector(-323.f, -2949.f, -115.f));
        Main->Zizi->SetActorRotation(FRotator(0.f, 85.f, 0.f));
    }

    if (dialogueNum == 16)
    {
        Main->SetActorLocation(FVector(-35.f, 3549.f, 184.f));
        Main->SetActorRotation(FRotator(0.f, 265.f, 0.f)); // y(pitch), z(yaw), x(roll)

        Main->Momo->SetActorLocation(FVector(-86.f, 3263.f, 177.f));
        Main->Momo->SetActorRotation(FRotator(0.f, 272.f, 0.f));

        Main->Luko->SetActorLocation(FVector(184.f, 3317.f, 182.f));
        Main->Luko->SetActorRotation(FRotator(0.f, 260.f, 0.f));

        Main->Vovo->SetActorLocation(FVector(-140.f, 3370.f, 182.f));
        Main->Vovo->SetActorRotation(FRotator(0.f, 270.f, 0.f));

        Main->Vivi->SetActorLocation(FVector(105.f, 3176.f, 182.f));
        Main->Vivi->SetActorRotation(FRotator(0.f, 271.f, 0.f));

        Main->Zizi->SetActorLocation(FVector(68.f, 3398.f, 184.f));
        Main->Zizi->SetActorRotation(FRotator(0.f, 268.f, 0.f));

    }

    if (dialogueNum == 18)
    {
        Main->SetActorLocation(FVector(16.f, -2734.f, 582.f));
        Main->SetActorRotation(FRotator(0.f, 270.f, 0.f)); // y(pitch), z(yaw), x(roll)

        Main->Momo->SetActorLocation(FVector(-23.f, -3006.f, 603.f));
        Main->Momo->SetActorRotation(FRotator(0.f, 82.f, 0.f));

        Main->Luko->SetActorLocation(FVector(146.f, -2832.f, 582.f));
        Main->Luko->SetActorRotation(FRotator(0.f, 187.f, 0.f));

        Main->Vovo->SetActorLocation(FVector(-105.f, -2942.f, 581.f));
        Main->Vovo->SetActorRotation(FRotator(0.f, 10.f, 0.f));

        Main->Vivi->SetActorLocation(FVector(104.f, -2982.f, 581.f));
        Main->Vivi->SetActorRotation(FRotator(0.f, 120.f, 0.f));

        Main->Zizi->SetActorLocation(FVector(-125.f, -2804.f, 582.f));
        Main->Zizi->SetActorRotation(FRotator(0.f, 0.f, 0.f));

    }

    if (dialogueNum == 19)
    {
        Main->SetActorLocation(FVector(4658.f, 41.f, 148.f));
        Main->SetActorRotation(FRotator(-3.f, 180.f, 0.f)); // y(pitch), z(yaw), x(roll)

        Main->Momo->SetActorLocation(FVector(4247.f, 94.f, 98.f));
        Main->Momo->SetActorRotation(FRotator(0.f, 180.f, 0.f));

        Main->Luko->SetActorLocation(FVector(4345.f, -119.f, 108.f));
        Main->Luko->SetActorRotation(FRotator(0.f, 180.f, 0.f));

        Main->Vovo->SetActorLocation(FVector(4633.f, 121.f, 151.f));
        Main->Vovo->SetActorRotation(FRotator(0.f, 180.f, 0.f));

        Main->Vivi->SetActorLocation(FVector(4634.f, -105.f, 152.f));
        Main->Vivi->SetActorRotation(FRotator(0.f, 170.f, 0.f));

        Main->Zizi->SetActorLocation(FVector(4493.f, -26.f, 156.f));
        Main->Zizi->SetActorRotation(FRotator(0.f, 175.f, 0.f));
    }

    if (dialogueNum == 20)
    {
        Main->SetActorLocation(FVector(-4446.f, -20.f, 401.f));
        Main->SetActorRotation(FRotator(2.f, 180.f, 0.f)); // y(pitch), z(yaw), x(roll)

        Main->Momo->SetActorLocation(FVector(-4660.f, 118.f, 393.f));
        Main->Momo->SetActorRotation(FRotator(0.f, 296.f, 0.f));

        Main->Luko->SetActorLocation(FVector(-4545.f, -281.f, 401.f));
        Main->Luko->SetActorRotation(FRotator(0.f, 97.f, 0.f));

        Main->Vovo->SetActorLocation(FVector(-4429.f, 103.f, 396.f));
        Main->Vovo->SetActorRotation(FRotator(0.f, 219.f, 0.f));

        Main->Vivi->SetActorLocation(FVector(-4355.f, -195.f, 405.f));
        Main->Vivi->SetActorRotation(FRotator(0.f, 145.f, 0.f));

        Main->Zizi->SetActorLocation(FVector(-4695.f, -190.f, 394.f));
        Main->Zizi->SetActorRotation(FRotator(0.f, 49.f, 0.f));
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
