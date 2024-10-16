// Fill out your copyright notice in the Description page of Project Settings.


#include "Yaro/System/DialogueManager.h"
#include "EngineUtils.h"
#include "Engine/DataTable.h"
#include "Yaro/DialogueUI.h"


UDialogueManager::UDialogueManager()
{
    static ConstructorHelpers::FClassFinder<UUserWidget> DialogueBPClass(TEXT("/Game/HUDandWigets/DialogueUI_BP.DialogueUI_BP_C"));

    if (ensure(DialogueBPClass.Class != nullptr)) DialogueUIClass = DialogueBPClass.Class;

    TArray<UObject*> Assets; // 동작 됨?
    EngineUtils::FindOrLoadAssetsByPath(TEXT("/Game/DialogueDatas"), Assets, EngineUtils::ATL_Class);

    for (UObject* Asset : Assets)
    {
        if (UDataTable* DataTable = Cast<UDataTable>(Asset))
        {
            DialogueDatas.Add(DataTable);
        }
    }

    DialogueDatas.Sort([](const UDataTable& A, const UDataTable& B) {
        return A.GetName() < B.GetName();  // 이름(오름차순)으로 정렬
        });

}

void UDialogueManager::Init()
{
    if (DialogueUIClass != nullptr)
    {
        DialogueUI = CreateWidget<UDialogueUI>(this, DialogueUIClass);
    }

    if (DialogueUI != nullptr)
    {
        DialogueUI->AddToViewport();
        DialogueUI->SetVisibility(ESlateVisibility::Hidden);
    }

    SpeechBubble = GetWorld()->SpawnActor<AActor>(SpeechBubble_BP);

}

void UDialogueManager::Tick()
{
    if (bSpeechBuubbleVisible)
    {
        SpeechBubble->SetActorRotation(Main->GetControlRotation() + FRotator(0.f, 180.f, 0.f));
    }
}

void UDialogueManager::DisplayDialogueUI()
{
    if (DialogueUI)
    {
        //UE_LOG(LogTemp, Log, TEXT("DisplayDialogueUI"));

        if (!DialogueUI->bCanStartDialogue) return;

        if (bManualVisible) RemoveManual();


        if (bFallenPlayer && (FallingCount == 1 || FallingCount == 5))
        {
            DialogueUI->InitializeDialogue(DialogueDatas.Last(0));
        }

        bDialogueUIVisible = true;

        if (!bFallenPlayer)
        {
            switch (DialogueNum)
            {
            case 0:
            case 1:
                DialogueUI->InitializeDialogue(DialogueDatas[0]);
                break;
            case 2:
                DialogueUI->InitializeDialogue(DialogueDatas[1]);
                break;
            case 3: //after golem battle
                if (!bFadeOn)
                {
                    FadeAndDialogue();
                    return;
                }
                bCanDisplaySpeechBubble = true;
                DialogueUI->InitializeDialogue(DialogueDatas[2]);
                bFadeOn = false;
                break;
            case 4: // the boat move
                DialogueUI->InitializeDialogue(DialogueDatas[2]);
                break;
            case 5: // second dungeon
                bCanDisplaySpeechBubble = true;
                DialogueUI->InitializeDialogue(DialogueDatas[3]);
                break;
            case 6: // triggerbox1 overplap
            case 7: // triggerbox2, player jump to plane
            case 8: // triggerbox3, player go over the other side
            case 9: // plane2 up
            case 10: // Npcs went over the other side
                if (DialogueNum == 8 || DialogueNum == 9) bCanDisplaySpeechBubble = true;
                DialogueUI->InitializeDialogue(DialogueDatas[4]);
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
                if (DialogueNum == 11)
                    DialogueUI->InitializeDialogue(DialogueDatas[5]);
                else if (DialogueNum == 16 || DialogueNum == 18)
                    DialogueUI->InitializeDialogue(DialogueDatas[6]);
                else if (DialogueNum == 20)
                    DialogueUI->InitializeDialogue(DialogueDatas[7]);
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
                DialogueUI->InitializeDialogue(DialogueDatas[5]);
                break;
            case 15: // boss level enter
            case 17: // be ready to combat with boss 
                bCanDisplaySpeechBubble = true;
                DialogueUI->InitializeDialogue(DialogueDatas[6]);
                break;
            case 19: // back to cave
            case 21: // griffons come
            case 22: // last talk
            case 23:
                if (DialogueNum != 23)
                    bCanDisplaySpeechBubble = true;
                DialogueUI->InitializeDialogue(DialogueDatas[7]);
                break;
            }

        }

        DialogueUI->SetVisibility(ESlateVisibility::Visible);

        FInputModeGameAndUI InputMode;
        SetInputMode(InputMode);
        bShowMouseCursor = true;
    }
}

void UDialogueManager::RemoveDialogueUI()
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
            Main->SetCanMove(true);
            if (bSpeechBuubbleVisible)
                RemoveSpeechBuubble();
        }

        bDialogueUIVisible = false;

        bShowMouseCursor = false;

        FInputModeGameOnly InputModeGameOnly;
        SetInputMode(InputModeGameOnly);

        DialogueUI->OnAnimationHideMessageUI();

        if (Main->GetSkipStatus()) Main->SetSkipStatus(false);
    }
}

