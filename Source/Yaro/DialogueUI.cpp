// Fill out your copyright notice in the Description page of Project Settings.


#include "DialogueUI.h"
#include "Components/TextBlock.h"
#include "Main.h"
#include "MainPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "YaroCharacter.h"
#include "AIController.h"
#include "GameFramework/CharacterMovementComponent.h"


void UDialogueUI::SetMessage(const FString& Text)
{
    if (NPCText == nullptr) return;

    NPCText->SetText(FText::FromString(Text));
}

void UDialogueUI::SetCharacterName(const FString& Text)
{
    if (CharacterNameText == nullptr) return;

    CharacterNameText->SetText(FText::FromString(Text));
}

void UDialogueUI::AnimateMessage(const FString& Text)
{
    CurrentState = 1;

    InitialMessage = Text;
    //UE_LOG(LogTemp, Log, TEXT("%s, AnimateMessage"), *InitialMessage);

    OutputMessage = "";

    iLetter = 0;

    NPCText->SetText(FText::FromString(""));
    CharacterNameText->SetText(FText::FromString(Dialogue[RowIndex]->CharacterName.ToString()));

    SetSpeechBubbleLocation();

    GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UDialogueUI::OnAnimationTimerCompleted, 0.2f, false);
}

void UDialogueUI::OnAnimationTimerCompleted()
{
    GetWorld()->GetTimerManager().ClearTimer(TimerHandle);

    OutputMessage.AppendChar(InitialMessage[iLetter]);

    NPCText->SetText(FText::FromString(OutputMessage));
   // UE_LOG(LogTemp, Log, TEXT("OnAnimationTimerCompleted"));

    if (SoundCueMessage != nullptr)
        UAudioComponent* AudioComponent = UGameplayStatics::SpawnSound2D(this, SoundCueMessage);

    if ((iLetter + 1) < InitialMessage.Len())
    {
        iLetter += 1;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UDialogueUI::OnAnimationTimerCompleted, DelayBetweenLetters, false);
    }
    else
    {
        CurrentState = 2;
    }
}


void UDialogueUI::InitializeDialogue(UDataTable* DialogueTable)
{
    if (Main == nullptr)
        Main = Cast<AMain>(UGameplayStatics::GetPlayerCharacter(this, 0));

    if (MainPlayerController == nullptr)
        MainPlayerController = Cast<AMainPlayerController>(Main->GetController());

    //UE_LOG(LogTemp, Log, TEXT("InitializeDialogue"));

    CurrentState = 0;

    CharacterNameText->SetText(FText::FromString(""));
    NPCText->SetText(FText::FromString(""));

    OnResetOptions();

    Dialogue.Empty();

    for (auto it : DialogueTable->GetRowMap())
    {
        FNPCDialogue* Row = (FNPCDialogue*)it.Value;

        Dialogue.Add(Row);
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
        GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
        NPCText->SetText(FText::FromString(InitialMessage));
       // UE_LOG(LogTemp, Log, TEXT("Interact-1"));

        CurrentState = 2;
    }
    else if (CurrentState == 2) // Text completed
    {

        // Get next message
        if ((MessageIndex + 1) < Dialogue[RowIndex]->Messages.Num()) // 같은 npc의 다음 대사
        {
            MessageIndex += 1;
            DialogueEvents();
           // UE_LOG(LogTemp, Log, TEXT("Interact-2-1"));

        }
        else
        {
            NPCText->SetText(FText::FromString(""));

            if (Dialogue[RowIndex]->PlayerReplies.Num() > 0) // 플레이어 응답 있으면
            {
               // UE_LOG(LogTemp, Log, TEXT("Interact-2-2-1"));

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

                   // UE_LOG(LogTemp, Log, TEXT("Interact-2-2-2"));
                    MessageIndex = 0;
                    DialogueEvents();

                }
                else
                {
                   // UE_LOG(LogTemp, Log, TEXT("Interact-2-2-3"));
                    bCanStartDialogue = false;
                    MainPlayerController->RemoveDialogueUI();
                    CurrentState = 0;
                }
            }
        }
    }
    else if (CurrentState == 3) // 플레이어 응답 선택한 상태
    {
        // 플레이어 응답에 따라 RowIndex 바뀜
        int index = SelectedReply - 1; // SelectedReply는 1,2,3
        RowIndex = Dialogue[RowIndex]->PlayerReplies[index].AnswerIndex;
        OnResetOptions();

        if ((RowIndex >= 0) && (RowIndex < Dialogue.Num()))
        {
            NPCText->SetText(FText::FromString(""));
            //UE_LOG(LogTemp, Log, TEXT("Interact-3-1"));

            MessageIndex = 0;
            DialogueEvents();

        }
        else
        {
            //UE_LOG(LogTemp, Log, TEXT("Interact-3-2"));
            bCanStartDialogue = false;

            CurrentState = 0;
            MainPlayerController->RemoveDialogueUI();

        }
    }
}

