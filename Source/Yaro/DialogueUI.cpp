﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "DialogueUI.h"
#include "Components/TextBlock.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h"

#include "Yaro/System/GameManager.h"
#include "Yaro/System/DialogueManager.h"
#include "Yaro/System/NPCManager.h"
#include "Yaro/System/UIManager.h"
#include "Yaro/System/MainPlayerController.h"
#include "Yaro/Character/Main.h"
#include "Yaro/Character/YaroCharacter.h"

void UDialogueUI::NativeConstruct()
{
    Super::NativeConstruct();

    GameManager = Cast<UGameManager>(GetWorld()->GetGameInstance());
    if (GameManager)
    {
        DialogueManager = GameManager->GetDialogueManager();
        NPCManager = GameManager->GetNPCManager();
        Player = GameManager->GetPlayer();
        MainPlayerController = GameManager->GetMainPlayerController();
    }

    if (NPCManager)
    {
        Momo = NPCManager->GetNPC(ENPCType::Momo);
        Luko = NPCManager->GetNPC(ENPCType::Luko);
        Vovo = NPCManager->GetNPC(ENPCType::Vovo);
        Vivi = NPCManager->GetNPC(ENPCType::Vivi);
        Zizi = NPCManager->GetNPC(ENPCType::Zizi);
    }
}

void UDialogueUI::SetMessage(const FString& Text)
{
    if (NPCText == nullptr)
    {
        return;
    }

    NPCText->SetText(FText::FromString(Text));
}

void UDialogueUI::SetCharacterName(const FString& Text)
{
    if (CharacterNameText == nullptr)
    {
        return;
    }

    CharacterNameText->SetText(FText::FromString(Text));
}

void UDialogueUI::AnimateMessage(const FString& Text)
{
    CurrentState = 1;
    InitialMessage = Text;
    OutputMessage = "";
    iLetter = 0;

    NPCText->SetText(FText::FromString(""));
    CharacterNameText->SetText(FText::FromString(Dialogue[RowIndex]->CharacterName.ToString()));

    ActivateSpeechBubble();

    GetWorld()->GetTimerManager().SetTimer(TextTimer, this, &UDialogueUI::OnAnimationTimerCompleted, 0.2f, false);
}

void UDialogueUI::OnAnimationTimerCompleted()
{
    GetWorld()->GetTimerManager().ClearTimer(TextTimer);

    OutputMessage.AppendChar(InitialMessage[iLetter]);

    NPCText->SetText(FText::FromString(OutputMessage));

    if (TextSound != nullptr)
    {
        UAudioComponent* AudioComponent = UGameplayStatics::SpawnSound2D(this, TextSound);
    }

    if ((iLetter + 1) < InitialMessage.Len())
    {
        iLetter += 1;
        GetWorld()->GetTimerManager().SetTimer(TextTimer, this, &UDialogueUI::OnAnimationTimerCompleted, DelayBetweenLetters, false);
    }
    else
    {
        NPCManager->CloseAllMouth();
        CurrentState = 2;
    }
}

void UDialogueUI::InitializeDialogue(UDataTable* DialogueTable)
{
    CurrentState = 0;

    CharacterNameText->SetText(FText::FromString(""));
    NPCText->SetText(FText::FromString(""));

    OnResetOptions();

    Dialogue.Empty();

    for (auto it : DialogueTable->GetRowMap())
    {
        FNPCDialogue* Row = (FNPCDialogue*)it.Value;
        if (Row)
        {
            Dialogue.Add(Row);
        }
    }


    if (Dialogue.Num() > 0)
    {
        RowIndex = 0;

        if (Dialogue[RowIndex]->Messages.Num() > 0)
        {
            MessageIndex = 0;

            OnAnimationShowMessageUI();
            DialogueEvents();
        }
    }
}

