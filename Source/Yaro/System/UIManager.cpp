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

    static ConstructorHelpers::FClassFinder<UUserWidget> ControlGuideBPClass(TEXT("/Game/HUDandWigets/ControlGuide.ControlGuide_C"));
    static ConstructorHelpers::FClassFinder<UUserWidget> MenuBPClass(TEXT("/Game/HUDandWigets/Menu.Menu_C"));
    static ConstructorHelpers::FClassFinder<UUserWidget> SystemMessageBPClass(TEXT("/Game/HUDandWigets/SystemMessage.SystemMessage_C"));
    static ConstructorHelpers::FClassFinder<UUserWidget> TargetArrowBPClass(TEXT("/Game/HUDandWigets/TargetArrow.TargetArrow_C"));
    static ConstructorHelpers::FClassFinder<UUserWidget> EnemyHPBarBPClass(TEXT("/Game/HUDandWigets/EnemyHPBar.EnemyHPBar_C"));

    if (ensure(ControlGuideBPClass.Class != nullptr))
    {
        ControlGuide = CreateWidget<UUserWidget>(GameManager, ControlGuideBPClass.Class);
        if (ControlGuide)
        {
            ControlGuide->AddToViewport();
            ControlGuide->SetVisibility(ESlateVisibility::Hidden);
        }
    }

    if (ensure(MenuBPClass.Class != nullptr))
    {
        Menu = CreateWidget<UUserWidget>(this, MenuBPClass.Class);
        if (Menu)
        {
            Menu->AddToViewport();
            Menu->SetVisibility(ESlateVisibility::Hidden);
        }
    }

    if (ensure(SystemMessageBPClass.Class != nullptr))
    {
        SystemMessage = CreateWidget<UUserWidget>(this, SystemMessageBPClass.Class);
        if (SystemMessage)
        {
            SystemMessage->AddToViewport();
            SystemMessage->SetVisibility(ESlateVisibility::Hidden);
        }
    }

    if (ensure(TargetArrowBPClass.Class != nullptr))
    {
        TargetArrow = CreateWidget<UUserWidget>(this, TargetArrowBPClass.Class);
        if (TargetArrow)
        {
            TargetArrow->AddToViewport();
            TargetArrow->SetVisibility(ESlateVisibility::Hidden);
        }

        FVector2D Alignment(0.f, 0.f);
        TargetArrow->SetAlignmentInViewport(Alignment);
    }

    if (ensure(EnemyHPBarBPClass.Class != nullptr))
    {
        EnemyHPBar = CreateWidget<UUserWidget>(this, EnemyHPBarBPClass.Class);
        if (EnemyHPBar)
        {
            EnemyHPBar->AddToViewport();
            EnemyHPBar->SetVisibility(ESlateVisibility::Hidden);
        }

        FVector2D Alignment(0.f, 0.f);
        EnemyHPBar->SetAlignmentInViewport(Alignment);
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
            FString text = TEXT("��ȭ ���̰ų� �޴��� Ȱ��ȭ�� ���¿�����\n���� �Ŵ����� �� �� �����ϴ�.");
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
        FString text;
        if (bMenuVisible && DialogueManager->GetDialogueNum() < 3)
        {
            text = FString(TEXT("�� ������ ������� �ʽ��ϴ�."));
        }
        if (bMenuVisible && DialogueManager->IsDialogueUIVisible())
        {
            text = FString(TEXT("��ȭ �߿� ������� �ʽ��ϴ�."));
        }

        SystemText = FText::FromString(text);

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
            text = FString(TEXT("FŰ�� �����ų� ���콺 ���� ��ư�� Ŭ���Ͽ�\n��ȭ�� ������ �� �ֽ��ϴ�."));
            break;
        case 2:
            text = FString(TEXT("MŰ�� ���� ���� �Ŵ����� Ȯ���ϰ�\n�ٽ� MŰ�� ���� ���� �Ŵ����� ��������."));
            break;
        case 3:
            text = FString(TEXT("���� ������ �����̿��� ���콺 ���� ��ư�� Ŭ���Ͽ�\n�����̸� ����ϼ���."));
            break;
        case 4:
            text = FString(TEXT("��Ż�� �̿��� �������� �̵��ϼ���."));
            break;
        case 5:
            text = FString(TEXT("����Ű 1�� �� ���� ������ �� �� �ֽ��ϴ�.\n�غ� �Ǿ��ٸ� GŰ�� ��������."));
            break;
        case 6:
            text = FString(TEXT("���� 2�� �Ǿ����ϴ�!\n���� ����Ű 2�� �� ��° ������ �� �� �ֽ��ϴ�."));
            break;
        case 7:
            text = FString(TEXT("���� 3�� �Ǿ����ϴ�!\n���� ����Ű 3���� �� ��° ������ �� �� �ֽ��ϴ�."));
            break;
        case 8:
            text = FString(TEXT("�����մϴ�!\n���� ����Ű 4�� �� ��° ������ �� �� �ֽ��ϴ�."));
            break;
        case 9:
            text = FString(TEXT("���� 5�� �޼��߽��ϴ�!\n���� ����Ű 5�� �ټ� ��° ������ �� �� �ֽ��ϴ�."));
            break;
        case 10:
            text = FString(TEXT("���콺 ���� ��ư�� Ŭ���Ͽ� ���� �ݰ�\n�����̴� �� �����̼� ���콺 ���� ��ư�� Ŭ���Ͽ�\n���� ��������."));
            break;
        case 11:
            text = FString(TEXT("����� ���� ���� ��ҷ� �̵��ϼ���."));
            break;
        case 12:
            text = FString(TEXT("���ݺ��� ������ ������ ��������\n������ ���� �ʽ��ϴ�."));
            break;
        case 13:
            text = FString(TEXT("��Ż�� �̿��� ���� �Ա��� ���ư�����."));
            break;
        case 14:
            text = FString(TEXT("���콺 ���� ��ư�� Ŭ���Ͽ�\n��� �����õ���� ì�⼼��."));
            break;
        case 15:
            text = FString(TEXT("���� 5�� �Ǿ����ϴ�!\n����Ű 1~5�� ��� ������ �� �� �ֽ��ϴ�."));
            break;
        case 16:
            text = FString(TEXT("��ü ȸ�� ������ �޾ҽ��ϴ�!\n�ʿ��� �� PŰ�� ���� ����ϼ���."));
            break;
    }

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
