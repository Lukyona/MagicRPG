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
                DialogueUI->InitializeDialogue(DungeonDialogue2);
                bFadeOn = false;
                break;
            case 4: // the boat move
                DialogueUI->InitializeDialogue(DungeonDialogue2);
                break;
            case 5: // second dungeon
                DialogueUI->InitializeDialogue(DungeonDialogue3);
                break;
            case 6: // triggerbox1 overplap
            case 7: // triggerbox2, player jump to plane
            case 8: // triggerbox3, player go over the other side
            case 9: // plane2 up
            case 10: // Npcs went over the other side
                DialogueUI->InitializeDialogue(DungeonDialogue4);
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
        }
              
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
            for (int i = 0; i < Main->NPCList.Num(); i++)
            {
                Main->NPCList[i]->MoveToPlayer();
            }
            break;
        case 11: // npcs went over the other side
            Main->bCanMove = true;
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

            if (bFallenPlayer) FallingCount += 1;

            if (DialogueNum == 3)
            {
                SetCinematicMode(true, true, true);
                SetControlRotation(FRotator(0.f, 57.f, 0.f));
            }
            FadeOut();
            FadeInOut->AddToViewport();
        }
    }
    
}

void AMainPlayerController::SetPositions()
{
    if (DialogueNum == 3) // after golem battle
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
            text = FString(TEXT("마우스 왼쪽 버튼을 클릭하여 돌을 줍고\n움직이는 돌 가까이서 마우스 왼쪽 버튼을 클릭하여 돌을 놓으세요."));
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
            FTimerHandle Timer;
            GetWorld()->GetTimerManager().SetTimer(Timer, FTimerDelegate::CreateLambda([&]() {
                
                if (SystemMessageOn) SetSystemMessage();
                else if (bSystemMessageVisible) RemoveSystemMessage();
                
            }), 2.f, false);
        }
        else
        {
            if (ManualSoundCue != nullptr)
                UAudioComponent* AudioComponent = UGameplayStatics::SpawnSound2D(this, ManualSoundCue);

            bManualVisible = true;
            Manual->SetVisibility(ESlateVisibility::Visible);
        }
    }
}

void AMainPlayerController::RemoveManual()
{
    if (Manual)
    {
        if (ManualSoundCue != nullptr)
            UAudioComponent* AudioComponent = UGameplayStatics::SpawnSound2D(this, ManualSoundCue);

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
