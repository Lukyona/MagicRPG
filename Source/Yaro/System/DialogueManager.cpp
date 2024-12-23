// Fill out your copyright notice in the Description page of Project Settings.

#include "Yaro/System/DialogueManager.h"
#include "Engine/DataTable.h"
#include "EngineUtils.h"

#include "Yaro/System/GameManager.h"
#include "Yaro/System/NPCManager.h"
#include "Yaro/System/UIManager.h"
#include "Yaro/System/MainPlayerController.h"
#include "Yaro/Character/Main.h"
#include "Yaro/DialogueUI.h"

UDialogueManager* UDialogueManager::Instance = nullptr; // 정적 멤버 변수 초기화

void UDialogueManager::BeginPlay()
{
    if (GameManager)
    {
        NPCManager = GameManager->GetNPCManager();
        UIManager = GameManager->GetUIManager();
        Player = GameManager->GetPlayer();
        MainPlayerController = GameManager->GetMainPlayerController();
    }
    else 
    {
        return;
    }

    TSoftClassPtr<UUserWidget> DialogueBPClass(FSoftObjectPath(TEXT("/Game/HUDandWigets/DialogueUI.DialogueUI_C")));

    if (!DialogueBPClass.IsValid())
    {
        DialogueBPClass.LoadSynchronous();
    }
    if (ensure(DialogueBPClass.IsValid()))
    {
        DialogueUI = CreateWidget<UDialogueUI>(GameManager, DialogueBPClass.Get());
    }

    if (DialogueUI != nullptr)
    {
        DialogueUI->AddToViewport();
        DialogueUI->SetVisibility(ESlateVisibility::Hidden);
    }

    if (DialogueDatas.Num() == 0)
    {
        TArray<UObject*> Assets;
        FString DialogueDataPath = TEXT("/Game/DataTables/DialogueDatas");
        EngineUtils::FindOrLoadAssetsByPath(DialogueDataPath, Assets, EngineUtils::ATL_Regular);

        for (UObject* Asset : Assets)
        {
            if (UDataTable* DataTable = Cast<UDataTable>(Asset))
            {
                DialogueDatas.Add(DataTable);
            }
        }

        DialogueDatas.Sort([](const UDataTable& A, const UDataTable& B)
        {
            return A.GetName() < B.GetName();  // 이름(오름차순)으로 정렬
        });
    }
}

void UDialogueManager::Tick()
{
    if (bSpeechBuubbleVisible)
    {
        FVector SBLocation = SpeakingTarget->GetActorLocation() + FVector(0.f, 0.f, 95.f);
        SpeechBubble->SetActorLocation(SBLocation);                                     // 좌우 카메라 돌려도 항상 보이게
        SpeechBubble->SetActorRotation(GameManager->GetPlayer()->GetControlRotation() + FRotator(0.f, 180.f, 0.f));
    }
}

void UDialogueManager::CheckDialogueStartCondition()
{
    NPCManager->UpdateNPCPositions(DialogueNum);
}

void UDialogueManager::TriggerNextDialogue()
{
    if (!DialogueUI) return;

    if (DialogueUI->GetCurrentState() != 3 && !UIManager->IsMenuVisible())
    {
        if (DialogueUI->IsInputDisabled())
        {
            return;
        }
        else
        {
            DialogueUI->Interact();
        }
    }
}