void UDialogueUI::Interact()
{
    if (CurrentState == 1) // The text is being animated, skip
    {
        GetWorld()->GetTimerManager().ClearTimer(TextTimer);
        NPCText->SetText(FText::FromString(InitialMessage));
        NPCManager->CloseAllMouth();

        CurrentState = 2;
    }
    else if (CurrentState == 2) // Text completed
    {
        // Get next message
        if ((MessageIndex + 1) < Dialogue[RowIndex]->Messages.Num()) // 같은 npc의 다음 대사
        {
            MessageIndex += 1;
            DialogueEvents();
        }
        else
        {
            NPCText->SetText(FText::FromString(""));

            if (Dialogue[RowIndex]->PlayerReplies.Num() > 0) // 플레이어 응답 있으면
            {
                OnResetOptions();
                NumOfReply = Dialogue[RowIndex]->PlayerReplies.Num();
                SelectedReply = 0;

                for (int i = 0; i < Dialogue[RowIndex]->PlayerReplies.Num(); i++)
                {
                    OnSetOption(i, Dialogue[RowIndex]->PlayerReplies[i].ReplyText);
                }

                CurrentState = 3;
            }
            else // 플레이어의 응답이 존재하지 않으면
            {
                RowIndex += 1;

                if ((RowIndex >= 0) && (RowIndex < Dialogue.Num())) // 다음 npc 대사
                {
                    MessageIndex = 0;
                    DialogueEvents();
                }
                else
                {
                    bCanStartDialogue = false;
                    DialogueManager->RemoveDialogueUI();
                    CurrentState = 0;
                }
            }
        }
    }
    else if (CurrentState == 3) // 플레이어 응답 선택한 상태
    {
        // 플레이어 응답에 따라 RowIndex 바뀜
        int Index = SelectedReply - 1; // SelectedReply는 1,2,3
        RowIndex = Dialogue[RowIndex]->PlayerReplies[Index].AnswerIndex;
        OnResetOptions();

        if ((RowIndex >= 0) && (RowIndex < Dialogue.Num()))
        {
            NPCText->SetText(FText::FromString(""));

            MessageIndex = 0;
            DialogueEvents();
        }
        else
        {
            bCanStartDialogue = false;

            CurrentState = 0;
            DialogueManager->RemoveDialogueUI();
        }
    }
}

