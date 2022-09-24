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
    UE_LOG(LogTemp, Log, TEXT("%s, AnimateMessage"), *InitialMessage);

    OutputMessage = "";

    iLetter = 0;

    NPCText->SetText(FText::FromString(""));
    CharacterNameText->SetText(FText::FromString(Dialogue[RowIndex]->CharacterName.ToString()));

    GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UDialogueUI::OnAnimationTimerCompleted, 0.1f, false);
}


void UDialogueUI::OnTimerCompleted()
{
    GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
    //UE_LOG(LogTemp, Log, TEXT("OnTimerCompleted"));

    AnimateMessage(Dialogue[RowIndex]->Messages[MessageIndex].ToString());
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
    if(Main == nullptr)
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
        CurrentState = 0;

        RowIndex = 0;

        if (Dialogue[RowIndex]->Messages.Num() > 0)
        {
            MessageIndex = 0;

            OnAnimationShowMessageUI();

            GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UDialogueUI::OnTimerCompleted, 0.1f, false);
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
                CharacterNameText->SetText(FText::FromString("player"));
               // UE_LOG(LogTemp, Log, TEXT("Interact-2-2-1"));

                OnResetOptions();
                NumOfReply = Dialogue[RowIndex]->PlayerReplies.Num();

                for (int i = 0; i < Dialogue[RowIndex]->PlayerReplies.Num(); i++)
                {
                    OnSetOption(i, Dialogue[RowIndex]->PlayerReplies[i].ReplyText);
                }

                SelectedReply = 0;

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
        RowIndex = Dialogue[RowIndex]->PlayerReplies[SelectedReply].AnswerIndex;
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

            CurrentState = 0;
            MainPlayerController->RemoveDialogueUI();

        }
    }
}

void UDialogueUI::DialogueEvents()
{
    int DNum = MainPlayerController->DialogueNum;
   // UE_LOG(LogTemp, Log, TEXT("DialogueEvents"));


    if (DNum == 0) // First Dialogue (cave)
    {
        if (RowIndex < 10 && Main->CameraBoom->TargetArmLength > 0) // 1인칭시점이 아닐 때
        {
            AnimateMessage(Dialogue[RowIndex]->Messages[MessageIndex].ToString());
            return;

        }

        switch (RowIndex)
        {
            case 1: // Momo, Set FollowCamera's Z value of Rotation
            case 7: 
                Main->FollowCamera->SetRelativeRotation(FRotator(0.f, -15.f, 0.f));
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
                if(RowIndex == 5 && MessageIndex == 2)
                    Main->FollowCamera->SetRelativeRotation(FRotator(0.f, 5.f, 0.f));
                break;
            case 10: // npc go
                Main->FollowCamera->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
                Main->CameraBoom->TargetArmLength = 500.f;
                for(int i = 0; i < Main->NPCList.Num(); i++)
                {
                    if (!Main->NPCList[i]->GetName().Contains("Luko")) // npc move except luko
                    {
                        Main->NPCList[i]->AIController->MoveToLocation(FVector(5200.f, 35.f, 100.f));
                    }
                }
                break;    
            case 11: 
                CurrentState = 0;
                MainPlayerController->RemoveDialogueUI();
                FTimerHandle Timer;
                GetWorld()->GetTimerManager().SetTimer(Timer, FTimerDelegate::CreateLambda([&]()
                    {
                        MainPlayerController->DisplayDialogueUI();
                    }), 1.5f, false); // n초 뒤 루코 대화
                return;
                break;
        }
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
                for (int i = 0; i < Main->NPCList.Num(); i++)
                {
                    if (!Main->NPCList[i]->GetName().Contains("Vovo")) // npc move to the boat except vovo
                    {
                        Main->NPCList[i]->AIController->MoveToLocation(FVector(628.f, 946.f, 1840.f));
                    }
                }
            }
            break;
        case 4:
            for (int i = 0; i < Main->NPCList.Num(); i++)
            {
                if (Main->NPCList[i]->GetName().Contains("Vovo")) // vovo moves to the boat
                {
                    Main->NPCList[i]->AIController->MoveToLocation(FVector(628.f, 885.f, 1840.f));

                }
            }
            CurrentState = 0;
            MainPlayerController->RemoveDialogueUI();
            return;
            break;
        case 11:
            //CurrentState = 0;
            //MainPlayerController->RemoveDialogueUI();
            //FTimerHandle Timer;
            //GetWorld()->GetTimerManager().SetTimer(Timer, FTimerDelegate::CreateLambda([&]()
            //    {
            //        MainPlayerController->DisplayDialogueUI();
            //    }), 1.5f, false); // n초 뒤 루코 대화
            //return;
            break;
        }
    }

    //UE_LOG(LogTemp, Log, TEXT("pass4"));
    AnimateMessage(Dialogue[RowIndex]->Messages[MessageIndex].ToString());


}