void UDialogueManager::DisplayDialogueUI()
{
    if (DialogueUI)
    {
        if (!DialogueUI->CanStartDialogue())
        {
            FTimerHandle TimerHandle;
            GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UDialogueManager::DisplayDialogueUI, 1.f, false);
            return;
        }
           
        if (UIManager->IsControlGuideVisible())
        {
            UIManager->RemoveControlGuide();
        }

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
                    DialogueUI->InitializeDialogue(DialogueDatas[2]);
                    UIManager->SetIsFading(false);
                    break;
                case 4: // the boat move
                    DialogueUI->InitializeDialogue(DialogueDatas[2]);
                    break;
                case 5: // second dungeon
                    DialogueUI->InitializeDialogue(DialogueDatas[3]);
                    break;
                case 6: // triggerbox1 overplap
                case 7: // triggerbox2, player jump to plane
                case 8: // triggerbox3, player go over the other side
                case 9: // plane2 up
                case 10: // Npcs went over the other side
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
                    DialogueUI->InitializeDialogue(DialogueDatas[5]);
                    break;
                case 15: // boss level enter
                case 17: // be ready to combat with boss 
                    DialogueUI->InitializeDialogue(DialogueDatas[6]);
                    break;
                case 19: // back to cave
                case 21: // griffons come
                case 22: // last talk
                case 23:
                    DialogueUI->InitializeDialogue(DialogueDatas[7]);
                    break;
                default:
                    break;
            }
        }
        else // 두번째 던전에서 추락했을 때
        {
            uint8 FallCount = Player->GetFallCount();
            if (FallCount == 1) 
            {
                DialogueUI->InitializeDialogue(DialogueDatas.Last(0));
            }
        }

        UIManager->RemoveHUD();
        DialogueUI->SetVisibility(ESlateVisibility::Visible);
        bDialogueUIVisible = true;

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
            ++DialogueNum;
            DialogueEndEvents();
            if (DialogueNum > 2)
            {
                GameManager->SaveGame();
            }
        }
        else
        {
            Player->SetFallenInDungeon(false);
            Player->SetCanMove(true);
            if (bSpeechBuubbleVisible)
            {
                RemoveSpeechBuubble();
            }
        }

        bDialogueUIVisible = false;
        DialogueUI->OnAnimationHideMessageUI();

        FInputModeGameOnly InputModeGameOnly;
        MainPlayerController->SetInputMode(InputModeGameOnly);
        MainPlayerController->SetMouseCursorVisibility(false);
        
        UIManager->DisplayHUD();

        if (GameManager->IsSkipping())
        {
            GameManager->SetIsSkipping(false);
        }
    }
}

