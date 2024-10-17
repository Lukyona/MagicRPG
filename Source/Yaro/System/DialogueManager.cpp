// Fill out your copyright notice in the Description page of Project Settings.


#include "Yaro/System/DialogueManager.h"
#include "EngineUtils.h"
#include "Engine/DataTable.h"
#include "Yaro/System/GameManager.h"
#include "Yaro/System/NPCManager.h"
#include "Yaro/DialogueUI.h"
#include "Yaro/Character/Main.h"


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
    GameManager = Cast<UGameManager>(GetWorld()->GetGameInstance());
    if (GameManager)
    {
        Player = GameManager->GetPlayer();
        NPCManager = GameManager->GetNPCManager();
    }
    else return;

    if (DialogueUIClass != nullptr)
    {
        DialogueUI = CreateWidget<UDialogueUI>(GameManager, DialogueUIClass);
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
        SpeechBubble->SetActorRotation(GameManager->GetPlayer()->GetControlRotation() + FRotator(0.f, 180.f, 0.f));
    }
}

void UDialogueManager::DisplayDialogueUI()
{
    if (DialogueUI)
    {
        //UE_LOG(LogTemp, Log, TEXT("DisplayDialogueUI"));

        if (!DialogueUI->bCanStartDialogue) return;

       // if (bManualVisible) RemoveManual();


       /* if (bFallenPlayer && (FallingCount == 1 || FallingCount == 5))
        {
            DialogueUI->InitializeDialogue(DialogueDatas.Last(0));
        }*/

        bDialogueUIVisible = true;
        /*
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
                //bCanDisplaySpeechBubble = true;
                DialogueUI->InitializeDialogue(DialogueDatas[2]);
                bFadeOn = false;
                break;
            case 4: // the boat move
                DialogueUI->InitializeDialogue(DialogueDatas[2]);
                break;
            case 5: // second dungeon
                //bCanDisplaySpeechBubble = true;
                DialogueUI->InitializeDialogue(DialogueDatas[3]);
                break;
            case 6: // triggerbox1 overplap
            case 7: // triggerbox2, player jump to plane
            case 8: // triggerbox3, player go over the other side
            case 9: // plane2 up
            case 10: // Npcs went over the other side
                //if (DialogueNum == 8 || DialogueNum == 9) bCanDisplaySpeechBubble = true;
                DialogueUI->InitializeDialogue(DialogueDatas[4]);
                break;
            case 11: // discover food table trap
            case 16: // move to the rocks
            case 18:// after combat with boss
            case 20: // discover divinum~, someone take the divinum~
                if (!bFadeOn)
                {
                    FadeAndDialogue();
                    if (DialogueNum == 18) Player->SaveGame();
                    return;
                }
                //bCanDisplaySpeechBubble = true;
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
                //bCanDisplaySpeechBubble = true;
                DialogueUI->InitializeDialogue(DialogueDatas[5]);
                break;
            case 15: // boss level enter
            case 17: // be ready to combat with boss 
               // bCanDisplaySpeechBubble = true;
                DialogueUI->InitializeDialogue(DialogueDatas[6]);
                break;
            case 19: // back to cave
            case 21: // griffons come
            case 22: // last talk
            case 23:
               // if (DialogueNum != 23)
                 //   bCanDisplaySpeechBubble = true;
                DialogueUI->InitializeDialogue(DialogueDatas[7]);
                break;
            }

        }

        DialogueUI->SetVisibility(ESlateVisibility::Visible);

        FInputModeGameAndUI InputMode;
        SetInputMode(InputMode);
        bShowMouseCursor = true;*/
    }
}

void UDialogueManager::RemoveDialogueUI()
{
    if (DialogueUI)
    {/*
        if (!bFallenPlayer)
        {
            DialogueNum++;
            DialogueEvents();
        }
        else
        {
            Player->SetCanMove(true);
            if (bSpeechBuubbleVisible)
                RemoveSpeechBuubble();
        }

        bDialogueUIVisible = false;

        bShowMouseCursor = false;

        FInputModeGameOnly InputModeGameOnly;
        SetInputMode(InputModeGameOnly);
        
        DialogueUI->OnAnimationHideMessageUI();
        */
        if (Player->GetSkipStatus()) Player->SetSkipStatus(false);
    }
}