void UDialogueUI::DialogueEvents()
{
    int DNum = MainPlayerController->DialogueNum;
   // UE_LOG(LogTemp, Log, TEXT("DialogueEvents"));

    if (MainPlayerController->bFallenPlayer)
    {
        bDisableMouseAndKeyboard = false;

        if (MainPlayerController->FallingCount == 1)
        {
            if (RowIndex == 0)
            {
                AllNpcLookAtPlayer();
            }

            if (RowIndex == 7)
            {
                MainPlayerController->FallingCount++;
                AutoCloseDialogue();
                AllNpcDisableLookAt();
                MainPlayerController->bFallenPlayer = false;
                return;
            }
        }
        else if (MainPlayerController->FallingCount == 6)
        {
            if (RowIndex == 0) RowIndex = 7;

            if (RowIndex == 8)
            {
                MainPlayerController->FallingCount++;
                AutoCloseDialogue();
                MainPlayerController->bFallenPlayer = false;
                return;
            }

        }
        else return;
    }
    else
    {
        if (DNum == 0) // First Dialogue (cave)
        {
            if (RowIndex < 10 && Main->CameraBoom->TargetArmLength > 0) // 1인칭시점이 아닐 때
            {
                AnimateMessage(Dialogue[RowIndex]->Messages[MessageIndex].ToString());

                if (RowIndex == 0 && MessageIndex == 2)
                {
                    MainPlayerController->SystemMessageNum = 1;
                    MainPlayerController->SetSystemMessage();
                }

                switch (RowIndex)
                {
                case 1:
                    if (MessageIndex > 0)
                        MainPlayerController->DisplaySpeechBuubble(Main->Momo);
                    break;
                case 7:
                    MainPlayerController->DisplaySpeechBuubble(Main->Momo);
                    break;
                case 2:
                case 6:
                    MainPlayerController->DisplaySpeechBuubble(Main->Vivi);
                    break;
                case 3:
                case 8:
                    MainPlayerController->DisplaySpeechBuubble(Main->Luko);
                    break;
                case 4:
                    MainPlayerController->DisplaySpeechBuubble(Main->Zizi);
                    break;
                case 5:
                case 9:
                    MainPlayerController->DisplaySpeechBuubble(Main->Vovo);
                    break;
                case 10:
                    MainPlayerController->RemoveSpeechBuubble();
                }
                return;
            }

            switch (RowIndex) // 1인칭 시점일 때
            {
            case 1: // Momo, Set FollowCamera's Z value of Rotation
            case 7:
                Main->FollowCamera->SetRelativeRotation(FRotator(0.f, -15.f, 0.f));
                bDisableMouseAndKeyboard = false;
                MainPlayerController->DisplaySpeechBuubble(Main->Momo);
                break;
            case 2: // Vivi
            case 6:
                Main->FollowCamera->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
                MainPlayerController->DisplaySpeechBuubble(Main->Vivi);
                break;
            case 3: // Luko
            case 8:
                Main->FollowCamera->SetRelativeRotation(FRotator(0.f, -25.f, 0.f));
                MainPlayerController->DisplaySpeechBuubble(Main->Luko);
                break;
            case 4: // Zizi
                Main->FollowCamera->SetRelativeRotation(FRotator(0.f, 13.f, 0.f));
                MainPlayerController->DisplaySpeechBuubble(Main->Zizi);
                break;
            case 5: // Vovo
            case 9:
                Main->FollowCamera->SetRelativeRotation(FRotator(0.f, 30.f, 0.f));
                MainPlayerController->DisplaySpeechBuubble(Main->Vovo);
                if (RowIndex == 5 && MessageIndex == 2)
                    Main->FollowCamera->SetRelativeRotation(FRotator(0.f, 5.f, 0.f));
                break;
            case 10: // npc go
                Main->FollowCamera->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
                Main->CameraBoom->TargetArmLength = 500.f;
                MainPlayerController->RemoveSpeechBuubble();

                // npc move except luko              
                Main->Momo->AIController->MoveToLocation(FVector(5200.f, 35.f, 100.f));
                Main->Vovo->AIController->MoveToLocation(FVector(5200.f, 35.f, 100.f));
                Main->Vivi->AIController->MoveToLocation(FVector(5200.f, 35.f, 100.f));
                Main->Zizi->AIController->MoveToLocation(FVector(5200.f, 35.f, 100.f));
                bDisableMouseAndKeyboard = true;
                GetWorld()->GetTimerManager().SetTimer(OnceTimer, this, &UDialogueUI::AutoDialogue, 2.f, false);
                break;
            case 11:
                Main->bCanMove = false;
                GetWorld()->GetTimerManager().SetTimer(OnceTimer, FTimerDelegate::CreateLambda([&]()
                    {
                        MainPlayerController->DisplayDialogueUI();
                        Main->bInterpToNpc = true;
                        Main->TargetNpc = Main->Luko;
                        GetWorld()->GetTimerManager().ClearTimer(OnceTimer);

                    }), 2.f, false); // 2초 뒤 루코 대화
                AutoCloseDialogue();
                return;
                break;
            }
        }

        if (DNum == 1 && RowIndex == 0) // 루코 대화
        {
            bDisableMouseAndKeyboard = false;
            RowIndex = 11;
            Main->CameraBoom->TargetArmLength = 500.f;
        }

        if (DNum == 2) // enter first dungeon
        {
            switch (RowIndex)
            {
            case 0:
            case 2:
                MainPlayerController->bCanDisplaySpeechBubble = true;
                break;
            case 1:
                break;
            case 3:
                break;
            case 4:
                Main->Luko->bInterpToCharacter = true;
                Main->Luko->TargetCharacter = Main;
                break;
            }
        }

        if (DNum == 3) // Third Dialogue (first dungeon, after golem battle)
        {
            switch (RowIndex)
            {
            case 0:
                Main->CameraBoom->TargetArmLength = 200.f;
            case 2:
                if (MessageIndex == 1)
                {
                    // npc move to the boat except vovo
                    Main->Momo->AIController->MoveToLocation(FVector(660.f, 1035.f, 1840.f));
                    Main->Luko->AIController->MoveToLocation(FVector(598.f, 1030.f, 1840.f));
                    Main->Vivi->AIController->MoveToLocation(FVector(710.f, 995.f, 1840.f));
                    Main->Zizi->AIController->MoveToLocation(FVector(690.f, 930.f, 1840.f));
                    MainPlayerController->RemoveSpeechBuubble();
                }
                break;
            case 3: // vovo look at player and player look at vovo
                Main->Vovo->bInterpToCharacter = true;
                Main->Vovo->TargetCharacter = Main;
                Main->bInterpToNpc = true;
                Main->TargetNpc = Main->Vovo;
                MainPlayerController->bCanDisplaySpeechBubble = true;
                break;
            case 5:
                if (SelectedReply == 1) RowIndex = 7;
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
            }
        }

        if (DNum == 4) // Third Dialogue (first dungeon, boat moves)
        {
            switch (RowIndex)
            {
                case 0:
                    for (int i = 0; i < Main->NPCList.Num(); i++)
                    {
                        Main->NPCList[i]->AIController->StopMovement();
                    }
                    RowIndex = 8;
                    bDisableMouseAndKeyboard = true;
                case 9:
                case 10:
                case 11:
                    // GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("first"));
                    GetWorld()->GetTimerManager().SetTimer(OnceTimer, this, &UDialogueUI::AutoDialogue, 3.f, false);
                    break;
            }
        }

        if (DNum == 5) // enter the second dungeon
        {
            switch (RowIndex)
            {
            case 0:
                MainPlayerController->bCanDisplaySpeechBubble = true;
                break;
            case 2:
                Main->Momo->TargetCharacter = Main->Vivi;
                Main->Momo->bInterpToCharacter = true;
                Main->Zizi->TargetCharacter = Main->Vivi;
                Main->Zizi->bInterpToCharacter = true;
                Main->TargetNpc = Main->Vivi;
                Main->bInterpToNpc = true;
                break;
            case 6:
                if (MessageIndex == 2)
                {
                    Main->Zizi->TargetCharacter = Main;
                    Main->TargetNpc = Main->Zizi;
                }
                break;
            case 7:
                Main->Momo->TargetCharacter = Main;
                Main->Vivi->TargetCharacter = Main;
                Main->Vivi->bInterpToCharacter = true;
                Main->Luko->TargetCharacter = Main;
                Main->Luko->bInterpToCharacter = true;
                Main->Vovo->TargetCharacter = Main;
                Main->Vovo->bInterpToCharacter = true;
                Main->TargetNpc = Main->Momo;
                break;
            case 10:
                Main->Zizi->TargetCharacter = Main->Vivi;
                break;
            case 13:
                if (MessageIndex == 0)
                {
                    Main->Momo->MoveToPlayer();
                }
                else
                {
                    AllNpcDisableLookAt();
                    Main->bInterpToNpc = false;
                }
                break;
            }
        }

        if (DNum == 6)
        {
            Main->bCanMove = false;
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
                AllNpcLookAtPlayer();
                AllNpcStopFollowPlayer();
                Main->Momo->AIController->MoveToLocation(FVector(5307.f, -3808.f, -2122.f));
                Main->Luko->AIController->MoveToLocation(FVector(5239.f, -3865.f, -2117.f));
                Main->Vovo->AIController->MoveToLocation(FVector(5433.f, -3855.f, -2117.f));
                Main->Vivi->AIController->MoveToLocation(FVector(5392.f, -3686.f, -2117.f));
                Main->Zizi->AIController->MoveToLocation(FVector(5538.f, -3696.f, -2115.f));
                break;
            case 6: // zizi says
                bDisableMouseAndKeyboard = false;
                Main->bCanMove = true;
                AutoCloseDialogue();
                return;
                break;
            }
        }

        if (DNum == 8) // player go over the side
        {
            switch (RowIndex)
            {
            case 0:
                RowIndex = 6;
                AllNpcLookAtPlayer();
                AllNpcStopFollowPlayer();
                Main->Momo->AIController->MoveToLocation(FVector(5307.f, -3808.f, -2122.f));
                Main->Luko->AIController->MoveToLocation(FVector(5239.f, -3865.f, -2117.f));
                Main->Vovo->AIController->MoveToLocation(FVector(5433.f, -3855.f, -2117.f));
                Main->Vivi->AIController->MoveToLocation(FVector(5392.f, -3686.f, -2117.f));
                Main->Zizi->AIController->MoveToLocation(FVector(5538.f, -3696.f, -2115.f));
                GetWorld()->GetTimerManager().SetTimer(OnceTimer, FTimerDelegate::CreateLambda([&]()
                    {
                        Main->TargetNpc = Main->Vivi;
                        Main->bInterpToNpc = true;
                        GetWorld()->GetTimerManager().ClearTimer(OnceTimer);
                    }), 0.6f, false); // npc들쪽 쳐다보기
                break;
            case 10:
                Main->bCanMove = true;
                AutoCloseDialogue();
                return;
                break;
            }
        }

        if (DNum == 9) // plane2 up
        {
            switch (RowIndex)
            {
                case 0:
                    Main->bCanMove = false;
                    RowIndex = 10;
                    Main->bInterpToNpc = true;
                    Main->TargetNpc = Main->Vivi;
                    break;
                case 11:
                    Main->Zizi->TargetCharacter = Main->Vivi;
                    Main->Zizi->bInterpToCharacter = true;
                    break;
                case 12:
                    if (MessageIndex == 1)
                    {
                        bDisableMouseAndKeyboard = true;
                        GetWorld()->GetTimerManager().SetTimer(OnceTimer, this, &UDialogueUI::AutoDialogue, 1.f, false);
                    }
                    break;
                case 13:
                    AutoCloseDialogue();
                    bDisableMouseAndKeyboard = false;
                    return;
                    break;
            }
        }

        if (DNum == 10)
        {
            switch (RowIndex)
            {
                case 0:
                    RowIndex = 13;
                    Main->TargetNpc = Main->Momo;
                    Main->Momo->TargetCharacter = Main;
                    Main->Momo->bInterpToCharacter = true;
                    break;
                case 14:
                    Main->Vovo->TargetCharacter = Main;
                    Main->Vovo->bInterpToCharacter = true;
                    Main->TargetNpc = Main->Vovo;
                    break;
                case 15:
                    Main->Vivi->TargetCharacter = Main;
                    Main->Vivi->bInterpToCharacter = true;
                    Main->TargetNpc = Main->Vivi;
                    break;
                case 16:
                    Main->Zizi->TargetCharacter = Main;
                    Main->Zizi->bInterpToCharacter = true;
                    Main->TargetNpc = Main->Zizi;
                    break;
            }
        }

        if (DNum == 11)
        {
            switch (RowIndex)
            {
                case 2:
                    Main->Vivi->SetActorRotation(FRotator(0.f, 0.f, 0.f));
                    Main->Luko->TargetCharacter = Main->Vivi;
                    Main->Luko->bInterpToCharacter = true;
                    Main->Vovo->TargetCharacter = Main->Vivi;
                    Main->Vovo->bInterpToCharacter = true;
                    Main->Zizi->TargetCharacter = Main->Vivi;
                    Main->Zizi->bInterpToCharacter = true;
                    Main->CameraBoom->TargetArmLength = 290.f;
                    MainPlayerController->SetViewTargetWithBlend(Main, 1.f); // 플레이어 카메라로 복귀
                    break;
                case 4:
                    Main->Momo->AIController->MoveToLocation(FVector(-260.f, -1900.f, -121.f)); //momo move to the table
                    break;
                case 5: // 모모 방향으로 카메라 돌리기
                    MainPlayerController->RemoveSpeechBuubble();
                    Main->FollowCamera->SetRelativeRotation((FRotator(0.f, 330.f, 0.f)));
                    Main->CameraBoom->SocketOffset = FVector(0.f, -750.f, 70.f);
                    Main->CameraBoom->TargetArmLength = -10.f;

                    bDisableMouseAndKeyboard = true;
                    GetWorld()->GetTimerManager().SetTimer(OnceTimer, this, &UDialogueUI::AutoDialogue, 2.f, false);
                    break;
                case 6://다시 카메라 플레이어쪽으로 돌리기
                    MainPlayerController->bCanDisplaySpeechBubble = true;
                    bDisableMouseAndKeyboard = false;
                    Main->FollowCamera->SetRelativeRotation((FRotator(0.f, 0.f, 0.f)));
                    Main->CameraBoom->SocketOffset = FVector(0.f, 0.f, 70.f);
                    Main->CameraBoom->TargetArmLength = 290.f;
                    break;
                case 8:
                    AllNpcDisableLookAt();
                    if (ExplosionSound != nullptr)
                        UAudioComponent* AudioComponent = UGameplayStatics::SpawnSound2D(this, ExplosionSound);
                    bDisableMouseAndKeyboard = true;
                    GetWorld()->GetTimerManager().SetTimer(OnceTimer, this, &UDialogueUI::AutoDialogue, 0.6f, false);
                    break;
                case 9: //npc이동 지지 제외
                    Main->Luko->GetCharacterMovement()->MaxWalkSpeed = 450.f;
                    Main->Vovo->GetCharacterMovement()->MaxWalkSpeed = 450.f;
                    Main->Vivi->GetCharacterMovement()->MaxWalkSpeed = 500.f;
                    Main->Luko->AIController->MoveToLocation(FVector(-198.f, -2065.f, -118.f));
                    Main->Vivi->AIController->MoveToLocation(FVector(-347.f, -2040.f, -121.f));
                    Main->Vovo->AIController->MoveToLocation(FVector(-248.f, -2141.f, -118.f));
                    GetWorld()->GetTimerManager().SetTimer(OnceTimer, this, &UDialogueUI::AutoDialogue, 1.5f, false);
                    break;
                case 10:
                    Main->Zizi->GetCharacterMovement()->MaxWalkSpeed = 500.f;
                    Main->Zizi->AIController->MoveToLocation(FVector(-415.f, -2180.f, -116.f));
                    AutoCloseDialogue();
                    bDisableMouseAndKeyboard = false;
                    MainPlayerController->RemoveSpeechBuubble();
                    return;
                    break;
            }
        }

        if (DNum == 12)
        {
            if (RowIndex == 0)
            {
                Main->bCanMove = false;
                RowIndex = 10;
                Main->Momo->TargetCharacter = Main;
                for (auto npc : Main->NPCList)
                {
                    if (!npc->GetName().Contains("Momo"))
                    {
                        npc->TargetCharacter = Main->Momo;
                    }
                    npc->bInterpToCharacter = true;
                }
                Main->TargetNpc = Main->Momo;
                Main->bInterpToNpc = true;
            }
            if (RowIndex == 12)
            {
                AllNpcDisableLookAt();
                Main->bInterpToNpc = false;
                AutoCloseDialogue();
                return;
            }
        }

        if (DNum == 13)
        {
            switch (RowIndex)
            {
                case 0:
                    Main->bCanMove = false;
                    RowIndex = 12;
                    Main->Vivi->TargetCharacter = Main;
                    AllNpcStopFollowPlayer();
                    for (auto npc : Main->NPCList)
                    {
                        if (!npc->GetName().Contains("Vivi"))
                        {
                            npc->TargetCharacter = Main->Vivi;
                        }
                        npc->bInterpToCharacter = true;
                    }
                    Main->TargetNpc = Main->Vivi;
                    Main->bInterpToNpc = true;
                    break;
                case 14:
                    Main->Vovo->TargetCharacter = Main->Momo;
                    break;
                case 15:
                    if(MessageIndex == 1) 
                        Main->Momo->TargetCharacter = Main;
                    break;
                case 16:
                    Main->Zizi->TargetCharacter = Main;
                    Main->TargetNpc = Main->Zizi;
                    break;
                case 17:
                    Main->bCanMove = true;
                    AllNpcDisableLookAt();
                    Main->bInterpToNpc = false;
                    AutoCloseDialogue();
                    return;
                    break;
            }
        }

        if (DNum == 14)
        {
            if (RowIndex == 0)
            {
                RowIndex = 17;
                Main->bCanMove = false;
            }

        }

        if (DNum == 15)
        {
            switch (RowIndex)
            {
                case 2:
                    if (MessageIndex == 1)
                    {
                        Main->Momo->TargetCharacter = Main;
                        Main->Momo->bInterpToCharacter = true;
                        Main->TargetNpc = Main->Momo;
                        Main->bInterpToNpc = true;
                    }
                    break;
                case 3:
                    Main->Luko->TargetCharacter = Main->Momo;
                    Main->Luko->bInterpToCharacter = true;
                    break;
                case 4:
                    Main->Vivi->TargetCharacter = Main;
                    Main->Vivi->bInterpToCharacter = true;
                    Main->TargetNpc = Main->Vivi;
                    break;
                case 5:
                    bDisableMouseAndKeyboard = true;
                    AllNpcDisableLookAt();
                    GetWorld()->GetTimerManager().SetTimer(OnceTimer, this, &UDialogueUI::AutoDialogue, 1.5f, false);
                    break;
                case 6:
                    if (SelectedReply == 1)
                    {
                        MainPlayerController->RemoveSpeechBuubble();
                        Main->Vivi->AIController->MoveToLocation(FVector(105.f, 3176.f, 182.f));
                        Main->Momo->AIController->MoveToLocation(FVector(-86.f, 3263.f, 177.f));
                        Main->Luko->AIController->MoveToLocation(FVector(184.f, 3317.f, 182.f));
                        Main->Vovo->AIController->MoveToLocation(FVector(-140.f, 3370.f, 182.f));
                        Main->Zizi->AIController->MoveToLocation(FVector(68.f, 3398.f, 184.f));
                        AutoCloseDialogue();
                        return;
                    }
                    else if (MessageIndex == 1)
                    {
                        bDisableMouseAndKeyboard = true;
                        GetWorld()->GetTimerManager().SetTimer(OnceTimer, this, &UDialogueUI::AutoDialogue, 2.f, false);
                    }
                    break;
                case 7:
                    if (SelectedReply == 2)
                    {
                        MainPlayerController->RemoveSpeechBuubble();
                        Main->Vivi->AIController->MoveToLocation(FVector(105.f, 3176.f, 182.f));
                        Main->Momo->AIController->MoveToLocation(FVector(-86.f, 3263.f, 177.f));
                        Main->Luko->AIController->MoveToLocation(FVector(184.f, 3317.f, 182.f));
                        Main->Vovo->AIController->MoveToLocation(FVector(-140.f, 3370.f, 182.f));
                        Main->Zizi->AIController->MoveToLocation(FVector(68.f, 3398.f, 184.f));
                        AutoCloseDialogue();
                        return;
                    }
                    break;
            }
        }

        if (DNum == 16) // 돌 치우기 직전
        {
            switch (RowIndex)
            {
            case 0:
                Main->bCanMove = false;
                RowIndex = 7;
                break;
            case 8:
                Main->Momo->TargetCharacter = Main->Zizi;
                Main->Momo->bInterpToCharacter = true;
                Main->Luko->TargetCharacter = Main->Zizi;
                Main->Luko->bInterpToCharacter = true;
                Main->Vovo->TargetCharacter = Main->Zizi;
                Main->Vovo->bInterpToCharacter = true;
                Main->Vivi->TargetCharacter = Main->Zizi;
                Main->Vivi->bInterpToCharacter = true;
                Main->TargetNpc = Main->Zizi;
                Main->bInterpToNpc = true;
                break;
            case 10:
                Main->Zizi->AIController->MoveToLocation(FVector(18, 3090, 182.f));
                bDisableMouseAndKeyboard = true;

                GetWorld()->GetTimerManager().SetTimer(OnceTimer, this, &UDialogueUI::AutoDialogue, 1.5f, false);
                break;
            case 11:
                Main->Zizi->SetActorRotation(FRotator(0.f, 260.f, 0.f));
                if (Main->Zizi->CombatMontage != nullptr)
                {
                    Main->Zizi->AnimInstance->Montage_Play(Main->Zizi->CombatMontage);
                    Main->Zizi->AnimInstance->Montage_JumpToSection(FName("Attack"));
                }
                bDisableMouseAndKeyboard = true;
                GetWorld()->GetTimerManager().SetTimer(OnceTimer, this, &UDialogueUI::AutoDialogue, 1.6f, false);
                break;
            case 12:
                bDisableMouseAndKeyboard = false;
                break;
            case 16:
                bDisableMouseAndKeyboard = true;
                GetWorld()->GetTimerManager().SetTimer(OnceTimer, this, &UDialogueUI::AutoDialogue, 1.4f, false);
                break;
            case 17:
                Main->bInterpToNpc = false;
                AllNpcDisableLookAt();
                Main->Vivi->AIController->MoveToLocation(FVector(100.f, 1997.f, 182.f));
                Main->Momo->AIController->MoveToLocation(FVector(-86.f, 2150.f, 177.f));
                Main->Luko->AIController->MoveToLocation(FVector(171.f, 2130.f, 182.f));
                Main->Vovo->AIController->MoveToLocation(FVector(-160.f, 2060.f, 182.f));
                Main->Zizi->AIController->MoveToLocation(FVector(18.f, 2105.f, 184.f));
                AutoCloseDialogue();
                return;
                break;
            }
        }

        if (DNum == 17) // 돌 치우고 건너간 뒤
        {
            switch (RowIndex)
            {
                case 0:
                    Main->bCanMove = false;
                    RowIndex = 17;
                    Main->Vivi->TargetCharacter = Main;
                    Main->Vivi->bInterpToCharacter = true;
                    break;
                case 18:
                    if (MessageIndex == 0) MainPlayerController->RemoveSpeechBuubble();
                    if (MessageIndex == 1) MainPlayerController->bCanDisplaySpeechBubble = true;
                    Main->Luko->TargetCharacter = Main->Momo;
                    Main->Luko->bInterpToCharacter = true;
                    Main->Vovo->TargetCharacter = Main->Momo;
                    Main->Vovo->bInterpToCharacter = true;
                    Main->Zizi->TargetCharacter = Main->Momo;
                    Main->Zizi->bInterpToCharacter = true;
                    Main->Momo->AIController->MoveToLocation(FVector(-86.f, 2307.f, 177.f));
                    break;
                case 20:
                    Main->Zizi->TargetCharacter = Main;
                    break;
                case 21:
                    AllNpcDisableLookAt();
                    AutoCloseDialogue();
                    return;
                    break;          
            }
        }

        if (DNum == 18) // 보스 전투 끝
        {
            switch (RowIndex)
            {
                case 0:
                    if (AfterAllCombat != nullptr)
                        UAudioComponent* AudioComponent = UGameplayStatics::SpawnSound2D(this, AfterAllCombat);
                    Main->CameraBoom->TargetArmLength = 170.f;
                    RowIndex = 21;
                    break;
                case 23:
                    Main->TargetNpc = Main->Vovo;
                    Main->bInterpToNpc = true;
                case 24:
                    if (MessageIndex == 0)
                    {
                        Main->Zizi->TargetCharacter = Main->Vovo;
                        Main->Zizi->bInterpToCharacter = true;
                    }
                    else
                    {
                        Main->Zizi->TargetCharacter = Main->Vivi;
                    }
                    break;
                case 25:
                    Main->Vovo->TargetCharacter = Main->Vivi;
                    Main->Vovo->bInterpToCharacter = true;
                    Main->TargetNpc = Main->Vivi;
                    break;
                case 26:
                    Main->Momo->TargetCharacter = Main->Vivi;
                    Main->Momo->bInterpToCharacter = true;
                    break;
            }
        }

        if (DNum == 19) // 동굴 복귀
        {
            switch (RowIndex)
            {
                case 3:
                    MainPlayerController->RemoveSpeechBuubble();
                    Main->Momo->AIController->MoveToLocation(FVector(-4660.f, 118.f, 393.f));
                    Main->Luko->AIController->MoveToLocation(FVector(-4545.f, -241.f, 401.f));
                    Main->Vovo->AIController->MoveToLocation(FVector(-4429.f, 103.f, 396.f));
                    Main->Vivi->AIController->MoveToLocation(FVector(-4355.f, -195.f, 405.f));
                    Main->Zizi->AIController->MoveToLocation(FVector(-4695.f, -190.f, 394.f));
                    AutoCloseDialogue();
                    return;
                    break;
            }
        }

        if (DNum == 20) // 돌 발견, 챙기기
        {
            switch (RowIndex)
            {
            case 0:
                RowIndex = 3;
                Main->CameraBoom->TargetArmLength = 470.f;
                break;
            case 9:
                if (SelectedReply == 1)
                {
                    AllNpcLookAtPlayer();
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
                    bDisableMouseAndKeyboard = true;
                    GetWorld()->GetTimerManager().SetTimer(OnceTimer, this, &UDialogueUI::AutoDialogue, 1.5f, false);
                }
                break;
            case 11:
                Main->Vivi->GetCharacterMovement()->MaxWalkSpeed = 250.f;
                Main->Vivi->AIController->MoveToLocation(FVector(-4520.f, -50.f, 409.f));
                AutoCloseDialogue();
                return;
                break;
            }
        }

        if (DNum == 21) // 돌 챙기고 난 뒤
        {
            switch (RowIndex)
            {
                case 0:
                    AllNpcDisableLookAt();
                    RowIndex = 11;
                    break;
                case 12:
                    if (GriffonSound != nullptr)
                        UAudioComponent* AudioComponent = UGameplayStatics::SpawnSound2D(this, GriffonSound);
                    SpawnGriffon();
                    break;
                case 13:
                    bDisableMouseAndKeyboard = true;
                    GetWorld()->GetTimerManager().SetTimer(OnceTimer, this, &UDialogueUI::AutoDialogue, 1.5f, false);
                    break;
                case 14:
                    MainPlayerController->RemoveSpeechBuubble();
                    Main->Momo->GetCharacterMovement()->MaxWalkSpeed = 600.f;
                    Main->Vivi->GetCharacterMovement()->MaxWalkSpeed = 500.f;
                    Main->Zizi->GetCharacterMovement()->MaxWalkSpeed = 500.f;
                    Main->Vovo->GetCharacterMovement()->MaxWalkSpeed = 450.f;
                    Main->Luko->GetCharacterMovement()->MaxWalkSpeed = 450.f;
                    Main->Momo->AIController->MoveToLocation(FVector(508.f, 120.f, 100.f));
                    Main->Luko->AIController->MoveToLocation(FVector(311.f, -78.f, 103.f));
                    Main->Vovo->AIController->MoveToLocation(FVector(469.f, -22.f, 103.f));
                    Main->Vivi->AIController->MoveToLocation(FVector(267.f, 65.f, 101.f));
                    Main->Zizi->AIController->MoveToLocation(FVector(591.f, 28.f, 104.f));
                    AutoCloseDialogue();
                    return;
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
                    AllNpcLookAtPlayer();
                    break;
                case 22:
                    AllNpcDisableLookAt();
                    if (MessageIndex == 1)
                    {
                        Main->Zizi->AIController->MoveToLocation(FVector(2560.f, 320.f, 157.f));
                        Main->Vovo->AIController->MoveToLocation(FVector(2570.f, 325.f, 154.f));
                        Main->Luko->AIController->MoveToLocation(FVector(1517.f, 325.f, 155.f));
                        Main->Momo->AIController->MoveToLocation(FVector(1530.f, 330.f, 150.f));
                    }
                    break;
                case 23:
                    AutoCloseDialogue();
                    return;
                    break;
            }
        }

        if (DNum == 23)
        {
            RowIndex = 23;
        }

    }
   
    //UE_LOG(LogTemp, Log, TEXT("pass77"));
    AnimateMessage(Dialogue[RowIndex]->Messages[MessageIndex].ToString());
}

void UDialogueUI::AutoCloseDialogue()
{
    CurrentState = 0;
    MainPlayerController->RemoveDialogueUI();
}


void UDialogueUI::AllNpcLookAtPlayer()
{
    if (Main == nullptr)
        Main = Cast<AMain>(UGameplayStatics::GetPlayerCharacter(this, 0));

    for (int i = 0; i < Main->NPCList.Num(); i++)
    {
        Main->NPCList[i]->TargetCharacter = Main;
        Main->NPCList[i]->bInterpToCharacter = true;
    }
}

void UDialogueUI::AllNpcDisableLookAt()
{
    for (int i = 0; i < Main->NPCList.Num(); i++)
    {
        Main->NPCList[i]->bInterpToCharacter = false;
    }
}

void UDialogueUI::AllNpcStopFollowPlayer()
{
    for (int i = 0; i < Main->NPCList.Num(); i++)
    {
        Main->NPCList[i]->AIController->StopMovement();
        GetWorld()->GetTimerManager().ClearTimer(Main->NPCList[i]->MoveTimer);
    }
}

void UDialogueUI::AutoDialogue()
{
    Interact();
}