void UDialogueUI::DialogueEvents()
{
    int DNum = DialogueManager->GetDialogueNum();
    FTimerManager& TimerManager = GetWorld()->GetTimerManager();

    if (Player->IsFallenInDungeon())
    {
        if (Player->GetFallCount() == 1)
        {
            bInputDisabled = false;

            if (RowIndex == 0)
            {
                NPCManager->AllNpcLookAtPlayer();
            }

            if (RowIndex == 7)
            {
                AutoCloseDialogue();
                NPCManager->AllNpcDisableLookAt();
                NPCManager->AllNpcMoveToPlayer();
                Player->SetFallenInDungeon(false);
                return;
            }
        }
        else return;
    }
    else
    {
        if (DNum == 0) // First Dialogue (cave)
        {
            if (RowIndex < 10 && Player->GetCameraBoom()->TargetArmLength > 0) // 1인칭시점이 아닐 때
            {
                AnimateMessage(Dialogue[RowIndex]->Messages[MessageIndex].ToString());

                if (RowIndex == 0 && MessageIndex == 2)
                {
                    GameManager->GetUIManager()->SetSystemMessage(1);
                }

                switch (RowIndex)
                {
                    case 1:
                        if (MessageIndex > 0)
                        {
                            DialogueManager->DisplaySpeechBuubble(Momo);
                        }
                        break;
                    case 7:
                        DialogueManager->DisplaySpeechBuubble(Momo);
                        break;
                    case 2:
                    case 6:
                        DialogueManager->DisplaySpeechBuubble(Vivi);
                        break;
                    case 3:
                    case 8:
                        DialogueManager->DisplaySpeechBuubble(Luko);
                        break;
                    case 4:
                        DialogueManager->DisplaySpeechBuubble(Zizi);
                        break;
                    case 5:
                    case 9:
                        DialogueManager->DisplaySpeechBuubble(Vovo);
                        break;
                    default:
                        break;
                }
                return;
            }

            switch (RowIndex) // 1인칭 시점일 때
            {
                case 1: // Momo, Set GetFollowCamera()'s Z value of Rotation
                    NPCManager->OpenMouth(Momo);
                    DialogueManager->DisplaySpeechBuubble(Momo);
                case 7:
                    Player->GetFollowCamera()->SetRelativeRotation(FRotator(0.f, -15.f, 0.f));
                    bInputDisabled = false;
                    break;
                case 2: // Vivi
                    NPCManager->OpenMouth(Vivi);
                    DialogueManager->DisplaySpeechBuubble(Vivi);
                case 6:
                    Player->GetFollowCamera()->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
                    break;
                case 3: // Luko
                    NPCManager->OpenMouth(Luko);
                    DialogueManager->DisplaySpeechBuubble(Luko);
                case 8:
                    Player->GetFollowCamera()->SetRelativeRotation(FRotator(0.f, -25.f, 0.f));
                    break;
                case 4: // Zizi
                    NPCManager->OpenMouth(Zizi);
                    DialogueManager->DisplaySpeechBuubble(Zizi);
                    Player->GetFollowCamera()->SetRelativeRotation(FRotator(0.f, 13.f, 0.f));
                    break;
                case 5: // Vovo
                case 9:
                    NPCManager->OpenMouth(Vovo);
                    DialogueManager->DisplaySpeechBuubble(Vovo);
                    Player->GetFollowCamera()->SetRelativeRotation(FRotator(0.f, 30.f, 0.f));
                    if (RowIndex == 5 && MessageIndex == 2)
                    {
                        Player->GetFollowCamera()->SetRelativeRotation(FRotator(0.f, 5.f, 0.f));
                    }
                    break;
                case 10: // npc go
                    Player->GetFollowCamera()->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
                    Player->GetCameraBoom()->TargetArmLength = 500.f;
                    DialogueManager->RemoveSpeechBuubble();

                    // npc move except luko
                    Momo->GetAIController()->MoveToLocation(FVector(5200.f, 35.f, 100.f));
                    Vovo->GetAIController()->MoveToLocation(FVector(5200.f, 35.f, 100.f));
                    Vivi->GetAIController()->MoveToLocation(FVector(5200.f, 35.f, 100.f));
                    Zizi->GetAIController()->MoveToLocation(FVector(5200.f, 35.f, 100.f));
                    bInputDisabled = true;
                    TimerManager.SetTimer(AutoDialogueTimer, this, &UDialogueUI::Interact, 2.f, false);
                    break;
                case 11:
                    Player->SetCanMove(false);
                    {
                        FTimerHandle Timer;
                        TimerManager.SetTimer(Timer, FTimerDelegate::CreateLambda([&]()
                        {
                            DialogueManager->DisplayDialogueUI();
                            Player->SetTargetActor(Luko);
                            TimerManager.ClearTimer(Timer);
                        }), 2.f, false); // 2초 뒤 루코 대화
                    }
                    AutoCloseDialogue();
                    return;
                    break;
                default:
                    break;
            }
        }

        if (DNum == 1) // 루코 대화
        {
            if (RowIndex == 0)
            {
                bInputDisabled = false;
                RowIndex = 11;
                Player->GetCameraBoom()->TargetArmLength = 500.f;
            }
        }

        if (DNum == 2) // enter first dungeon
        {
            switch (RowIndex)
            {
                case 4:
                    Luko->SetTargetActor(Player);
                    break;
                default:
                    break;
            }
        }

        if (DNum == 3) // Third Dialogue (first dungeon, after golem battle)
        {
            switch (RowIndex)
            {
                case 0:
                    Player->GetCameraBoom()->TargetArmLength = 200.f;
                case 2:
                    if (MessageIndex == 1)
                    {
                        // npc move to the boat except vovo
                        NPCManager->SetAllNpcMovementSpeed(false);
                        Momo->GetAIController()->MoveToLocation(FVector(660.f, 1035.f, 1840.f));
                        Luko->GetAIController()->MoveToLocation(FVector(598.f, 1030.f, 1840.f));
                        Vivi->GetAIController()->MoveToLocation(FVector(710.f, 995.f, 1840.f));
                        Zizi->GetAIController()->MoveToLocation(FVector(690.f, 930.f, 1840.f));
                        DialogueManager->RemoveSpeechBuubble();
                    }
                    break;
                case 3: // vovo look at player and player look at vovo
                    Vovo->SetTargetActor(Player);
                    Player->SetTargetActor(Vovo);
                    break;
                case 5:
                    if (SelectedReply == 1)
                    {
                        RowIndex = 7;
                    }
                    break;
                case 6:
                    if (SelectedReply == 2)
                    {
                        AutoCloseDialogue();
                        return;
                    }
                    break;
                case 8:
                    // vovo moves to the boat
                    if (SelectedReply == 1 || SelectedReply == 3)
                    {
                        AutoCloseDialogue();
                        return;
                    }
                    break;
                default:
                    break;
            }
        }

        if (DNum == 4) // Third Dialogue (first dungeon, boat moves)
        {
            switch (RowIndex)
            {
                case 0:
                    for (const TPair<FString, AYaroCharacter*>& NPC : NPCManager->GetNPCMap())
                    {
                        NPC.Value->GetAIController()->StopMovement();
                    }
                    RowIndex = 8;
                    bInputDisabled = true;
                case 9:
                case 10:
                case 11:
                    TimerManager.SetTimer(AutoDialogueTimer, this, &UDialogueUI::Interact, 2.5f, false);
                    break;
                default:
                    break;
            }
        }

        if (DNum == 5) // enter the second dungeon
        {
            switch (RowIndex)
            {
                case 0:
                    GameManager->SetIsSaveAllowed(true);
                    break;
                case 2:
                    Momo->SetTargetActor(Vivi);
                    Zizi->SetTargetActor(Vivi);
                    Player->SetTargetActor(Vivi);
                    break;
                case 6:
                    if (MessageIndex == 2)
                    {
                        Zizi->SetTargetActor(Player);
                        Player->SetTargetActor(Zizi);
                    }
                    break;
                case 7:
                    Momo->SetTargetActor(Player);
                    Vivi->SetTargetActor(Player);
                    Luko->SetTargetActor(Player);
                    Vovo->SetTargetActor(Player);
                    Player->SetTargetActor(Momo);
                    break;
                case 8:
                    Momo->SetSmileStatus(true);
                    break;
                case 9:
                    Momo->SetSmileStatus(false);
                case 10:
                    Zizi->SetTargetActor(Vivi);
                    break;
                case 13:
                    if (MessageIndex == 0)
                    {
                        Momo->SetSmileStatus(true);
                        Momo->MoveToPlayer();
                    }
                    else
                    {
                        NPCManager->AllNpcDisableLookAt();
                        Player->SetTargetActor(nullptr);
                    }
                    break;
                default:
                    break;
            }
        }

        if (DNum == 6)
        {
            Player->SetCanMove(false);
            if (RowIndex == 4)
            {
                AutoCloseDialogue();
                return;
            }
        }

        if (DNum == 7)
        {
            switch (RowIndex)
            {
                case 0: // player jump to the plane
                    RowIndex = 4;
                    Momo->GetAIController()->MoveToLocation(FVector(5307.f, -3808.f, -2122.f));
                    Luko->GetAIController()->MoveToLocation(FVector(5239.f, -3865.f, -2117.f));
                    Vovo->GetAIController()->MoveToLocation(FVector(5433.f, -3855.f, -2117.f));
                    Vivi->GetAIController()->MoveToLocation(FVector(5392.f, -3686.f, -2117.f));
                    Zizi->GetAIController()->MoveToLocation(FVector(5538.f, -3696.f, -2115.f));
                    break;
                case 6: // zizi says
                    bInputDisabled = false;
                    Player->SetCanMove(true);
                    AutoCloseDialogue();
                    return;
                    break;
                default:
                    break;
            }
        }

        if (DNum == 8) // player go over the side
        {
            switch (RowIndex)
            {
                case 0:
                {
                    RowIndex = 6;
                    NPCManager->AllNpcLookAtPlayer();
                    NPCManager->AllNpcStopFollowPlayer();
                    Momo->GetAIController()->MoveToLocation(FVector(5307.f, -3808.f, -2122.f));
                    Luko->GetAIController()->MoveToLocation(FVector(5239.f, -3865.f, -2117.f));
                    Vovo->GetAIController()->MoveToLocation(FVector(5433.f, -3855.f, -2117.f));
                    Vivi->GetAIController()->MoveToLocation(FVector(5392.f, -3686.f, -2117.f));
                    Zizi->GetAIController()->MoveToLocation(FVector(5538.f, -3696.f, -2115.f));
                    FTimerHandle Timer;
                    TimerManager.SetTimer(Timer, FTimerDelegate::CreateLambda([&]()
                    {
                        Player->SetTargetActor(Vivi);
                        TimerManager.ClearTimer(Timer);
                    }), 0.6f, false); // npc쪽 쳐다보기
                    break;
                }
                case 10:
                    Player->SetCanMove(true);
                    AutoCloseDialogue();
                    return;
                    break;
                default:
                    break;
            }
        }

        if (DNum == 9) // plane2 up
        {
            switch (RowIndex)
            {
                case 0:
                    Player->SetCanMove(false);
                    RowIndex = 10;
                    Player->SetTargetActor(Vivi);
                    break;
                case 11:
                    Zizi->SetTargetActor(Vivi);
                    break;
                case 12:
                    if (MessageIndex == 1)
                    {
                        bInputDisabled = true;
                        TimerManager.SetTimer(AutoDialogueTimer, this, &UDialogueUI::Interact, 1.f, false);
                    }
                    break;
                case 13:
                    AutoCloseDialogue();
                    bInputDisabled = false;
                    return;
                    break;
                default:
                    break;
            }
        }

        if (DNum == 10)
        {
            switch (RowIndex)
            {
                case 0:
                    Momo->SetSmileStatus(true);
                    RowIndex = 13;
                    Player->SetTargetActor(Momo);
                    Momo->SetTargetActor(Player);
                    break;
                case 14:
                    Vovo->SetSmileStatus(true);
                    Vovo->SetTargetActor(Player);
                    Player->SetTargetActor(Vovo);
                    break;
                case 15:
                    Vivi->SetSmileStatus(true);
                    Vivi->SetTargetActor(Player);
                    Player->SetTargetActor(Vivi);
                    break;
                case 16:
                    Zizi->SetSmileStatus(true);
                    Zizi->SetTargetActor(Player);
                    Player->SetTargetActor(Zizi);
                    break;
                default:
                    break;
            }
        }

        if (DNum == 11) // food table event
        {
            switch (RowIndex)
            {
                case 2:
                    Vivi->SetActorRotation(FRotator(0.f, 0.f, 0.f));
                    Luko->SetTargetActor(Vivi);
                    Vovo->SetTargetActor(Vivi);
                    Zizi->SetTargetActor(Vivi);
                    Player->GetCameraBoom()->TargetArmLength = 290.f;
                    MainPlayerController->SetViewTargetWithBlend(Player, 1.f); // 플레이어 카메라로 복귀
                    break;
                case 4:
                    NPCManager->SetAllNpcMovementSpeed(true);
                    Momo->GetAIController()->MoveToLocation(FVector(-260.f, -1900.f, -121.f)); //momo move to the table
                    break;
                case 5: // 모모 방향으로 카메라 돌리기
                    DialogueManager->RemoveSpeechBuubble();
                    Player->GetFollowCamera()->SetRelativeRotation((FRotator(0.f, 330.f, 0.f)));
                    Player->GetCameraBoom()->SocketOffset = FVector(0.f, -750.f, 70.f);
                    Player->GetCameraBoom()->TargetArmLength = -10.f;

                    bInputDisabled = true;
                    TimerManager.SetTimer(AutoDialogueTimer, this, &UDialogueUI::Interact, 2.f, false);
                    break;
                case 6://다시 카메라 플레이어쪽으로 돌리기
                    bInputDisabled = false;
                    Player->GetFollowCamera()->SetRelativeRotation((FRotator(0.f, 0.f, 0.f)));
                    Player->GetCameraBoom()->SocketOffset = FVector(0.f, 0.f, 70.f);
                    Player->GetCameraBoom()->TargetArmLength = 290.f;
                    break;
                case 8:
                    NPCManager->AllNpcDisableLookAt();
                    if (ExplosionSound != nullptr)
                    {
                        UAudioComponent* AudioComponent = UGameplayStatics::SpawnSound2D(this, ExplosionSound);
                    }
                    bInputDisabled = true;
                    TimerManager.SetTimer(AutoDialogueTimer, this, &UDialogueUI::Interact, 0.6f, false);
                    break;
                case 9: //npc이동 지지 제외
                    NPCManager->SetAllNpcMovementSpeed(true);
                    Luko->GetAIController()->MoveToLocation(FVector(-198.f, -2065.f, -118.f));
                    Vivi->GetAIController()->MoveToLocation(FVector(-347.f, -2040.f, -121.f));
                    Vovo->GetAIController()->MoveToLocation(FVector(-248.f, -2141.f, -118.f));
                    TimerManager.SetTimer(AutoDialogueTimer, this, &UDialogueUI::Interact, 1.5f, false);
                    break;
                case 10:
                    Zizi->GetAIController()->MoveToLocation(FVector(-415.f, -2180.f, -116.f));
                    AutoCloseDialogue();
                    bInputDisabled = false;
                    DialogueManager->RemoveSpeechBuubble();
                    return;
                    break;
                default:
                    break;
            }
        }

        if (DNum == 12) // all spider died
        {
            if (RowIndex == 0)
            {
                Momo->SetSmileStatus(true);
                Player->SetCanMove(false);
                RowIndex = 10;
                Momo->SetTargetActor(Player);
                for (auto NPC : NPCManager->GetNPCMap())
                {
                    if (!NPC.Key.Contains("Momo"))
                    {
                        NPC.Value->SetTargetActor(Momo);
                    }
                }
                Player->SetTargetActor(Momo);
            }
            if (RowIndex == 12)
            {
                NPCManager->AllNpcDisableLookAt();
                Player->SetTargetActor(nullptr);
                AutoCloseDialogue();
                return;
            }
        }

        if (DNum == 13)
        {
            switch (RowIndex)
            {
                case 0:
                    Player->SetCanMove(false);
                    RowIndex = 12;
                    Vivi->SetTargetActor(Player);
                    NPCManager->AllNpcStopFollowPlayer();
                    for (auto NPC : NPCManager->GetNPCMap())
                    {
                        if (!NPC.Key.Contains("Vivi"))
                        {
                            NPC.Value->SetTargetActor(Momo);
                        }
                    }
                    Player->SetTargetActor(Vivi);
                    break;
                case 14:
                    Vovo->SetTargetActor(Momo);
                    break;
                case 15:
                    if(MessageIndex == 1) 
                    {
                        Momo->SetTargetActor(Player);
                    }
                    break;
                case 16:
                    Zizi->SetSmileStatus(true);
                    Zizi->SetTargetActor(Player);
                    Player->SetTargetActor(Zizi);
                    break;
                case 17:
                    Player->SetCanMove(true);
                    NPCManager->AllNpcDisableLookAt();
                    Player->SetTargetActor(nullptr);
                    AutoCloseDialogue();
                    Zizi->SetSmileStatus(false);
                    return;
                    break;
                default:
                    break;
            }
        }

        if (DNum == 14)
        {
            if (RowIndex == 0)
            {
                RowIndex = 17;
                Player->SetCanMove(false);
            }
        }

        if (DNum == 15)
        {
            switch (RowIndex)
            {
                case 2:
                    if (MessageIndex == 1)
                    {
                        Momo->SetTargetActor(Player);
                        Player->SetTargetActor(Momo);
                    }
                    break;
                case 3:
                    Luko->SetTargetActor(Momo);
                    break;
                case 4:
                    Vivi->SetTargetActor(Player);
                    Player->SetTargetActor(Vivi);
                    break;
                case 5:
                    bInputDisabled = true;
                    NPCManager->AllNpcDisableLookAt();
                    TimerManager.SetTimer(AutoDialogueTimer, this, &UDialogueUI::Interact, 1.5f, false);
                    break;
                case 6:
                    if (SelectedReply == 1)
                    {
                        DialogueManager->RemoveSpeechBuubble();
                        NPCManager->SetAllNpcMovementSpeed(false);
                        Vivi->GetAIController()->MoveToLocation(FVector(105.f, 3176.f, 182.f));
                        Momo->GetAIController()->MoveToLocation(FVector(-86.f, 3263.f, 177.f));
                        Luko->GetAIController()->MoveToLocation(FVector(184.f, 3317.f, 182.f));
                        Vovo->GetAIController()->MoveToLocation(FVector(-140.f, 3370.f, 182.f));
                        Zizi->GetAIController()->MoveToLocation(FVector(68.f, 3398.f, 184.f));
                        AutoCloseDialogue();
                        return;
                    }
                    else if (MessageIndex == 1)
                    {
                        bInputDisabled = true;
                        TimerManager.SetTimer(AutoDialogueTimer, this, &UDialogueUI::Interact, 2.f, false);
                    }
                    break;
                case 7:
                    if (SelectedReply == 2)
                    {
                        DialogueManager->RemoveSpeechBuubble();
                        NPCManager->SetAllNpcMovementSpeed(false);
                        Vivi->GetAIController()->MoveToLocation(FVector(105.f, 3176.f, 182.f));
                        Momo->GetAIController()->MoveToLocation(FVector(-86.f, 3263.f, 177.f));
                        Luko->GetAIController()->MoveToLocation(FVector(184.f, 3317.f, 182.f));
                        Vovo->GetAIController()->MoveToLocation(FVector(-140.f, 3370.f, 182.f));
                        Zizi->GetAIController()->MoveToLocation(FVector(68.f, 3398.f, 184.f));
                        AutoCloseDialogue();
                        return;
                    }
                    break;
                default:
                    break;
            }
        }

        if (DNum == 16) // 돌 치우기
        {
            switch (RowIndex)
            {
                case 0:
                    Player->SetCanMove(false);
                    RowIndex = 7;
                    break;
                case 8:
                    Momo->SetTargetActor(Zizi);
                    Luko->SetTargetActor(Zizi);
                    Vovo->SetTargetActor(Zizi);
                    Vivi->SetTargetActor(Zizi);
                    Player->SetTargetActor(Zizi);
                    break;
                case 10:
                    NPCManager->SetAllNpcMovementSpeed(false);
                    Zizi->GetAIController()->MoveToLocation(FVector(18, 3090, 182.f));
                    bInputDisabled = true;

                    TimerManager.SetTimer(AutoDialogueTimer, this, &UDialogueUI::Interact, 1.5f, false);
                    break;
                case 11:
                    Zizi->SetActorRotation(FRotator(0.f, 260.f, 0.f));
                    if (Zizi->GetCombatMontage() != nullptr)
                    {
                        Zizi->GetAnimInstance()->Montage_Play(Zizi->GetCombatMontage());
                        Zizi->GetAnimInstance()->Montage_JumpToSection(FName("Attack"));
                    }
                    bInputDisabled = true;
                    TimerManager.SetTimer(AutoDialogueTimer, this, &UDialogueUI::Interact, 1.6f, false);
                    break;
                case 12:
                    bInputDisabled = false;
                    break;
                case 15:
                    Vovo->SetSmileStatus(true);
                    break;
                case 16:
                    bInputDisabled = true;
                    TimerManager.SetTimer(AutoDialogueTimer, this, &UDialogueUI::Interact, 1.4f, false);
                    break;
                case 17:
                    Vovo->SetSmileStatus(false);
                    Player->SetTargetActor(nullptr);
                    NPCManager->AllNpcDisableLookAt();
                    NPCManager->SetAllNpcMovementSpeed(false);
                    Vivi->GetAIController()->MoveToLocation(FVector(100.f, 1997.f, 182.f));
                    Momo->GetAIController()->MoveToLocation(FVector(-86.f, 2150.f, 177.f));
                    Luko->GetAIController()->MoveToLocation(FVector(171.f, 2130.f, 182.f));
                    Vovo->GetAIController()->MoveToLocation(FVector(-160.f, 2060.f, 182.f));
                    Zizi->GetAIController()->MoveToLocation(FVector(18.f, 2105.f, 184.f));
                    AutoCloseDialogue();
                    return;
                    break;
                default:
                    break;
            }
        }

        if (DNum == 17) // 돌 치우고 건너간 뒤
        {
            switch (RowIndex)
            {
                case 0:
                    Player->SetCanMove(false);
                    RowIndex = 17;
                    Vivi->SetTargetActor(Player);
                    break;
                case 18:
                    if (MessageIndex == 0) DialogueManager->RemoveSpeechBuubble();
                    Luko->SetTargetActor(Momo);
                    Vovo->SetTargetActor(Momo);
                    Zizi->SetTargetActor(Momo);
                    Momo->GetAIController()->MoveToLocation(FVector(-86.f, 2307.f, 177.f));
                    break;
                case 20:
                    Zizi->SetTargetActor(Player);
                    break;
                case 21:
                    NPCManager->AllNpcDisableLookAt();
                    AutoCloseDialogue();
                    return;
                    break;   
                default:
                    break;
            }
        }

        if (DNum == 18) // 보스 전투 끝
        {
            switch (RowIndex)
            {
                case 0:
                    if (AfterAllCombatBGM != nullptr)
                    {
                        UAudioComponent* AudioComponent = UGameplayStatics::SpawnSound2D(this, AfterAllCombatBGM);
                    }
                    Player->GetCameraBoom()->TargetArmLength = 170.f;
                    RowIndex = 21;
                    break;
                case 23:
                    Player->SetTargetActor(Vovo);
                case 24:
                    if (MessageIndex == 0)
                    {
                        Zizi->SetTargetActor(Vovo);
                    }
                    else
                    {
                        Zizi->SetTargetActor(Vivi);
                    }
                    break;
                case 25:
                    Vovo->SetTargetActor(Vivi);
                    Player->SetTargetActor(Vivi);
                    break;
                case 26:
                    Momo->SetTargetActor(Vivi);
                    break;
                default:
                    break;
            }
        }

        if (DNum == 19) // 동굴 복귀
        {
            switch (RowIndex)
            {
                case 3:
                    DialogueManager->RemoveSpeechBuubble();
                    NPCManager->SetAllNpcMovementSpeed(false);
                    Momo->GetAIController()->MoveToLocation(FVector(-4660.f, 118.f, 393.f));
                    Luko->GetAIController()->MoveToLocation(FVector(-4545.f, -241.f, 401.f));
                    Vovo->GetAIController()->MoveToLocation(FVector(-4429.f, 103.f, 396.f));
                    Vivi->GetAIController()->MoveToLocation(FVector(-4355.f, -195.f, 405.f));
                    Zizi->GetAIController()->MoveToLocation(FVector(-4695.f, -190.f, 394.f));
                    AutoCloseDialogue();
                    return;
                    break;
                default:
                    break;
            }
        }

        if (DNum == 20) // 돌 발견, 챙기기
        {
            switch (RowIndex)
            {
                case 0:
                    RowIndex = 3;
                    Player->GetCameraBoom()->TargetArmLength = 470.f;
                    break;
                case 9:
                    if (SelectedReply == 1)
                    {
                        NPCManager->AllNpcLookAtPlayer();
                    }
                    break;
                case 10:
                    if (SelectedReply == 1)
                    {
                        MainPlayerController->SetControlRotation(FRotator(0.f, 180.f, 0.f));
                        AutoCloseDialogue();
                        return;
                    }
                    else if (SelectedReply == 2)
                    {
                        bInputDisabled = true;
                        TimerManager.SetTimer(AutoDialogueTimer, this, &UDialogueUI::Interact, 1.5f, false);
                    }
                    break;
                case 11:
                    NPCManager->SetAllNpcMovementSpeed(false);
                    Vivi->GetAIController()->MoveToLocation(FVector(-4520.f, -50.f, 409.f));
                    AutoCloseDialogue();
                    return;
                    break;
                default:
                    break;
            }
        }

        if (DNum == 21) // 돌 챙기고 난 뒤
        {
            switch (RowIndex)
            {
                case 0:
                    NPCManager->AllNpcDisableLookAt();
                    RowIndex = 11;
                    break;
                case 12:
                    if (GriffonSound != nullptr)
                    {
                        UAudioComponent* AudioComponent = UGameplayStatics::SpawnSound2D(this, GriffonSound);
                    }
                    SpawnGriffon();
                    break;
                case 13:
                    bInputDisabled = true;
                    TimerManager.SetTimer(AutoDialogueTimer, this, &UDialogueUI::Interact, 1.5f, false);
                    break;
                case 14:
                    DialogueManager->RemoveSpeechBuubble();
                    NPCManager->SetAllNpcMovementSpeed(true);
                    Momo->GetAIController()->MoveToLocation(FVector(508.f, 120.f, 100.f));
                    Luko->GetAIController()->MoveToLocation(FVector(311.f, -78.f, 103.f));
                    Vovo->GetAIController()->MoveToLocation(FVector(469.f, -22.f, 103.f));
                    Vivi->GetAIController()->MoveToLocation(FVector(267.f, 65.f, 101.f));
                    Zizi->GetAIController()->MoveToLocation(FVector(591.f, 28.f, 104.f));
                    AutoCloseDialogue();
                    return;
                    break;
                default:
                    break;
            }
            
        }

        if (DNum == 22) // 그리폰 탈 차례
        {
            switch (RowIndex)
            {
                case 0:
                    RowIndex = 14;
                    break;
                case 16:
                    Zizi->SetSmileStatus(true);
                    NPCManager->AllNpcLookAtPlayer();
                    break;
                case 17:
                    Momo->SetSmileStatus(true);
                    break;
                case 18:
                    Vovo->SetSmileStatus(true);
                    break;
                case 19:
                    Vivi->SetSmileStatus(true);
                    break;
                case 20:
                    if (MessageIndex == 1)
                    {
                        Luko->SetSmileStatus(true);
                    }
                    break;
                case 22:
                    NPCManager->AllNpcDisableLookAt();
                    if (MessageIndex == 1)
                    {
                        NPCManager->SetAllNpcMovementSpeed(false);
                        Zizi->GetAIController()->MoveToLocation(FVector(2560.f, 320.f, 157.f));
                        Vovo->GetAIController()->MoveToLocation(FVector(2570.f, 325.f, 154.f));
                        Luko->GetAIController()->MoveToLocation(FVector(1517.f, 325.f, 155.f));
                        Momo->GetAIController()->MoveToLocation(FVector(1530.f, 330.f, 150.f));
                    }
                    break;
                case 23:
                    AutoCloseDialogue();
                    return;
                    break;
                default:
                    break;
            }
        }

        if (DNum == 23)
        {
            DialogueManager->RemoveSpeechBuubble();
            RowIndex = 23;
        }
    }
   
    AnimateMessage(Dialogue[RowIndex]->Messages[MessageIndex].ToString());
}

void UDialogueUI::AutoCloseDialogue()
{
    CurrentState = 0;
    DialogueManager->RemoveDialogueUI();
}

void UDialogueUI::ClearAutoDialogueTimer()
{
    GetWorld()->GetTimerManager().ClearTimer(AutoDialogueTimer);
}
