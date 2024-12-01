// Fill out your copyright notice in the Description page of Project Settings.


#include "System/UIManager.h"
#include "Blueprint/UserWidget.h"
#include "System/GameManager.h"
#include "System/DialogueManager.h"
#include "System/MainPlayerController.h"
#include "Structs/SystemMessageData.h"
#include "Character/Main.h"

const FVector2D DEFAULT_WIDGET_ALIGNMENT(0.f, 0.f);

UUIManager* UUIManager::Instance = nullptr;

void UUIManager::BeginPlay(FString WorldName)
{
    if (GameManager)
    {
        DialogueManager = GameManager->GetDialogueManager();
        MainPlayerController = GameManager->GetMainPlayerController();
    }
    else
    {
        return;
    }

    // 위젯들, 순서 있음
    TSoftClassPtr<UUserWidget> HUDBPClass(FSoftObjectPath(TEXT("/Game/HUDandWigets/HUDOverlay.HUDOverlay_C")));
    TSoftClassPtr<UUserWidget> SystemMessageBPClass(FSoftObjectPath(TEXT("/Game/HUDandWigets/SystemMessage.SystemMessage_C")));
    TSoftClassPtr<UUserWidget> ControlGuideBPClass(FSoftObjectPath(TEXT("/Game/HUDandWigets/ControlGuide.ControlGuide_C")));
    TSoftClassPtr<UUserWidget> MenuBPClass(FSoftObjectPath(TEXT("/Game/HUDandWigets/Menu.Menu_C")));
    TSoftClassPtr<UUserWidget> TargetArrowBPClass(FSoftObjectPath(TEXT("/Game/HUDandWigets/TargetArrow.TargetArrow_C")));
    TSoftClassPtr<UUserWidget> EnemyHPBarBPClass(FSoftObjectPath(TEXT("/Game/HUDandWigets/EnemyHPBar.EnemyHPBar_C")));
    TSoftClassPtr<UUserWidget> FadeInOutBPClass(FSoftObjectPath(TEXT("/Game/HUDandWigets/FadeInOut.FadeInOut_C")));

    if(!WorldName.Contains("2"))
    {
        LoadAndCreateWidget(HUDBPClass, HUDOverlay);
    }

    LoadAndCreateWidget(SystemMessageBPClass, SystemMessage);
    LoadAndCreateWidget(ControlGuideBPClass, ControlGuide);
    LoadAndCreateWidget(MenuBPClass, Menu);

    LoadAndCreateWidget(TargetArrowBPClass, TargetArrow);
    if (TargetArrow)
    {
        TargetArrow->SetAlignmentInViewport(DEFAULT_WIDGET_ALIGNMENT);
    }

    LoadAndCreateWidget(EnemyHPBarBPClass, EnemyHPBar);
    if (EnemyHPBar)
    {
        EnemyHPBar->SetAlignmentInViewport(DEFAULT_WIDGET_ALIGNMENT);
    }

    if (!FadeInOutBPClass.IsValid())
    {
        FadeInOutBPClass.LoadSynchronous();
    }

    if (ensure(FadeInOutBPClass.IsValid()))
    {
        FadeInOutClass = FadeInOutBPClass.Get();
    }

    SystemMessageData = LoadObject<UDataTable>(nullptr, TEXT("/Game/DataTables/DT_SystemMessage"));
}

void UUIManager::LoadAndCreateWidget(TSoftClassPtr<UUserWidget> WidgetClass, UUserWidget*& WidgetInstance)
{
    if (!WidgetClass.IsValid())
    {
        WidgetClass.LoadSynchronous();
    }
    if (ensure(WidgetClass.IsValid()))
    {
        WidgetInstance = CreateWidget<UUserWidget>(GameManager, WidgetClass.Get());
        if (WidgetInstance)
        {
            WidgetInstance->AddToViewport();
            WidgetInstance->SetVisibility(ESlateVisibility::Hidden);
        }
    }
}

