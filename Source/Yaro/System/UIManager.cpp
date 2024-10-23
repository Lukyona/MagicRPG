// Fill out your copyright notice in the Description page of Project Settings.


#include "System/UIManager.h"
#include "Blueprint/UserWidget.h"
#include "System/GameManager.h"
#include "System/DialogueManager.h"
#include "System/MainPlayerController.h"
#include "Character/Main.h"


UUIManager* UUIManager::Instance = nullptr;

void UUIManager::Init()
{
    GameManager = Cast<UGameManager>(GetWorld()->GetGameInstance());
    if (GameManager)
    {
        DialogueManager = GameManager->GetDialogueManager();
        MainPlayerController = GameManager->GetMainPlayerController();
    }
    else return;

    TSoftObjectPtr<UClass> ControlGuideBPClass(FSoftObjectPath(TEXT("/Game/HUDandWigets/ControlGuide.ControlGuide_C")));
    TSoftObjectPtr<UClass> MenuBPClass(FSoftObjectPath(TEXT("/Game/HUDandWigets/Menu.Menu_C")));
    TSoftObjectPtr<UClass> SystemMessageBPClass(FSoftObjectPath(TEXT("/Game/HUDandWigets/SystemMessage.SystemMessage_C")));
    TSoftObjectPtr<UClass> TargetArrowBPClass(FSoftObjectPath(TEXT("/Game/HUDandWigets/TargetArrow.TargetArrow_C")));
    TSoftObjectPtr<UClass> EnemyHPBarBPClass(FSoftObjectPath(TEXT("/Game/HUDandWigets/EnemyHPBar.EnemyHPBar_C")));
    TSoftObjectPtr<UClass> FadeInOutBPClass(FSoftObjectPath(TEXT("/Game/HUDandWigets/FadeInOut.FadeInOut_C")));

    if (ensure(ControlGuideBPClass.IsValid()))
    {
        ControlGuideBPClass.LoadSynchronous();
        ControlGuide = CreateWidget<UUserWidget>(GameManager, ControlGuideBPClass.Get());
        if (ControlGuide)
        {
            ControlGuide->AddToViewport();
            ControlGuide->SetVisibility(ESlateVisibility::Hidden);
        }
    }

    if (ensure(MenuBPClass.IsValid()))
    {
        MenuBPClass.LoadSynchronous();
        Menu = CreateWidget<UUserWidget>(GameManager, MenuBPClass.Get());
        if (Menu)
        {
            Menu->AddToViewport();
            Menu->SetVisibility(ESlateVisibility::Hidden);
        }
    }

    if (ensure(SystemMessageBPClass.IsValid()))
    {
        SystemMessageBPClass.LoadSynchronous();
        SystemMessage = CreateWidget<UUserWidget>(GameManager, SystemMessageBPClass.Get());
        if (SystemMessage)
        {
            SystemMessage->AddToViewport();
            SystemMessage->SetVisibility(ESlateVisibility::Hidden);
        }
    }

    if (ensure(TargetArrowBPClass.IsValid()))
    {
        TargetArrowBPClass.LoadSynchronous();
        TargetArrow = CreateWidget<UUserWidget>(GameManager, TargetArrowBPClass.Get());
        if (TargetArrow)
        {
            TargetArrow->AddToViewport();
            TargetArrow->SetVisibility(ESlateVisibility::Hidden);
        }

        FVector2D Alignment(0.f, 0.f);
        TargetArrow->SetAlignmentInViewport(Alignment);
    }

    if (ensure(EnemyHPBarBPClass.IsValid()))
    {
        EnemyHPBarBPClass.LoadSynchronous();
        EnemyHPBar = CreateWidget<UUserWidget>(GameManager, EnemyHPBarBPClass.Get());
        if (EnemyHPBar)
        {
            EnemyHPBar->AddToViewport();
            EnemyHPBar->SetVisibility(ESlateVisibility::Hidden);
        }

        FVector2D Alignment(0.f, 0.f);
        EnemyHPBar->SetAlignmentInViewport(Alignment);
    }

    if (ensure(FadeInOutBPClass.IsValid()))
    {
        FadeInOutBPClass.LoadSynchronous();
        FadeInOutClass = FadeInOutBPClass.Get();
    }
}

void UUIManager::Tick()
{
    if (TargetArrow)
    {
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
        if (DialogueManager->GetDialogueNum() == 2 && SystemMessageNum != 4)
        {
            GameManager->GetPlayer()->SetCanMove(true);
            SetSystemMessage(3);

            DialogueManager->DialogueEvents();
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
    FString text;
    switch (MessageNum)
    {
    case 1:
        text = FString(TEXT("F키를 누르거나 마우스 왼쪽 버튼을 클릭하여\n대화를 진행할 수 있습니다."));
        break;
    case 2:
        text = FString(TEXT("M키를 눌러 조작 매뉴얼을 확인하고\n다시 M키를 눌러 조작 매뉴얼을 닫으세요."));
        break;
    case 3:
        text = FString(TEXT("이제 지팡이 가까이에서 마우스 왼쪽 버튼을 클릭하여\n지팡이를 장비하세요."));
        break;
    case 4:
        text = FString(TEXT("포탈을 이용해 던전으로 이동하세요."));
        break;
    case 5:
        text = FString(TEXT("숫자키 1로 한 가지 마법을 쓸 수 있습니다.\n준비가 되었다면 G키를 누르세요."));
        break;
    case 6:
        text = FString(TEXT("레벨 2가 되었습니다!\n이제 숫자키 2로 두 번째 마법을 쓸 수 있습니다."));
        break;
    case 7:
        text = FString(TEXT("레벨 3이 되었습니다!\n이제 숫자키 3으로 세 번째 마법을 쓸 수 있습니다."));
        break;
    case 8:
        text = FString(TEXT("축하합니다!\n이제 숫자키 4로 네 번째 마법을 쓸 수 있습니다."));
        break;
    case 9:
        text = FString(TEXT("레벨 5를 달성했습니다!\n이제 숫자키 5로 다섯 번째 마법을 쓸 수 있습니다."));
        break;
    case 10:
        text = FString(TEXT("마우스 왼쪽 버튼을 클릭하여 돌을 줍고\n움직이는 돌 가까이서 마우스 왼쪽 버튼을 클릭하여\n돌을 놓으세요."));
        break;
    case 11:
        text = FString(TEXT("계단을 통해 다음 장소로 이동하세요."));
        break;
    case 12:
        text = FString(TEXT("지금부터 전투가 끝나는 시점까지\n저장이 되지 않습니다."));
        break;
    case 13:
        text = FString(TEXT("포탈을 이용해 던전 입구로 돌아가세요."));
        break;
    case 14:
        text = FString(TEXT("마우스 왼쪽 버튼을 클릭하여\n디비눔 프레시디움을 챙기세요."));
        break;
    case 15:
        text = FString(TEXT("레벨 5가 되었습니다!\n숫자키 1~5로 모든 마법을 쓸 수 있습니다."));
        break;
    case 16:
        text = FString(TEXT("전체 회복 포션을 받았습니다!\n필요할 때 P키를 눌러 사용하세요."));
        break;
    }

    SystemText = FText::FromString(text);
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
            uint32 FallCount = Player->GetFallCount();
            if (Player->IsFallenInDungeon() && ((FallCount == 0 || FallCount >= 2)))
            {
                if (FallCount != 6)
                    Player->SetFallCount(FallCount+1);
            }

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
