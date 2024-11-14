// Fill out your copyright notice in the Description page of Project Settings.


#include "System/UIManager.h"
#include "Blueprint/UserWidget.h"
#include "System/GameManager.h"
#include "System/DialogueManager.h"
#include "System/MainPlayerController.h"
#include "Character/Main.h"
#include "Structs/SystemMessageData.h"


UUIManager* UUIManager::Instance = nullptr;

void UUIManager::BeginPlay()
{
    if (GameManager)
    {
        DialogueManager = GameManager->GetDialogueManager();
        MainPlayerController = GameManager->GetMainPlayerController();
    }
    else return;

    // 위젯들, 순서 있음
    TSoftClassPtr<UUserWidget> HUDBPClass(FSoftObjectPath(TEXT("/Game/HUDandWigets/HUDOverlay.HUDOverlay_C")));
    TSoftClassPtr<UUserWidget> SystemMessageBPClass(FSoftObjectPath(TEXT("/Game/HUDandWigets/SystemMessage.SystemMessage_C")));
    TSoftClassPtr<UUserWidget> ControlGuideBPClass(FSoftObjectPath(TEXT("/Game/HUDandWigets/ControlGuide.ControlGuide_C")));
    TSoftClassPtr<UUserWidget> MenuBPClass(FSoftObjectPath(TEXT("/Game/HUDandWigets/Menu.Menu_C")));
    TSoftClassPtr<UUserWidget> TargetArrowBPClass(FSoftObjectPath(TEXT("/Game/HUDandWigets/TargetArrow.TargetArrow_C")));
    TSoftClassPtr<UUserWidget> EnemyHPBarBPClass(FSoftObjectPath(TEXT("/Game/HUDandWigets/EnemyHPBar.EnemyHPBar_C")));
    TSoftClassPtr<UUserWidget> FadeInOutBPClass(FSoftObjectPath(TEXT("/Game/HUDandWigets/FadeInOut.FadeInOut_C")));

    if(!GetWorld()->GetName().Contains("2"))
    {
        if (!HUDBPClass.IsValid())
            HUDBPClass.LoadSynchronous();
        if (ensure(HUDBPClass.IsValid()))
        {
            HUDOverlay = CreateWidget<UUserWidget>(GameManager, HUDBPClass.Get());
            if (HUDOverlay)
            {
                HUDOverlay->AddToViewport();
                HUDOverlay->SetVisibility(ESlateVisibility::Hidden);
            }
        }
    }

    if (!SystemMessageBPClass.IsValid())
        SystemMessageBPClass.LoadSynchronous();

    if (ensure(SystemMessageBPClass.IsValid()))
    {
        SystemMessage = CreateWidget<UUserWidget>(GameManager, SystemMessageBPClass.Get());
        if (SystemMessage)
        {
            SystemMessage->AddToViewport();
            SystemMessage->SetVisibility(ESlateVisibility::Hidden);
        }
    }

    if (!ControlGuideBPClass.IsValid())
        ControlGuideBPClass.LoadSynchronous();

    if (ensure(ControlGuideBPClass.IsValid()))
    {
        ControlGuide = CreateWidget<UUserWidget>(GameManager, Cast <UClass>(ControlGuideBPClass.Get()));
        if (ControlGuide)
        {
            ControlGuide->AddToViewport();
            ControlGuide->SetVisibility(ESlateVisibility::Hidden);
        }
    }

    if (!MenuBPClass.IsValid())
        MenuBPClass.LoadSynchronous();

    if (ensure(MenuBPClass.IsValid()))
    {
        Menu = CreateWidget<UUserWidget>(GameManager, MenuBPClass.Get());
        if (Menu)
        {
            Menu->AddToViewport();
            Menu->SetVisibility(ESlateVisibility::Hidden);
        }
    }

    if (!TargetArrowBPClass.IsValid())
        TargetArrowBPClass.LoadSynchronous();

    if (ensure(TargetArrowBPClass.IsValid()))
    {
        TargetArrow = CreateWidget<UUserWidget>(GameManager, TargetArrowBPClass.Get());
        if (TargetArrow)
        {
            TargetArrow->AddToViewport();
            TargetArrow->SetVisibility(ESlateVisibility::Hidden);
        }

        FVector2D Alignment(0.f, 0.f);
        TargetArrow->SetAlignmentInViewport(Alignment);
    }

    if (!EnemyHPBarBPClass.IsValid())
        EnemyHPBarBPClass.LoadSynchronous();

    if (ensure(EnemyHPBarBPClass.IsValid()))
    {
        EnemyHPBar = CreateWidget<UUserWidget>(GameManager, EnemyHPBarBPClass.Get());
        if (EnemyHPBar)
        {
            EnemyHPBar->AddToViewport();
            EnemyHPBar->SetVisibility(ESlateVisibility::Hidden);
        }

        FVector2D Alignment(0.f, 0.f);
        EnemyHPBar->SetAlignmentInViewport(Alignment);
    }

    if (!FadeInOutBPClass.IsValid())
        FadeInOutBPClass.LoadSynchronous();

    if (ensure(FadeInOutBPClass.IsValid()))
    {
        FadeInOutClass = FadeInOutBPClass.Get();
    }

    SystemMessageData = LoadObject<UDataTable>(nullptr, TEXT("/Game/DataTables/DT_SystemMessage"));
}

