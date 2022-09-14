// Fill out your copyright notice in the Description page of Project Settings.


#include "DialogueUI.h"
#include "Components/TextBlock.h"
#include "Main.h"
#include "MainPlayerController.h"
#include "Kismet/GameplayStatics.h"


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
    UE_LOG(LogTemp, Log, TEXT("%s, unitaial"), *InitialMessage);

    OutputMessage = "";

    iLetter = 0;

    NPCText->SetText(FText::FromString(""));
    CharacterNameText->SetText(FText::FromString(Dialogue[RowIndex]->CharacterName.ToString()));

    GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UDialogueUI::OnAnimationTimerCompleted, 0.1f, false);
}


void UDialogueUI::OnTimerCompleted()
{
    GetWorld()->GetTimerManager().ClearTimer(TimerHandle);

    AnimateMessage(Dialogue[RowIndex]->Messages[MessageIndex].ToString());
}

void UDialogueUI::OnAnimationTimerCompleted()
{
    GetWorld()->GetTimerManager().ClearTimer(TimerHandle);

    OutputMessage.AppendChar(InitialMessage[iLetter]);

    NPCText->SetText(FText::FromString(OutputMessage));

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

    Main = Cast<AMain>(UGameplayStatics::GetPlayerCharacter(this, 0));

    MainPlayerController = Cast<AMainPlayerController>(Main->GetController());

    CurrentState = 0;

    CharacterNameText->SetText(FText::FromString(""));
    NPCText->SetText(FText::FromString(""));

    OnResetOptions();

    for (auto it : DialogueTable->GetRowMap())
    {
        FNPCDialogue* Row = (FNPCDialogue*)it.Value;
        
        Dialogue.Add(Row);
    }


    if (Dialogue.Num() > 0)
    {
        CurrentState = 0;
        UE_LOG(LogTemp, Log, TEXT("work"));

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
    if (RowIndex >= Dialogue.Num())
    {
        CurrentState = 0;
        MainPlayerController->RemoveDialogueUI();
        UE_LOG(LogTemp, Log, TEXT("end"));
    }
    if (CurrentState == 1) // The text is being animated, skip
    {
        GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
        NPCText->SetText(FText::FromString(InitialMessage));

        CurrentState = 2;
    }
    else if (CurrentState == 2) // Text completed
    {
        // Get next message
        if ((MessageIndex + 1) < Dialogue[RowIndex]->Messages.Num())
        {
            MessageIndex += 1;
            AnimateMessage(Dialogue[RowIndex]->Messages[MessageIndex].ToString());
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("here"));

            NPCText->SetText(FText::FromString(""));

            if (Dialogue[RowIndex]->PlayerReplies.Num() > 0)
            {
                CharacterNameText->SetText(FText::FromString("player"));

                OnResetOptions();
                NumOfReply = Dialogue[RowIndex]->PlayerReplies.Num();

                for (int i = 0; i < Dialogue[RowIndex]->PlayerReplies.Num(); i++)
                {
                    OnSetOption(i, Dialogue[RowIndex]->PlayerReplies[i].ReplyText);
                }

                SelectedReply = 0;

                CurrentState = 3;
            }
            else
            {
                RowIndex += 1;

                UE_LOG(LogTemp, Log, TEXT("Rwo index here, %d"), RowIndex);
                if ((RowIndex >= 0) && (RowIndex < Dialogue.Num()))
                {
                    NPCText->SetText(FText::FromString(""));

                    MessageIndex = 0;

                    AnimateMessage(Dialogue[RowIndex]->Messages[MessageIndex].ToString());
                }
                else
                {
                    MainPlayerController->RemoveDialogueUI();

                    //RowIndex = 0;
                    CurrentState = 0;
                    UE_LOG(LogTemp, Log, TEXT("this"));
                }

                UE_LOG(LogTemp, Log, TEXT("done!"));

            }
        }
    }
    else if (CurrentState == 3)
    {
        // 플레이어 응답에 따라 다르게 하려고 있는 거
        RowIndex = Dialogue[RowIndex]->PlayerReplies[SelectedReply].AnswerIndex;
        OnResetOptions();
        //RowIndex += 1;
        UE_LOG(LogTemp, Log, TEXT("Rwo index here22, %d"), RowIndex);

        if ((RowIndex >= 0) && (RowIndex < Dialogue.Num()))
        {
            NPCText->SetText(FText::FromString(""));

            MessageIndex = 0;
            AnimateMessage(Dialogue[RowIndex]->Messages[MessageIndex].ToString());
        }
        else
        {
            CurrentState = 0;
            MainPlayerController->RemoveDialogueUI();
            UE_LOG(LogTemp, Log, TEXT("this22"));

        }
    }
}


