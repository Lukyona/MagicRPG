// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "UIManager.generated.h"

/**
 *
 */

class UGameManager;
class UDialogueManager;
class AMainPlayerController;
class UDataTable;

UCLASS(BlueprintType)
class YARO_API UUIManager : public UObject
{
    GENERATED_BODY()

    friend class UGameManager;

    static UUIManager* Instance;

    UPROPERTY()
    UGameManager* GameManager;
    UPROPERTY()
    UDialogueManager* DialogueManager;
    UPROPERTY()
    AMainPlayerController* MainPlayerController;

    // UI Managements
    UPROPERTY()
    UUserWidget* HUDOverlay;

    UPROPERTY()
    UUserWidget* ControlGuide;
    bool bControlGuideVisible = false;

    UPROPERTY()
    UUserWidget* Menu;
    bool bMenuVisible = false;

    UPROPERTY()
    UDataTable* SystemMessageData;
    UPROPERTY()
    FText SystemText;
    UPROPERTY()
    UUserWidget* SystemMessage;
    bool bSystemMessageVisible = false;
    int32 SystemMessageNum;

    //Player can Targeting, then TargetArrow appear on Targeted enemy
    UPROPERTY()
    UUserWidget* TargetArrow;
    bool bTargetArrowVisible = false;

    UPROPERTY()
    UUserWidget* EnemyHPBar;
    bool bEnemyHPBarVisible = false;
    FVector EnemyLocation;

    UPROPERTY()
    TSubclassOf<UUserWidget> FadeInOutClass;
    UPROPERTY()
    UUserWidget* FadeInOut;
    bool bIsFading = false;

    void LoadAndCreateWidget(TSoftClassPtr<UUserWidget> WidgetClass, UUserWidget*& WidgetInstance);
public:
    static UUIManager* CreateInstance(UGameInstance* Outer)
    {
        if (Instance == nullptr)
        {
            Instance = NewObject<UUIManager>(Outer, UUIManager::StaticClass());
        }
        return Instance;
    }

    //Getters and setters
    void SetGameManager(UGameManager* Manager)
    {
        GameManager = Manager;
    }

    bool IsControlGuideVisible()
    {
        return bControlGuideVisible;
    }

    bool IsMenuVisible() 
    {
        return bMenuVisible; 
    }

    UFUNCTION(BlueprintCallable)
    bool IsSystemMessageVisible() const 
    {
        return bSystemMessageVisible; 
    }
    UFUNCTION(BlueprintCallable)
    void SetSystemMessage(int MessageNum);
    UFUNCTION(BlueprintCallable)
    FText GetSystemText() const 
    {
        return SystemText; 
    }
    int32 GetSystemMessageNum() const 
    {
        return SystemMessageNum; 
    }

    void SetEnemyLocation(FVector Location) 
    {
        EnemyLocation = Location; 
    }
    bool IsTargetArrowVisible() 
    {
        return bTargetArrowVisible; 
    }

    bool IsFading() 
    {
        return bIsFading; 
    }
    void SetIsFading(bool Value) 
    {
        bIsFading = Value; 
    }
    UFUNCTION(BlueprintCallable, BlueprintPure)
    UUserWidget* GetFadeInOut();


    //Core Methods
    void BeginPlay(FString WorldName);
    void Tick();

    UFUNCTION(BlueprintCallable)
    void DisplayHUD();
    void RemoveHUD();

    void DisplayControlGuide();
    void RemoveControlGuide();

    void DisplayMenu();
    void RemoveMenu();
    void ToggleMenu();

    UFUNCTION(BlueprintCallable)
    void DisplaySystemMessage();
    UFUNCTION(BlueprintCallable)
    void RemoveSystemMessage();

    void DisplayTargetArrow();
    void RemoveTargetArrow();

    void DisplayEnemyHPBar();
    void RemoveEnemyHPBar();

    UFUNCTION(BlueprintCallable)
    void FadeAndDialogue();

    UFUNCTION(BlueprintCallable)
    void CreateFadeWidget(bool bExecuteFadeOutEvent);
};