void UUIManager::Tick()
{
    if (TargetArrow)
    {
        if (!TargetArrow->IsInViewport() || TargetArrow->GetVisibility() == ESlateVisibility::Hidden) return;

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

void UUIManager::SetGameManager(UGameManager* Manager)
{
    this->GameManager = Manager;
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
    if (ControlGuide)
    {
        if (bMenuVisible || DialogueManager->IsDialogueUIVisible())
        {
            FTimerHandle TimerHandle;
            DisplaySystemMessage();
            FString text = TEXT("대화 중이거나 메뉴가 활성화된 상태에서는\n조작 매뉴얼을 볼 수 없습니다.");
            SystemText = FText::FromString(text);
            GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateLambda([&]() {

                if (bSystemMessageVisible) RemoveSystemMessage();

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
        if (DialogueManager->GetDialogueNum() == 2 && SystemMessageNum < 4)
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
        if (bControlGuideVisible) RemoveControlGuide();

        bMenuVisible = true;

        if (DialogueManager->GetDialogueNum() < 3 || DialogueManager->IsDialogueUIVisible())
            DisplaySystemMessage();

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
        if (bSystemMessageVisible) RemoveSystemMessage();

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
    if (bMenuVisible) RemoveMenu();
    else DisplayMenu();
}

void UUIManager::DisplaySystemMessage()
{
    if (SystemMessage)
    {
        FString WarningText = "";
        if (bMenuVisible && DialogueManager->GetDialogueNum() < 3)
        {
            WarningText = FString(TEXT("이 곳에선 저장되지 않습니다."));
        }
        if (bMenuVisible && DialogueManager->IsDialogueUIVisible())
        {
            WarningText = FString(TEXT("대화 중엔 저장되지 않습니다."));
        }

        if(WarningText != "")
            SystemText = FText::FromString(WarningText);

        bSystemMessageVisible = true;
        SystemMessage->SetVisibility(ESlateVisibility::Visible);
    }
}

void UUIManager::RemoveSystemMessage()
{
    if (SystemMessage)
    {
        SystemMessage->SetVisibility(ESlateVisibility::Hidden);
        SystemMessageOn = false;
        bSystemMessageVisible = false;
    }
}

void UUIManager::SetSystemMessage(int MessageNum)
{
    if (!SystemMessageData) return;

    FName Index = FName(*FString::FromInt(MessageNum));
    FText text = SystemMessageData->FindRow<FSystemMessage>(Index, "")->MessageText;

    SystemText = text;
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
        // bCanDisplaySpeechBubble = false;
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

            AMain* Player = GameManager->GetPlayer();

            int dialogueNum = DialogueManager->GetDialogueNum();
            if (dialogueNum == 11 || dialogueNum == 18 || dialogueNum == 20) // second dungeon food trap
            {
                Player->SetCanMove(false);
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
                MainPlayerController->FadeOutEvent();
        }
    }
}