void UDialogueManager::DialogueEndEvents()
{
    RemoveSpeechBuubble();
    
    switch (DialogueNum)
    {
        case 1: // luko moves to player   
            NPCManager->GetNPC(ENPCType::Luko)->MoveToPlayer();
            break;
        case 2:
            if (UIManager->GetSystemMessageNum() != 4)
            {
                Player->SetTargetActor(nullptr);
                NPCManager->GetNPC(ENPCType::Luko)->ClearPlayerFollowTimer();
                NPCManager->MoveNPCToLocation(ENPCType::Luko, FVector(5200.f, 35.f, 100.f));
                UIManager->SetSystemMessage(2);

                FTimerHandle TimerHandle;
                GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateLambda([&]() {
                    UIManager->SetSystemMessage(3);
                    }), 2.5f, false);
                return;
            }
            MainPlayerController->SetCinematicMode(false, true, true);
            if (Player->GetEquippedWeapon() == nullptr) // get the wand
            {
                UIManager->SetSystemMessage(4);
            }
            break;
        case 3: // enter the first dungeon
            NPCManager->GetNPC(ENPCType::Luko)->SetTargetActor(nullptr);
            GameManager->SaveGame();
            UIManager->SetSystemMessage(6);
            GameManager->SetIsSkippable(true);
            break;
        case 4: // move to boat
            MainPlayerController->SetCinematicMode(false, true, true);
            NPCManager->GetNPC(ENPCType::Vovo)->SetTargetActor(nullptr);
            NPCManager->MoveNPCToLocation(ENPCType::Vovo, FVector(630.f, 970.f, 1840.f));
            Player->SetTargetActor(nullptr);
            {
                FTimerHandle TimerHandle;
                GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateLambda([&]() {
                    GameManager->SaveGame();
                    GameManager->SetIsSaveAllowed(false);
                    }), 3.f, false);
            }
            break;
        case 5:
            DialogueUI->ClearAutoDialogueTimer();
            break;
        case 6: // enter the second dungeon
            MainPlayerController->SetCinematicMode(false, true, true);
            NPCManager->GetNPC(ENPCType::Momo)->SetSmileStatus(false);
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
            if (NPCManager->GetNPC(ENPCType::Vivi)->GetNormalMontage() != nullptr)
            {
                NPCManager->GetNPC(ENPCType::Vivi)->GetAnimInstance()->Montage_Play(NPCManager->GetNPC(ENPCType::Vivi)->GetNormalMontage());
                NPCManager->GetNPC(ENPCType::Vivi)->GetAnimInstance()->Montage_JumpToSection(FName("Throw"));
            }
            Player->SetTargetActor(nullptr);
            break;
        case 10:
            NPCManager->GetNPC(ENPCType::Zizi)->SetTargetActor(nullptr);
            Player->SetTargetActor(nullptr);
            NPCManager->AllNpcMoveToPlayer();
            break;
        case 11: // npcs went over the other side
        case 19: // after combat with boss
            MainPlayerController->SetCinematicMode(false, true, true);
            Player->SetCanMove(true);
            Player->SetTargetActor(nullptr);
            NPCManager->AllNpcDisableLookAt();
            if (DialogueNum == 19)
            {
                NPCManager->MoveNPCToLocation(ENPCType::Momo, FVector(8.f, -3585.f, 684.f));
                NPCManager->MoveNPCToLocation(ENPCType::Luko, FVector(8.f, -3585.f, 684.f));
                NPCManager->MoveNPCToLocation(ENPCType::Vovo, FVector(8.f, -3585.f, 684.f));
                NPCManager->MoveNPCToLocation(ENPCType::Vivi, FVector(8.f, -3585.f, 684.f));
                NPCManager->MoveNPCToLocation(ENPCType::Zizi, FVector(8.f, -3585.f, 684.f));
                UIManager->SetSystemMessage(14);
            }
            if (DialogueNum == 11)
            {
                for (auto NPC : NPCManager->GetNPCMap())
                {
                    NPC.Value->SetSmileStatus(false);
                }
            }
            break;
        case 12:
            MainPlayerController->SetCinematicMode(false, true, true);
            Player->SetCanMove(true);
            MainPlayerController->ResetIgnoreMoveInput();
            MainPlayerController->ResetIgnoreLookInput();
           DialogueUI->ClearAutoDialogueTimer();
            break;
        case 13:
        case 14:
            NPCManager->GetNPC(ENPCType::Momo)->SetSmileStatus(false);
            Player->SetCanMove(true);

            NPCManager->AllNpcMoveToPlayer();
            break;
        case 15:
            DialogueUI->SetInputDisabled(false);
            Player->SetCanMove(true);
            UIManager->SetSystemMessage(12);
            NPCManager->AllNpcMoveToPlayer();
            break;
        case 16:
            MainPlayerController->SetCinematicMode(false, true, true);
            NPCManager->AllNpcDisableLookAt();
            Player->SetTargetActor(nullptr);
            DialogueUI->SetInputDisabled(false);
            break;
        case 17:
        case 22:
        case 23:
            DialogueUI->SetInputDisabled(false);
            MainPlayerController->SetCinematicMode(false, true, true);
            Player->SetCanMove(true);
            if (DialogueNum == 23)
                NPCManager->MoveNPCToLocation(ENPCType::Vivi, FVector(625.f, 318.f, 153.f));
            break;
        case 18: // fog appear, boss combat soon
        {
            DialogueUI->SetInputDisabled(false);
            Player->SetCanMove(true);
            NPCManager->AllNpcMoveToPlayer();
            UIManager->SetSystemMessage(13);
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
            DialogueUI->SetInputDisabled(false);
            if (DialogueUI->GetSelectedReply() == 1)
            {
                MainPlayerController->SetCinematicMode(false, true, true);
                Player->SetCanMove(true);
                UIManager->SetSystemMessage(15);
            }
            break;
        default:
            break;
    }
}

void UDialogueManager::DisplaySpeechBuubble(class AYaroCharacter* NPC)
{
    if(!SpeechBubble)
    {
        TSoftClassPtr<AActor> SpeechBubbleBPClass(FSoftObjectPath(TEXT("/Game/Blueprints/SpeechBubble_BP.SpeechBubble_BP_C")));
        if (!SpeechBubbleBPClass.IsValid())
        {
            SpeechBubbleBPClass.LoadSynchronous();
        }

        if (ensure(SpeechBubbleBPClass.IsValid()))
        {
            SpeechBubble = Player->GetWorld()->SpawnActor<AActor>(SpeechBubbleBPClass.Get());
        }
    }

    if (SpeechBubble)
    {
        SpeakingTarget = NPC;
        SpeechBubble->SetActorHiddenInGame(false);
        bSpeechBuubbleVisible = true;
    }
}

void UDialogueManager::RemoveSpeechBuubble()
{
    if (SpeechBubble)
    {
        bSpeechBuubbleVisible = false;
        SpeechBubble->SetActorHiddenInGame(true);
    }
}