void UUIManager::Tick()
{
    if (TargetArrow)
    {
        if (!TargetArrow->IsInViewport() || TargetArrow->GetVisibility() == ESlateVisibility::Hidden)
        {
            return;
        }

        FVector2D PositionInViewport;
        MainPlayerController->ProjectWorldLocationToScreen(EnemyLocation, PositionInViewport);

        PositionInViewport.Y -= 140.f;
        PositionInViewport.X -= 130.f;
        EnemyHPBar->SetPositionInViewport(PositionInViewport);

        PositionInViewport.Y -= 120.f;
        PositionInViewport.X += 60.f;
        TargetArrow->SetPositionInViewport(PositionInViewport);

        FVector2D SizeInViewport = FVector2D(150.f, 15.f);
        EnemyHPBar->SetDesiredSizeInViewport(SizeInViewport);
    }
}

void UUIManager::DisplayHUD()
{
    if (HUDOverlay)
    {
        HUDOverlay->SetVisibility(ESlateVisibility::Visible);
    }
}

void UUIManager::RemoveHUD()
{
    if (HUDOverlay)
    {
        HUDOverlay->SetVisibility(ESlateVisibility::Hidden);
    }
}

void UUIManager::DisplayControlGuide()
{
    if (DialogueManager->GetDialogueState() < EDialogueState::BeforeFirstDungeon || GetSystemMessageNum() < 3)
    {
        return;
    }

    if (IsControlGuideVisible())
    {
        RemoveControlGuide();
        return;
    }
   
    if (ControlGuide)
    {
        if (bMenuVisible || DialogueManager->IsDialogueUIVisible())
        {
            DisplaySystemMessage();
            FSystemMessage* Row = SystemMessageData->FindRow<FSystemMessage>(FName("CANNOT_SHOW_CONTROLGUIDE"), "");
            if (!Row || Row->MessageText.IsEmpty())
            {
                UE_LOG(LogTemp, Warning, TEXT("SystemMessageData row not found"));
                return;
            }
            FText Text = Row->MessageText;
            SystemText = Text;

            FTimerHandle TimerHandle;
            GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateLambda([&]()
            {
                if (bSystemMessageVisible)
                {
                    RemoveSystemMessage();
                }
            }), 2.f, false);
        }
        else
        {
            bControlGuideVisible = true;
            ControlGuide->SetVisibility(ESlateVisibility::Visible);
        }
    }
}

void UUIManager::RemoveControlGuide()
{
    if (ControlGuide)
    {
        bControlGuideVisible = false;
        ControlGuide->SetVisibility(ESlateVisibility::Hidden);
        if (DialogueManager->GetDialogueState() == EDialogueState::BeforeFirstDungeon && SystemMessageNum < 4)
        {
            GameManager->GetPlayer()->SetCanMove(true);
            SetSystemMessage(4);
            DialogueManager->DialogueEndEvents();
        }
    }
}

void UUIManager::DisplayMenu()
{
    if (Menu)
    {
        if (bControlGuideVisible)
        {
            RemoveControlGuide();
        }

        bMenuVisible = true;

        if (DialogueManager->GetDialogueState() < EDialogueState::FirstDungeonStarted 
            || DialogueManager->IsDialogueUIVisible())
        {
            DisplaySystemMessage();
        }

        Menu->SetVisibility(ESlateVisibility::Visible);

        FInputModeGameAndUI InputMode;
        MainPlayerController->SetInputMode(InputMode);
        MainPlayerController->SetMouseCursorVisibility(true);
    }
}

void UUIManager::RemoveMenu()
{
    if (Menu)
    {
        bMenuVisible = false;
        if (bSystemMessageVisible)
        {
            RemoveSystemMessage();
        }

        Menu->SetVisibility(ESlateVisibility::Hidden);

        if (!DialogueManager->IsDialogueUIVisible())
        {
            FInputModeGameOnly InputModeGameOnly;
            MainPlayerController->SetInputMode(InputModeGameOnly);
            MainPlayerController->SetMouseCursorVisibility(true);
        }
    }
}

void UUIManager::ToggleMenu()
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