void UDialogueManager::DialogueEvents()
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
            Main->SetInterpToCharacter(false);
            Main->SetTargetCharacter(nullptr);
            GetWorldTimerManager().ClearTimer(Main->Luko->GetMoveTimer());
            Main->Luko->GetAIController()->MoveToLocation(FVector(5200.f, 35.f, 100.f));
            SystemMessageNum = 16;
            SetSystemMessage();
            GetWorld()->GetTimerManager().SetTimer(Timer, FTimerDelegate::CreateLambda([&]() {

                SystemMessageNum = 2;
                SetSystemMessage();

                }), 2.5f, false);
            return;
        }
        SetCinematicMode(false, true, true);
        if (Main->GetEquippedWeapon() == nullptr) // get the wand
        {
            SetSystemMessage();
        }
        break;
    case 3: // enter the first dungeon
        Main->Luko->SetInterpToCharacter(false);
        Main->Luko->SetTargetCharacter(nullptr);
        Main->SaveGame();
        SystemMessageNum = 5;
        SetSystemMessage();
        break;
    case 4: // move to boat
        SetCinematicMode(false, true, true);
        Main->Vovo->SetInterpToCharacter(false);
        Main->Vovo->GetAIController()->MoveToLocation(FVector(630.f, 970.f, 1840.f));
        Main->SetInterpToCharacter(false);
        Main->SetTargetCharacter(nullptr);
        break;
    case 5:
        GetWorld()->GetTimerManager().ClearTimer(DialogueUI->OnceTimer);
        break;
    case 6: // enter the second dungeon
        SetCinematicMode(false, true, true);
        Main->Momo->UsualFace();
        for (int i = 0; i < Main->GetNPCList().Num(); i++)
        {
            if (Main->GetNPCList()[i] != Main->Momo) // momo's already follwing to player
            {
                Main->GetNPCList()[i]->MoveToPlayer();
            }
        }
        break;
    case 7:
    case 8:
        Main->SetCanMove(true);
        break;
    case 9: // player go over the other side
        if (Main->Vivi->GetNormalMontage() != nullptr)
        {
            Main->Vivi->GetAnimInstance()->Montage_Play(Main->Vivi->GetNormalMontage());
            Main->Vivi->GetAnimInstance()->Montage_JumpToSection(FName("Throw"));
        }
        Main->SetInterpToCharacter(false);
        Main->SetTargetCharacter(nullptr);
        break;
    case 10:
        Main->Zizi->SetInterpToCharacter(false);
        Main->SetInterpToCharacter(false);
        Main->AllNpcMoveToPlayer();
        break;
    case 11: // npcs went over the other side
    case 19: // after combat with boss
        SetCinematicMode(false, true, true);
        Main->SetCanMove(true);
        Main->SetInterpToCharacter(false);
        Main->SetTargetCharacter(nullptr);
        DialogueUI->AllNpcDisableLookAt();
        if (DialogueNum == 19)
        {
            Main->Momo->GetAIController()->MoveToLocation(FVector(8.f, -3585.f, 684.f));
            Main->Luko->GetAIController()->MoveToLocation(FVector(8.f, -3585.f, 684.f));
            Main->Vovo->GetAIController()->MoveToLocation(FVector(8.f, -3585.f, 684.f));
            Main->Vivi->GetAIController()->MoveToLocation(FVector(8.f, -3585.f, 684.f));
            Main->Zizi->GetAIController()->MoveToLocation(FVector(8.f, -3585.f, 684.f));
            SystemMessageNum = 13;
            SetSystemMessage();
        }
        if (DialogueNum == 11)
        {
            for (int i = 0; i < Main->GetNPCList().Num(); i++)
            {
                Main->GetNPCList()[i]->UsualFace();
            }
        }
        break;
    case 12:
        SetCinematicMode(false, true, true);
        Main->SetCanMove(true);
        ResetIgnoreMoveInput();
        ResetIgnoreLookInput();
        GetWorld()->GetTimerManager().ClearTimer(DialogueUI->OnceTimer);
        break;
    case 13:
    case 14:
        Main->Momo->UsualFace();
        Main->SetCanMove(true);

        Main->AllNpcMoveToPlayer();
        break;
    case 15:
        DialogueUI->bDisableMouseAndKeyboard = false;
        Main->SetCanMove(true);

        SystemMessageNum = 11;
        SetSystemMessage();
        Main->AllNpcMoveToPlayer();
        break;
    case 16:
        SetCinematicMode(false, true, true);
        DialogueUI->AllNpcDisableLookAt();
        Main->SetInterpToCharacter(false);
        DialogueUI->bDisableMouseAndKeyboard = false;
        break;
    case 17:
    case 22:
    case 23:
        DialogueUI->bDisableMouseAndKeyboard = false;
        SetCinematicMode(false, true, true);
        Main->SetCanMove(true);

        if (DialogueNum == 23)
            Main->Vivi->GetAIController()->MoveToLocation(FVector(625.f, 318.f, 153.f));
        break;
    case 18: // fog appear, boss combat soon
        DialogueUI->bDisableMouseAndKeyboard = false;
        Main->SetCanMove(true);
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
            Main->SetCanMove(true);

            SystemMessageNum = 14;
            SetSystemMessage();
        }
        break;
    }
}

void UDialogueManager::DisplaySpeechBuubble(class AYaroCharacter* npc)
{
    if (SpeechBubble) //&& bCanDisplaySpeechBubble)
    {
        SBLocation = npc->GetActorLocation() + FVector(0.f, 0.f, 93.f);

        SpeechBubble->SetActorLocation(SBLocation);
        SpeechBubble->SetActorHiddenInGame(false);

        //bSpeechBuubbleVisible = true;
    }
}

void UDialogueManager::RemoveSpeechBuubble()
{
    if (SpeechBubble)
    {
       // bCanDisplaySpeechBubble = false;
        bSpeechBuubbleVisible = false;
        SpeechBubble->SetActorHiddenInGame(true);
    }

}
