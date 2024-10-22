// Fill out your copyright notice in the Description page of Project Settings.


#include "Yaro/System/DialogueManager.h"
#include "EngineUtils.h"
#include "Engine/DataTable.h"
#include "Yaro/System/GameManager.h"
#include "Yaro/System/NPCManager.h"
#include "Yaro/System/UIManager.h"
#include "Yaro/System/MainPlayerController.h"
#include "Yaro/DialogueUI.h"
#include "Yaro/Character/Main.h"

UDialogueManager* UDialogueManager::Instance = nullptr; // 정적 멤버 변수 초기화

void UDialogueManager::Init()
{
    GameManager = Cast<UGameManager>(GetWorld()->GetGameInstance());
    if (GameManager)
    {
        NPCManager = GameManager->GetNPCManager();
        UIManager = GameManager->GetUIManager();
        Player = GameManager->GetPlayer();
        MainPlayerController = GameManager->GetMainPlayerController();
    }
    else return;

    static ConstructorHelpers::FClassFinder<UUserWidget> DialogueBPClass(TEXT("/Game/HUDandWigets/DialogueUI_BP.DialogueUI_BP_C"));

    if (ensure(DialogueBPClass.Class != nullptr))
    {
        DialogueUI = CreateWidget<UDialogueUI>(GameManager, DialogueBPClass.Class);
    }

    if (DialogueUI != nullptr)
    {
        DialogueUI->AddToViewport();
        DialogueUI->SetVisibility(ESlateVisibility::Hidden);
    }

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


    SpeechBubble = GetWorld()->SpawnActor<AActor>(SpeechBubble_BP);

}

void UDialogueManager::Tick()
{
    if (bSpeechBuubbleVisible)
    {
        SpeechBubble->SetActorRotation(GameManager->GetPlayer()->GetControlRotation() + FRotator(0.f, 180.f, 0.f));
    }
}

void UDialogueManager::CheckDialogueStartCondition()
{
    if (GameManager->IsSkipping() || (!Player->IsDead() && NPCManager->IsNPCInTalkRange()))
    {
        DisplayDialogueUI();
    }
    else
    {
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UDialogueManager::CheckDialogueStartCondition, 2.f, false);
    }
}

void UDialogueManager::DisplayDialogueUI()
{
    if (DialogueUI)
    {
        if (!DialogueUI->bCanStartDialogue) return;

        if (UIManager->IsControlGuideVisible()) UIManager->RemoveControlGuide();

        uint8 FallCount = Player->GetFallCount();
        if (Player->IsFallenInDungeon() && (FallCount == 1 || FallCount == 5))
        {
            DialogueUI->InitializeDialogue(DialogueDatas.Last(0));
        }

        bDialogueUIVisible = true;
        
        if (!Player->IsFallenInDungeon())
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
                if (!UIManager->IsFading())
                {
                    UIManager->FadeAndDialogue();
                    return;
                }
                //bCanDisplaySpeechBubble = true;
                DialogueUI->InitializeDialogue(DialogueDatas[2]);
                UIManager->SetIsFading(false);
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
                if (!UIManager->IsFading())
                {
                    UIManager->FadeAndDialogue();
                    if (DialogueNum == 18) GameManager->SaveGame();
                    return;
                }
                //bCanDisplaySpeechBubble = true;
                if (DialogueNum == 11)
                    DialogueUI->InitializeDialogue(DialogueDatas[5]);
                else if (DialogueNum == 16 || DialogueNum == 18)
                    DialogueUI->InitializeDialogue(DialogueDatas[6]);
                else if (DialogueNum == 20)
                    DialogueUI->InitializeDialogue(DialogueDatas[7]);
                UIManager->SetIsFading(false);
                break;
            case 12: // after combat with spiders
            case 13: // before combat with final monsters in second dungeon
            case 14: // after combat with little monsters
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
        MainPlayerController->SetInputMode(InputMode);
        MainPlayerController->SetMouseCursorVisibility(true);
    }
}

