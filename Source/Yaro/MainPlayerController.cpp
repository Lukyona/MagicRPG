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
#include "Kismet/KismetMathLibrary.h"
#include "Camera/CameraComponent.h"


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

    bCalculateOn = true;

    SpeechBubble = GetWorld()->SpawnActor<AActor>(SpeechBubble_BP);

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

void AMainPlayerController::DisplaySpeechBuubble(class AYaroCharacter* npc)
{
    if (SpeechBubble && bCanDisplaySpeechBubble)
    {
        SBLocation = npc->GetActorLocation() + FVector(0.f, 0.f, 93.f);

        SpeechBubble->SetActorLocation(SBLocation);
        SpeechBubble->SetActorHiddenInGame(false);

        bSpeechBuubbleVisible = true;
    }
}

void AMainPlayerController::RemoveSpeechBuubble()
{
    if (SpeechBubble)
    {
        bCanDisplaySpeechBubble = false;
        bSpeechBuubbleVisible = false;
        SpeechBubble->SetActorHiddenInGame(true);
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

    if (bSpeechBuubbleVisible)
    {
        SpeechBubble->SetActorRotation(Main->GetControlRotation() + FRotator(0.f, 180.f, 0.f));
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
        bCanDisplaySpeechBubble = false;
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


        if (bFallenPlayer && (FallingCount == 1 || FallingCount == 5))
        {
            DialogueUI->InitializeDialogue(SpawnDialogue);
        }

        bDialogueUIVisible = true;

        if (!bFallenPlayer)
        {
            switch (DialogueNum)
            {
                case 0:
                case 1:
                    DialogueUI->InitializeDialogue(IntroDialogue);
                    break;
                case 2:
                    DialogueUI->InitializeDialogue(DungeonDialogue1);
                    break;
                case 3: //after golem battle
                    if (!bFadeOn)
                    {
                        FadeAndDialogue();
                        return;
                    }
                    bCanDisplaySpeechBubble = true;
                    DialogueUI->InitializeDialogue(DungeonDialogue2);
                    bFadeOn = false;
                    break;
                case 4: // the boat move
                    DialogueUI->InitializeDialogue(DungeonDialogue2);
                    break;
                case 5: // second dungeon
                    bCanDisplaySpeechBubble = true;
                    DialogueUI->InitializeDialogue(DungeonDialogue3);
                    break;
                case 6: // triggerbox1 overplap
                case 7: // triggerbox2, player jump to plane
                case 8: // triggerbox3, player go over the other side
                case 9: // plane2 up
                case 10: // Npcs went over the other side
                    if(DialogueNum == 8 || DialogueNum == 9) bCanDisplaySpeechBubble = true;
                    DialogueUI->InitializeDialogue(DungeonDialogue4);
                    break;
                case 11: // discover food table trap
                case 16: // move to the rocks
                case 18:// after combat with boss
                case 20: // discover divinum~, someone take the divinum~
                    if (!bFadeOn)
                    {
                        FadeAndDialogue();
                        if (DialogueNum == 18) Main->SaveGame();
                        return;
                    }
                    bCanDisplaySpeechBubble = true;
                    if(DialogueNum == 11)
                        DialogueUI->InitializeDialogue(DungeonDialogue5);
                    else if(DialogueNum == 16 || DialogueNum == 18)
                        DialogueUI->InitializeDialogue(DungeonDialogue6);
                    else if (DialogueNum == 20)
                        DialogueUI->InitializeDialogue(FinalDialogue);
                    bFadeOn = false;
                    break;
                case 12: // after combat with spiders
                case 13: // before combat with final monsters in second dungeon
                case 14: // after combat with little monsters
                    if (bCalculateOn)
                    {
                        bDialogueUIVisible = false;
                        bCalculateOn = false;
                        CalculateDialogueDistance();
                        return;
                    }
                    bCanDisplaySpeechBubble = true;
                    DialogueUI->InitializeDialogue(DungeonDialogue5);
                    break;
                case 15: // boss level enter
                case 17: // be ready to combat with boss 
                    bCanDisplaySpeechBubble = true;
                    DialogueUI->InitializeDialogue(DungeonDialogue6);
                    break;
                case 19: // back to cave
                case 21: // griffons come
                case 22: // last talk
                case 23:
                    if(DialogueNum != 23)
                        bCanDisplaySpeechBubble = true;
                    DialogueUI->InitializeDialogue(FinalDialogue);
                    break;
            }
            
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
        if (!bFallenPlayer)
        {
            DialogueNum++;
            DialogueEvents();
        }
        else
        {
            Main->bCanMove = true;
            if(bSpeechBuubbleVisible)
                RemoveSpeechBuubble();
        }
              
        bDialogueUIVisible = false;

        bShowMouseCursor = false;

        FInputModeGameOnly InputModeGameOnly;
        SetInputMode(InputModeGameOnly);

        DialogueUI->OnAnimationHideMessageUI();

        if (Main->bSkip) Main->bSkip = false;
    }
}


void AMainPlayerController::DialogueEvents()
{
    RemoveSpeechBuubble();

    switch (DialogueNum)
    {
        case 1: // luko moves to player   
            Main->Luko->MoveToPlayer();         
            break;
        case 2:
            if (SystemMessageNum != 3)
            {
                Main->bInterpToNpc = false;
                Main->TargetNpc = nullptr;
                GetWorldTimerManager().ClearTimer(Main->Luko->MoveTimer);
                Main->Luko->AIController->MoveToLocation(FVector(5200.f, 35.f, 100.f));
                SystemMessageNum = 2;
                SetSystemMessage();
                return;
            }
            SetCinematicMode(false, true, true);   
            if (Main->EquippedWeapon == nullptr) // get the wand
            {
                SetSystemMessage();
            }
            break;
        case 3: // enter the first dungeon
            Main->Luko->bInterpToCharacter = false;
            Main->Luko->TargetCharacter = nullptr;
            Main->SaveGame();
            SystemMessageNum = 5;
            SetSystemMessage();
            break;
        case 4: // move to boat
            SetCinematicMode(false, true, true);
            Main->Vovo->bInterpToCharacter = false;
            Main->Vovo->AIController->MoveToLocation(FVector(630.f, 970.f, 1840.f));
            Main->bInterpToNpc = false;
            Main->TargetNpc = nullptr;
            break;
        case 5:
            GetWorld()->GetTimerManager().ClearTimer(DialogueUI->OnceTimer);
            break;
        case 6: // enter the second dungeon
            SetCinematicMode(false, true, true);
            for (int i = 0; i < Main->NPCList.Num(); i++)
            {
                if (Main->NPCList[i] != Main->Momo) // momo's already follwing to player
                {
                    Main->NPCList[i]->MoveToPlayer();
                }
            }
            break;
        case 7:
        case 8:
            Main->bCanMove = true;
            break;
        case 9: // player go over the other side
            if (Main->Vivi->NormalMontage != nullptr)
            {
                Main->Vivi->AnimInstance->Montage_Play(Main->Vivi->NormalMontage);
                Main->Vivi->AnimInstance->Montage_JumpToSection(FName("Throw"));
            }
            Main->bInterpToNpc = false;
            Main->TargetNpc = nullptr;
            break;
        case 10:
            Main->Zizi->bInterpToCharacter = false;
            Main->bInterpToNpc = false;
            Main->AllNpcMoveToPlayer();
            break;
        case 11: // npcs went over the other side
        case 19: // after combat with boss
            SetCinematicMode(false, true, true);
            Main->bCanMove = true;
            Main->bInterpToNpc = false;
            Main->TargetNpc = nullptr;
            DialogueUI->AllNpcDisableLookAt();
            if (DialogueNum == 19)
            {
                Main->Momo->AIController->MoveToLocation(FVector(8.f, -3585.f, 684.f));
                Main->Luko->AIController->MoveToLocation(FVector(8.f, -3585.f, 684.f));
                Main->Vovo->AIController->MoveToLocation(FVector(8.f, -3585.f, 684.f));
                Main->Vivi->AIController->MoveToLocation(FVector(8.f, -3585.f, 684.f));
                Main->Zizi->AIController->MoveToLocation(FVector(8.f, -3585.f, 684.f));
                SystemMessageNum = 13;
                SetSystemMessage();
            }
            break;
        case 12:
            SetCinematicMode(false, true, true);
            Main->bCanMove = true;
            ResetIgnoreMoveInput();
            ResetIgnoreLookInput();
            GetWorld()->GetTimerManager().ClearTimer(DialogueUI->OnceTimer);
            break;
        case 13:
        case 14:
            Main->bCanMove = true;
            Main->AllNpcMoveToPlayer();
            break;
        case 15:
            DialogueUI->bDisableMouseAndKeyboard = false;
            Main->bCanMove = true;
            SystemMessageNum = 11;
            SetSystemMessage();
            Main->AllNpcMoveToPlayer();
            break;
        case 16:
            SetCinematicMode(false, true, true);
            DialogueUI->AllNpcDisableLookAt();
            Main->bInterpToNpc = false;
            DialogueUI->bDisableMouseAndKeyboard = false;
            break;
        case 17:
        case 22:
        case 23:
            DialogueUI->bDisableMouseAndKeyboard = false;
            SetCinematicMode(false, true, true);
            Main->bCanMove = true;
            if(DialogueNum == 23)
                Main->Vivi->AIController->MoveToLocation(FVector(625.f, 318.f, 153.f));
            break;
        case 18: // fog appear, boss combat soon
            DialogueUI->bDisableMouseAndKeyboard = false;
            Main->bCanMove = true;
            Main->AllNpcMoveToPlayer();
            SystemMessageNum = 12;
            SetSystemMessage();
            Main->SaveGame();
            GetWorld()->GetTimerManager().SetTimer(Timer, FTimerDelegate::CreateLambda([&]() {

                RemoveSystemMessage();

                }), 4.f, false);
            break;
        case 20:
            SetCinematicMode(false, true, true);
            break;
        case 21:
            DialogueUI->bDisableMouseAndKeyboard = false;
            if (DialogueUI->SelectedReply == 1)
            {
                SetCinematicMode(false, true, true);
                Main->bCanMove = true;
                SystemMessageNum = 14;
                SetSystemMessage();
            }
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

            if (bFallenPlayer && ((FallingCount == 0 || FallingCount >= 2)))
            {
                if(FallingCount != 6)
                    FallingCount += 1;
            }

            if (DialogueNum == 11 || DialogueNum == 18 || DialogueNum == 20) // second dungeon food trap
            {
                Main->bCanMove = false;
                SetCinematicMode(true, true, true);

            }
            FadeOut();
        }
    }
    
}

void AMainPlayerController::SetPositions()
{
    for (AYaroCharacter* npc : Main->NPCList) 
    {
        npc->AIController->StopMovement();
        GetWorldTimerManager().ClearTimer(npc->MoveTimer);
        GetWorldTimerManager().ClearTimer(npc->TeamMoveTimer);
    }

    if (DialogueNum == 3) // after golem battle
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

    if (DialogueNum == 11)
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

    //if (DialogueNum == 12)
    //{
    //    Main->SetActorLocation(FVector(-260.f, -1981.f, -117.f));
    //    Main->SetActorRotation(FRotator(0.f, 99.f, 0.f)); // y(pitch), z(yaw), x(roll)
    //   
    //    Main->Momo->SetActorLocation(FVector(-301.f, -1663.f, -122.f));
    //    Main->Momo->SetActorRotation(FRotator(0.f, 274.f, 0.f));
 
    //    Main->Luko->SetActorLocation(FVector(-379.f, -1811.f, -117.f));
    //    Main->Luko->SetActorRotation(FRotator(0.f, 54.f, 0.f));

    //    Main->Vovo->SetActorLocation(FVector(-313.f, -2755.f, -117.5f));
    //    Main->Vovo->SetActorRotation(FRotator(0.f, 140.f, 0.f));
    //    
    //    Main->Vivi->SetActorLocation(FVector(-416.f, -1945.f, -117.5f));
    //    Main->Vivi->SetActorRotation(FRotator(0.f, 65.f, 0.f));

    //    Main->Zizi->SetActorLocation(FVector(-186.f, -1861.f, -115.f));
    //    Main->Zizi->SetActorRotation(FRotator(0.f, 128.f, 0.f));
    //}

    if (DialogueNum == 16)
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

    if (DialogueNum == 18)
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

    if (DialogueNum == 19)
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

    if (DialogueNum == 20)
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
    }
    SystemMessageOn = true;
    //UE_LOG(LogTemp, Log, TEXT("SetSystemMessage"));

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
        if (DialogueNum == 2 && SystemMessageNum != 4)
        {
            Main->bCanMove = true;
            SystemMessageNum = 3;
            DialogueEvents();
        }
    }
}
