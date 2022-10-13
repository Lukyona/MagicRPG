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
        int index = SelectedReply - 1;
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
                AutoCloseDialogue();
                Main->bCanMove = true;
                AllNpcDisableLookAt();
                return;
            }
        }
        else if (MainPlayerController->FallingCount == 5)
        {
            RowIndex = 7;
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
                return;
            }

            switch (RowIndex)
            {
            case 1: // Momo, Set FollowCamera's Z value of Rotation
            case 7:
                Main->FollowCamera->SetRelativeRotation(FRotator(0.f, -15.f, 0.f));
                bDisableMouseAndKeyboard = false;
                break;
            case 2: // Vivi
            case 6:
                Main->FollowCamera->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
                break;
            case 3: // Luko
            case 8:
                Main->FollowCamera->SetRelativeRotation(FRotator(0.f, -25.f, 0.f));
                break;
            case 4: // Zizi
                Main->FollowCamera->SetRelativeRotation(FRotator(0.f, 13.f, 0.f));
                break;
            case 5: // Vovo
            case 9:
                Main->FollowCamera->SetRelativeRotation(FRotator(0.f, 30.f, 0.f));
                if (RowIndex == 5 && MessageIndex == 2)
                    Main->FollowCamera->SetRelativeRotation(FRotator(0.f, 5.f, 0.f));
                break;
            case 10: // npc go
                Main->FollowCamera->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
                Main->CameraBoom->TargetArmLength = 500.f;

                // npc move except luko              
                Main->Momo->AIController->MoveToLocation(FVector(5200.f, 35.f, 100.f));
                Main->Vovo->AIController->MoveToLocation(FVector(5200.f, 35.f, 100.f));
                Main->Vivi->AIController->MoveToLocation(FVector(5200.f, 35.f, 100.f));
                Main->Zizi->AIController->MoveToLocation(FVector(5200.f, 35.f, 100.f));
                break;
            case 11:
                Main->bCanMove = false;
                GetWorld()->GetTimerManager().SetTimer(OnceTimer, FTimerDelegate::CreateLambda([&]()
                    {
                        MainPlayerController->DisplayDialogueUI();
                        Main->bInterpToNpc = true;
                        Main->TargetNpc = Main->Luko;
                        GetWorld()->GetTimerManager().ClearTimer(OnceTimer);

                    }), 1.7f, false); // 1.7초 뒤 루코 대화
                AutoCloseDialogue();
                return;
                break;
            }
        }

        if (DNum == 1)
        {
            if (RowIndex == 0) RowIndex = 11;
        }

        if (DNum == 2 && RowIndex == 4)
        {
            Main->Luko->bInterpToCharacter = true;
            Main->Luko->TargetCharacter = Main;
        }

        if (DNum == 3) // Third Dialogue (first dungeon, after golem battle)
        {
            switch (RowIndex)
            {
            case 0:
                Main->CameraBoom->TargetArmLength = 200.f;
                break;
            case 2:
                if (MessageIndex == 1)
                {
                    // npc move to the boat except vovo
                    Main->Momo->AIController->MoveToLocation(FVector(660.f, 1035.f, 1840.f));
                    Main->Luko->AIController->MoveToLocation(FVector(598.f, 1030.f, 1840.f));
                    Main->Vivi->AIController->MoveToLocation(FVector(710.f, 995.f, 1840.f));
                    Main->Zizi->AIController->MoveToLocation(FVector(690.f, 930.f, 1840.f));
                }
                break;
            case 3: // vovo look at player and player look at vovo
                Main->Vovo->bInterpToCharacter = true;
                Main->Vovo->TargetCharacter = Main;
                Main->bInterpToNpc = true;
                Main->TargetNpc = Main->Vovo;
                break;
            case 5:
                if (SelectedReply == 1) RowIndex = 7;
                break;
            case 6:
                if (SelectedReply == 2)
                {
                    //RowIndex = 8;
                    //Main->Vovo->AIController->MoveToLocation(FVector(630.f, 970.f, 1840.f));
                    AutoCloseDialogue();
                    return;
                }
                break;
            case 8:
                // vovo moves to the boat
                if (SelectedReply == 1 || SelectedReply == 3)
                {
                    //Main->Vovo->AIController->MoveToLocation(FVector(630.f, 970.f, 1840.f));    
                    //SelectedReply = 0;
                    AutoCloseDialogue();
                    return;
                }
                //case 9:
                //case 10:
                //case 11:
                //    GetWorld()->GetTimerManager().SetTimer(OnceTimer, FTimerDelegate::CreateLambda([&]()
                //    {
                //        Interact();
                //        if (RowIndex == 11)
                //        {
                //            bDisableMouseAndKeyboard = false;
                //            GetWorld()->GetTimerManager().ClearTimer(OnceTimer);
                //        }

                //    }), 2.f, false); // 대화 자동 진행
                break;
            }
        }

        if (DNum == 4) // Third Dialogue (first dungeon, after golem battle)
        {
            switch (RowIndex)
            {
            case 8:
                bDisableMouseAndKeyboard = true;
            case 9:
            case 10:
            case 11:
                GetWorld()->GetTimerManager().SetTimer(OnceTimer, FTimerDelegate::CreateLambda([&]()
                    {
                        Interact();
                        if (RowIndex == 11)
                        {
                            bDisableMouseAndKeyboard = false;
                            GetWorld()->GetTimerManager().ClearTimer(OnceTimer);
                        }

                    }), 3.f, false); // 대화 자동 진행
                break;
            }
        }

        if (DNum == 5) // enter the second dungeon
        {
            switch (RowIndex)
            {
            case 2:
                Main->Momo->TargetCharacter = Main->Vivi;
                Main->Momo->bInterpToCharacter = true;
                Main->Zizi->TargetCharacter = Main->Vivi;
                Main->Zizi->bInterpToCharacter = true;
                break;
            case 6:
                if (MessageIndex == 2)
                    Main->Zizi->TargetCharacter = Main;
                break;
            case 7:
                Main->Momo->TargetCharacter = Main;
                Main->Vivi->TargetCharacter = Main;
                Main->Vivi->bInterpToCharacter = true;
                Main->Luko->TargetCharacter = Main;
                Main->Luko->bInterpToCharacter = true;
                Main->Vovo->TargetCharacter = Main;
                Main->Vovo->bInterpToCharacter = true;
                break;
            case 10:
                Main->Zizi->TargetCharacter = Main->Vivi;
                break;
            case 13:
                if (MessageIndex == 0)
                    Main->Momo->MoveToPlayer();
                else
                {
                    AllNpcDisableLookAt();
                }
                break;
            }
        }

        if (DNum == 6)
        {
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
                    for (int i = 0; i < Main->NPCList.Num(); i++)
                    {
                        Main->NPCList[i]->AIController->StopMovement();
                        GetWorld()->GetTimerManager().ClearTimer(Main->NPCList[i]->MoveTimer);
                    }
                    Main->Momo->AIController->MoveToLocation(FVector(5320.f, -3702.f, -2122.f));
                    Main->Luko->AIController->MoveToLocation(FVector(5249.f, -3685.f, -2117.f));
                    Main->Vovo->AIController->MoveToLocation(FVector(5462.f, -3725.f, -2117.f));
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
                for (int i = 0; i < Main->NPCList.Num(); i++)
                {
                    Main->NPCList[i]->AIController->StopMovement();
                    GetWorld()->GetTimerManager().ClearTimer(Main->NPCList[i]->MoveTimer);
                }
                Main->Momo->AIController->MoveToLocation(FVector(5320.f, -3702.f, -2122.f));
                Main->Luko->AIController->MoveToLocation(FVector(5249.f, -3685.f, -2117.f));
                Main->Vovo->AIController->MoveToLocation(FVector(5462.f, -3725.f, -2117.f));
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
                        GetWorld()->GetTimerManager().SetTimer(OnceTimer, FTimerDelegate::CreateLambda([&]()
                            {
                                Interact();
                                GetWorld()->GetTimerManager().ClearTimer(OnceTimer);
                            }), 1.f, false); // 대화 자동 진행
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
                case 17:
                    AllNpcDisableLookAt();
                    Main->bInterpToNpc = false;
                    Main->TargetNpc = nullptr;
                    break;
            }
        }
    }
   

    //UE_LOG(LogTemp, Log, TEXT("pass4"));
    AnimateMessage(Dialogue[RowIndex]->Messages[MessageIndex].ToString());
}

void UDialogueUI::AutoCloseDialogue()
{
    CurrentState = 0;
    MainPlayerController->RemoveDialogueUI();
}


void UDialogueUI::AllNpcLookAtPlayer()
{
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