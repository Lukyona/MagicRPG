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

    if (WPauseMenu)
    {
        PauseMenu = CreateWidget<UUserWidget>(this, WPauseMenu);
        if (PauseMenu)
        {
            PauseMenu->AddToViewport();
            PauseMenu->SetVisibility(ESlateVisibility::Hidden);
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

void AMainPlayerController::DisplayPauseMenu()
{
    if (PauseMenu)
    {
        bPauseMenuVisible = true;
        PauseMenu->SetVisibility(ESlateVisibility::Visible);

        FInputModeGameAndUI InputMode;
        SetInputMode(InputMode);
        bShowMouseCursor = true;
    }
}

void AMainPlayerController::RemovePauseMenu()
{
    if (PauseMenu)
    {      
        bPauseMenuVisible = false;
        PauseMenu->SetVisibility(ESlateVisibility::Hidden);

        FInputModeGameOnly InputModeGameOnly;
        SetInputMode(InputModeGameOnly);
        bShowMouseCursor = false;
    }
}

void AMainPlayerController::TogglePauseMenu()
{
    if (bPauseMenuVisible)
    {
        RemovePauseMenu();
    }
    else
    {
        DisplayPauseMenu();
    }
}

void AMainPlayerController::DisplayDialogueUI()
{
    if (DialogueUI)
    {
        //UE_LOG(LogTemp, Log, TEXT("DisplayDialogueUI"));

        if (!DialogueUI->bCanStartDialogue) return;

        bDialogueUIVisible = true;


        switch (DialogueNum)
        {
            case 0:
                DialogueUI->InitializeDialogue(IntroDialogue);
                break;
            case 1: // onlu luko dialogue
            case 4: // the boat move
                DialogueUI->OnAnimationShowMessageUI();
                GetWorld()->GetTimerManager().SetTimer(DialogueUI->TimerHandle, DialogueUI, &UDialogueUI::OnTimerCompleted, 0.1f, false);
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
       
        DialogueEvents();

        bDialogueUIVisible = false;

        bShowMouseCursor = false;

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
            for (AYaroCharacter* npc : Main->NPCList)
            {
                if (npc->GetName().Contains("Luko"))
                {
                    npc->MoveToPlayer();
                }
            }
            break;
        case 2:
            SetCinematicMode(false, true, true);
            for (AYaroCharacter* npc : Main->NPCList)
            {
                if (npc->GetName().Contains("Luko"))
                {
                    GetWorldTimerManager().ClearTimer(npc->MoveTimer);
                    npc->AIController->MoveToLocation(FVector(5200.f, 35.f, 100.f));
                }
            }
            break;
        case 3:
        case 4:
            SetCinematicMode(false, true, true);
            break;
    }
    

}

void AMainPlayerController::FadeAndDialogue()
{
    if (WFadeInOut) // 
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


            if (npc->GetName().Contains("Momo"))
            {
                npc->SetActorLocation(FVector(594.f, -1543.f, 2531.f));
                npc->SetActorRotation(FRotator(0.f, 280.f, 0.f));
            }
            else if (npc->GetName().Contains("Luko"))
            {
                npc->SetActorLocation(FVector(494.f, -1629.f, 2561.f));
                npc->SetActorRotation(FRotator(0.f, 6.f, 0.f));
            }
            else if (npc->GetName().Contains("Vovo"))
            {
                npc->SetActorLocation(FVector(903.f, -1767.f, 2574.f));
                npc->SetActorRotation(FRotator(0.f, 165.f, 0.f));
            }
            else if (npc->GetName().Contains("Vivi"))
            {

                npc->SetActorLocation(FVector(790.f, -1636.f, 2566.f));
                npc->SetActorRotation(FRotator(00.f, 180.f, 0.f));
            }
            else if (npc->GetName().Contains("Zizi"))
            {
                npc->SetActorLocation(FVector(978.f, -1650.f, 2553.f));
                npc->SetActorRotation(FRotator(0.f, 187.f, 0.f));
            }
        }
    }
}