void UDialogueManager::DialogueEvents()
{
    RemoveSpeechBuubble();
    
    switch (DialogueNum)
    {
    case 1: // luko moves to player   
        NPCManager->GetNPC("Luko")->MoveToPlayer();
        break;
    case 2:
        if (SystemMessageNum != 3)
        {
            Player->SetInterpToCharacter(false);
            Player->SetTargetCharacter(nullptr);
            GetWorldTimerManager().ClearTimer(NPCManager->GetNPC("Luko")->GetMoveTimer());
            NPCManager->MoveNPCToLocation("Luko", FVector(5200.f, 35.f, 100.f));
            SystemMessageNum = 16;
            SetSystemMessage();
            GetWorld()->GetTimerManager().SetTimer(Timer, FTimerDelegate::CreateLambda([&]() {

                SystemMessageNum = 2;
                SetSystemMessage();

                }), 2.5f, false);
            return;
        }
        SetCinematicMode(false, true, true);
        if (Player->GetEquippedWeapon() == nullptr) // get the wand
        {
            SetSystemMessage();
        }
        break;
    case 3: // enter the first dungeon
        NPCManager->GetNPC("Luko")->SetInterpToCharacter(false);
        NPCManager->GetNPC("Luko")->SetTargetCharacter(nullptr);
        GameManager->SaveGame();
        SystemMessageNum = 5;
        SetSystemMessage();
        break;
    case 4: // move to boat
        SetCinematicMode(false, true, true);
        NPCManager->GetNPC("Vovo")->SetInterpToCharacter(false);
        NPCManager->MoveNPCToLocation("Vovo", FVector(630.f, 970.f, 1840.f));
        Player->SetInterpToCharacter(false);
        Player->SetTargetCharacter(nullptr);
        break;
    case 5:
        GetWorld()->GetTimerManager().ClearTimer(DialogueUI->OnceTimer);
        break;
    case 6: // enter the second dungeon
        SetCinematicMode(false, true, true);
        NPCManager->GetNPC("Momo")->UsualFace();
        for (auto NPC : NPCManager->GetNPCMap())
        {
            if (NPC.Key != "Momo") // momo's already follwing to player
            {
                NPC.Value->MoveToPlayer();
            }
        }
        break;
    case 7:
    case 8:
        Player->SetCanMove(true);
        break;
    case 9: // player go over the other side
        if (NPCManager->GetNPC("Vivi")->GetNormalMontage() != nullptr)
        {
            NPCManager->GetNPC("Vivi")->GetAnimInstance()->Montage_Play(NPCManager->GetNPC("Vivi")->GetNormalMontage());
            NPCManager->GetNPC("Vivi")->GetAnimInstance()->Montage_JumpToSection(FName("Throw"));
        }
        Player->SetInterpToCharacter(false);
        Player->SetTargetCharacter(nullptr);
        break;
    case 10:
        NPCManager->GetNPC("Zizi")->SetInterpToCharacter(false);
        Player->SetInterpToCharacter(false);
        NPCManager->AllNpcMoveToPlayer();
        break;
    case 11: // npcs went over the other side
    case 19: // after combat with boss
        SetCinematicMode(false, true, true);
        Player->SetCanMove(true);
        Player->SetInterpToCharacter(false);
        Player->SetTargetCharacter(nullptr);
        NPCManager->AllNpcDisableLookAt();
        if (DialogueNum == 19)
        {
            NPCManager->MoveNPCToLocation("Momo", FVector(8.f, -3585.f, 684.f));
            NPCManager->MoveNPCToLocation("Luko", FVector(8.f, -3585.f, 684.f));
            NPCManager->MoveNPCToLocation("Vovo", FVector(8.f, -3585.f, 684.f));
            NPCManager->MoveNPCToLocation("Vivi", FVector(8.f, -3585.f, 684.f));
            NPCManager->MoveNPCToLocation("Zizi", FVector(8.f, -3585.f, 684.f));
            SystemMessageNum = 13;
            SetSystemMessage();
        }
        if (DialogueNum == 11)
        {
            for (auto NPC : NPCManager->GetNPCMap())
            {
                NPC.Value->UsualFace();
            }
        }
        break;
    case 12:
        SetCinematicMode(false, true, true);
        Player->SetCanMove(true);
        ResetIgnoreMoveInput();
        ResetIgnoreLookInput();
        GetWorld()->GetTimerManager().ClearTimer(DialogueUI->OnceTimer);
        break;
    case 13:
    case 14:
        NPCManager->GetNPC("Momo")->UsualFace();
        Player->SetCanMove(true);

        NPCManager->AllNpcMoveToPlayer();
        break;
    case 15:
        DialogueUI->bDisableMouseAndKeyboard = false;
        Player->SetCanMove(true);

        SystemMessageNum = 11;
        SetSystemMessage();
        NPCManager->AllNpcMoveToPlayer();
        break;
    case 16:
        SetCinematicMode(false, true, true);
        NPCManager->AllNpcDisableLookAt();
        Player->SetInterpToCharacter(false);
        DialogueUI->bDisableMouseAndKeyboard = false;
        break;
    case 17:
    case 22:
    case 23:
        DialogueUI->bDisableMouseAndKeyboard = false;
        SetCinematicMode(false, true, true);
        Player->SetCanMove(true);

        if (DialogueNum == 23)
            NPCManager->MoveNPCToLocation("Vivi", FVector(625.f, 318.f, 153.f));
        break;
    case 18: // fog appear, boss combat soon
        DialogueUI->bDisableMouseAndKeyboard = false;
        Player->SetCanMove(true);
        NPCManager->AllNpcMoveToPlayer();
        SystemMessageNum = 12;
        SetSystemMessage();
        GameManager->SaveGame();
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
            Player->SetCanMove(true);

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