void UDialogueManager::RemoveDialogueUI()
{
    if (DialogueUI)
    {
        if (!Player->IsFallenInDungeon())
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
        DialogueUI->OnAnimationHideMessageUI();

        FInputModeGameOnly InputModeGameOnly;
        MainPlayerController->SetInputMode(InputModeGameOnly);
        MainPlayerController->SetMouseCursorVisibility(false);
        
        if (GameManager->IsSkipping()) GameManager->SetIsSkipping(false);
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
        if (UIManager->GetSystemMessageNum() != 3)
        {
            Player->SetInterpToCharacter(false);
            Player->SetTargetCharacter(nullptr);
            GetWorld()->GetTimerManager().ClearTimer(NPCManager->GetNPC("Luko")->GetMoveTimer());
            NPCManager->MoveNPCToLocation("Luko", FVector(5200.f, 35.f, 100.f));
            UIManager->SetSystemMessage(16);

            FTimerHandle TimerHandle;
            GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateLambda([&]() {
                UIManager->SetSystemMessage(2);
                }), 2.5f, false);
            return;
        }
        MainPlayerController->SetCinematicMode(false, true, true);
        if (Player->GetEquippedWeapon() == nullptr) // get the wand
        {
            UIManager->SetSystemMessage(3);
        }
        break;
    case 3: // enter the first dungeon
        NPCManager->GetNPC("Luko")->SetInterpToCharacter(false);
        NPCManager->GetNPC("Luko")->SetTargetCharacter(nullptr);
        GameManager->SaveGame();
        UIManager->SetSystemMessage(5);
        break;
    case 4: // move to boat
        MainPlayerController->SetCinematicMode(false, true, true);
        NPCManager->GetNPC("Vovo")->SetInterpToCharacter(false);
        NPCManager->MoveNPCToLocation("Vovo", FVector(630.f, 970.f, 1840.f));
        Player->SetInterpToCharacter(false);
        Player->SetTargetCharacter(nullptr);
        break;
    case 5:
        GetWorld()->GetTimerManager().ClearTimer(DialogueUI->OnceTimer);
        break;
    case 6: // enter the second dungeon
        MainPlayerController->SetCinematicMode(false, true, true);
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
        MainPlayerController->SetCinematicMode(false, true, true);
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
            UIManager->SetSystemMessage(13);
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
        MainPlayerController->SetCinematicMode(false, true, true);
        Player->SetCanMove(true);
        MainPlayerController->ResetIgnoreMoveInput();
        MainPlayerController->ResetIgnoreLookInput();
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
        UIManager->SetSystemMessage(11);
        NPCManager->AllNpcMoveToPlayer();
        break;
    case 16:
        MainPlayerController->SetCinematicMode(false, true, true);
        NPCManager->AllNpcDisableLookAt();
        Player->SetInterpToCharacter(false);
        DialogueUI->bDisableMouseAndKeyboard = false;
        break;
    case 17:
    case 22:
    case 23:
        DialogueUI->bDisableMouseAndKeyboard = false;
        MainPlayerController->SetCinematicMode(false, true, true);
        Player->SetCanMove(true);

        if (DialogueNum == 23)
            NPCManager->MoveNPCToLocation("Vivi", FVector(625.f, 318.f, 153.f));
        break;
    case 18: // fog appear, boss combat soon
    {
        DialogueUI->bDisableMouseAndKeyboard = false;
        Player->SetCanMove(true);
        NPCManager->AllNpcMoveToPlayer();
        UIManager->SetSystemMessage(12);
        GameManager->SaveGame();

        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateLambda([&]() {
            UIManager->RemoveSystemMessage();
            }), 4.f, false);
        break;
    }
    case 20:
        MainPlayerController->SetCinematicMode(false, true, true);
        break;
    case 21:
        DialogueUI->bDisableMouseAndKeyboard = false;
        if (DialogueUI->SelectedReply == 1)
        {
            MainPlayerController->SetCinematicMode(false, true, true);
            Player->SetCanMove(true);
            UIManager->SetSystemMessage(14);
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
