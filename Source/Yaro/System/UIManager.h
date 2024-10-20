// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "UIManager.generated.h"

/**
 *
 */
UCLASS()
class YARO_API UUIManager : public UObject
{
    GENERATED_BODY()

        static UUIManager* Instance;

    UPROPERTY()
        class UGameManager* GameManager;

    UPROPERTY()
        class UDialogueManager* DialogueManager;

    UPROPERTY()
        class AMainPlayerController* MainPlayerController;


    UPROPERTY()
        UUserWidget* ControlGuide;

    bool bControlGuideVisible = false;


    UPROPERTY()
        UUserWidget* Menu;

    bool bMenuVisible = false;


    UPROPERTY()
        FText SystemText;

    UPROPERTY()
        UUserWidget* SystemMessage;

    bool SystemMessageOn = false;

    bool bSystemMessageVisible = false;

    UPROPERTY()
        uint8 SystemMessageNum;


    //Player can Targeting, then TargetArrow appear on Targeted enemy
    UPROPERTY()
        UUserWidget* TargetArrow;

    bool bTargetArrowVisible = false;

    UPROPERTY()
        UUserWidget* EnemyHPBar;

    bool bEnemyHPBarVisible = false;

    FVector EnemyLocation;




    UPROPERTY()
        UUserWidget* FadeInOut;

    bool bFadeOn = false;


public:
    static UUIManager* CreateInstance(UObject* Outer)
    {
        if (Instance == nullptr)
        {
            Instance = NewObject<UUIManager>(Outer, UUIManager::StaticClass());
        }
        return Instance;
    }

    void Init();
    void Tick();

    void DisplayControlGuide();
    void RemoveControlGuide();
    bool IsControlGuideVisible() { return bControlGuideVisible; }

    void DisplayMenu();
    void RemoveMenu();
    void ToggleMenu();
    bool IsMenuVisible() { return bMenuVisible; }

    UFUNCTION(BlueprintCallable)
        void DisplaySystemMessage();
    UFUNCTION(BlueprintCallable)
        void RemoveSystemMessage();
    bool IsSystemMessageVisible() { return bSystemMessageVisible; }

    UFUNCTION(BlueprintCallable)
        void SetSystemMessage(int MessageNum);
    uint8 GetSystemMessageNum() const { return SystemMessageNum; }

    void SetEnemyLocation(FVector Location) { EnemyLocation = Location; }
    void DisplayTargetArrow();
    void RemoveTargetArrow();
    bool IsTargetArrowVisible() { return bTargetArrowVisible; }


    void DisplayEnemyHPBar();
    void RemoveEnemyHPBar();

    UFUNCTION(BlueprintImplementableEvent, Category = "Fade Events")
        void FadeOut();

    UFUNCTION(BlueprintCallable)
        void FadeAndDialogue();
};
