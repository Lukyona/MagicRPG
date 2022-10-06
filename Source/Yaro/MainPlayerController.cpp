// Fill out your copyright notice in the Description page of Project Settings.


#include "MainPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "BrainComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "DialogueUI.h"
#include "Main.h"
#include "YaroCharacter.h"
#include "AIController.h"

AMainPlayerController::AMainPlayerController()
{
    static ConstructorHelpers::FClassFinder<UUserWidget> DialogueBPClass(TEXT("/Game/HUDandWigets/DialogueUI_BP.DialogueUI_BP_C"));

    if (DialogueBPClass.Class != nullptr) DialogueUIClass = DialogueBPClass.Class;
}

void AMainPlayerController::BeginPlay()
{
    Super::BeginPlay();
    
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

    if (DialogueUIClass != nullptr)
    {
        DialogueUI = CreateWidget<UDialogueUI>(this, DialogueUIClass);
    }

    if (DialogueUI != nullptr)
    {
        DialogueUI->AddToViewport();
        DialogueUI->SetVisibility(ESlateVisibility::Hidden);
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
    }
}

void AMainPlayerController::DisplayMenu()
{
    if (Menu)
    {
        if (bManualVisible) RemoveManual();

        bMenuVisible = true;

        if (DialogueNum < 3 || bDialogueUIVisible) DisplaySystemMessage();

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

        if (!bDialogueUIVisible)
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

void AMainPlayerController::DisplayDialogueUI()
{
    if (DialogueUI)
    {
        //UE_LOG(LogTemp, Log, TEXT("DisplayDialogueUI"));

        if (!DialogueUI->bCanStartDialogue) return;

        if (bManualVisible) RemoveManual();

        bDialogueUIVisible = true;

        switch (DialogueNum)
        {
            case 0:
                DialogueUI->InitializeDialogue(IntroDialogue);
                break;
            case 1: // onlu luko dialogue
                DialogueUI->OnAnimationShowMessageUI();
                GetWorld()->GetTimerManager().SetTimer(DialogueUI->TimerHandle, DialogueUI, &UDialogueUI::StartAnimatedMessage, 0.1f, false);
                break;
            case 2:
                DialogueUI->InitializeDialogue(DungeonDialogue1);
                break;
            case 3:   
                if (!bFadeOn)
                {
                    FadeAndDialogue();
                    return;
                }              
                DialogueUI->InitializeDialogue(DungeonDialogue2);
                bFadeOn = false;
                break;
            case 4: // the boat move
                DialogueUI->OnAnimationShowMessageUI();
                GetWorld()->GetTimerManager().SetTimer(DialogueUI->TimerHandle, DialogueUI, &UDialogueUI::DialogueEvents, 0.1f, false);
                break;
        }

        DialogueUI->SetVisibility(ESlateVisibility::Visible);

        FInputModeGameAndUI InputMode;  
        SetInputMode(InputMode);
        bShowMouseCursor = true;
    }
}

void AMainPlayerController::RemoveDialogueUI()
{
    if (DialogueUI)
    {
        DialogueNum++;
       
        bDialogueUIVisible = false;

        bShowMouseCursor = false;

        DialogueEvents();

        FInputModeGameOnly InputModeGameOnly;
        SetInputMode(InputModeGameOnly);

        DialogueUI->OnAnimationHideMessageUI();

    }
}


void AMainPlayerController::DialogueEvents()
{
    switch (DialogueNum)
    {
        case 1: // luko moves to player    
            Main->Luko->MoveToPlayer();         
            break;
        case 2:
            if (SystemMessageNum != 3)
            {
                GetWorldTimerManager().ClearTimer(Main->Luko->MoveTimer);
                Main->Luko->AIController->MoveToLocation(FVector(5200.f, 35.f, 100.f));
                SystemMessageNum = 2;
                SetSystemMessage();
                return;
            }
            SetCinematicMode(false, true, true);   
            if (Main->EquippedWeapon == nullptr)
            {
                SetSystemMessage();
            }
            break;
        case 3:
            Main->SaveGame();
            SetCinematicMode(false, true, true);
            break;
        case 4:
            //DisplaySystemMessage();

            SetCinematicMode(false, true, true);
            break;
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

            SetCinematicMode(true, true, true);
            SetControlRotation(FRotator(0.f, 57.f, 0.f));

            FadeOut();
            FadeInOut->AddToViewport();                     
        }
    }
}

void AMainPlayerController::SetPositions()
{
    if (DialogueNum == 3)
    {
        Main->SetActorLocation(FVector(646.f, -1747.f, 2578.f));
        Main->SetActorRotation(FRotator(0.f, 57.f, 0.f)); // y(pitch), z(yaw), x(roll)

        for (AYaroCharacter* npc : Main->NPCList)
        {
            npc->AIController->StopMovement();
            GetWorldTimerManager().ClearTimer(npc->MoveTimer);
            GetWorldTimerManager().ClearTimer(npc->TeamMoveTimer);     
        }

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
}

void AMainPlayerController::DisplaySystemMessage()
{
    if (SystemMessage)
    {
        if (bMenuVisible && DialogueNum < 3)
        {
            text = FString(TEXT("이 곳에선 저장되지 않습니다."));

        }
        if (bMenuVisible && bDialogueUIVisible)
        {
            text = FString(TEXT("대화 중엔 저장되지 않습니다."));
        }

        SystemText = FText::FromString(text);

        bSystemMessageVisible = true;

        SystemMessage->SetVisibility(ESlateVisibility::Visible);
        UE_LOG(LogTemp, Log, TEXT("DisplaySystemMessage"));

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
    }
    SystemMessageOn = true;
    UE_LOG(LogTemp, Log, TEXT("SetSystemMessage"));

    DisplaySystemMessage();

}

void AMainPlayerController::DisplayManual()
{
    if (Manual)
    {
        if (bMenuVisible || bDialogueUIVisible)
        {
            DisplaySystemMessage();
            text = FString(TEXT("대화 중이거나 메뉴가 활성화된 상태에서는\n조작 매뉴얼을 볼 수 없습니다."));
            SystemText = FText::FromString(text);
            FTimerHandle Timer;
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
        if (DialogueNum == 2)
        {
            SystemMessageNum = 3;
            DialogueEvents();
        }
    }
}