void UUIManager::DisplaySystemMessage()
{
    if (SystemMessage)
    {
        FText WarningText = FText::GetEmpty();
        if (bMenuVisible && DialogueManager->GetDialogueState() < EDialogueState::FirstDungeonStarted)
        {
            FSystemMessage* Row = SystemMessageData->FindRow<FSystemMessage>(FName("CANNOT_SHOW_CONTROLGUIDE"), "");
            if (!Row || Row->MessageText.IsEmpty())
            {
                UE_LOG(LogTemp, Warning, TEXT("SystemMessageData row not found"));
                return;
            }
            WarningText = Row->MessageText;
        }
        if (bMenuVisible && DialogueManager->IsDialogueUIVisible())
        {
            FSystemMessage* Row = SystemMessageData->FindRow<FSystemMessage>(FName("CANNOT_SAVE_DURING_DIALOGUE"), "");
            if (!Row || Row->MessageText.IsEmpty())
            {
                UE_LOG(LogTemp, Warning, TEXT("SystemMessageData row not found"));
                return;
            }
            WarningText = Row->MessageText;
        }

        if(!WarningText.IsEmpty())
        {
            SystemText = WarningText;
        }

        bSystemMessageVisible = true;
        SystemMessage->SetVisibility(ESlateVisibility::Visible);
    }
}

void UUIManager::RemoveSystemMessage()
{
    if (SystemMessage)
    {
        SystemMessage->SetVisibility(ESlateVisibility::Hidden);
        bSystemMessageVisible = false;
    }
}

void UUIManager::SetSystemMessage(int MessageNum)
{
    if (!SystemMessageData)
    {
        return;
    }

    FName Index = FName(*FString::FromInt(MessageNum));
    FText Text = SystemMessageData->FindRow<FSystemMessage>(Index, "")->MessageText;
    if (Text.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("SystemMessageData row not found for %s"), *Index.ToString());
        return;
    }
    SystemText = Text;
    SystemMessageNum = MessageNum;
    DisplaySystemMessage();
}

void UUIManager::DisplayTargetArrow()
{
    if (TargetArrow)
    {
        bTargetArrowVisible = true;
        TargetArrow->SetVisibility(ESlateVisibility::Visible);
    }
}

void UUIManager::RemoveTargetArrow()
{
    if (TargetArrow)
    {
        bTargetArrowVisible = false;
        TargetArrow->SetVisibility(ESlateVisibility::Hidden);
    }
}

void UUIManager::DisplayEnemyHPBar()
{
    if (EnemyHPBar)
    {
        bEnemyHPBarVisible = true;
        EnemyHPBar->SetVisibility(ESlateVisibility::Visible);
    }
}

void UUIManager::RemoveEnemyHPBar()
{
    if (EnemyHPBar)
    {
        bEnemyHPBarVisible = false;
        EnemyHPBar->SetVisibility(ESlateVisibility::Hidden);
    }
}

void UUIManager::FadeAndDialogue()
{
    if (FadeInOutClass)
    {
        FadeInOut = CreateWidget<UUserWidget>(GameManager, FadeInOutClass);

        if (FadeInOut)
        {
            bIsFading = true;

            const EDialogueState DialogueState = DialogueManager->GetDialogueState();
            if (DialogueState == EDialogueState::FoodTableEvent 
                || DialogueState == EDialogueState::AfterCombatWithBoss
                || DialogueState == EDialogueState::GetMissionItem)
            {
                GameManager->GetPlayer()->SetCanMove(false);
                MainPlayerController->SetCinematicMode(true, true, true);
            }
            MainPlayerController->FadeOutEvent();
        }
    }
}

UUserWidget* UUIManager::GetFadeInOut()
{
    if (!FadeInOut && FadeInOutClass)
    {
        FadeInOut = CreateWidget<UUserWidget>(GameManager, FadeInOutClass);
    }
    return FadeInOut;
}

void UUIManager::CreateFadeWidget(bool bExecuteFadeOutEvent)
{
    if (FadeInOutClass)
    {
        FadeInOut = CreateWidget<UUserWidget>(GameManager, FadeInOutClass);
        
        if (FadeInOut)
        {
            FadeInOut->AddToViewport();

            if(bExecuteFadeOutEvent)
            {
                MainPlayerController->FadeOutEvent();
            }
        }
    }
}
