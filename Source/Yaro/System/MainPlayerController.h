// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MainPlayerController.generated.h"

class UGameManager;
class UDialogueManager;
class UUIManager;
class UNPCManager;
class AMain;

UCLASS()
class YARO_API AMainPlayerController : public APlayerController
{
    GENERATED_BODY()

protected:
    virtual void BeginPlay() override;

    // BP에서 사용
    UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    UGameManager* GameManager;
    UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    UDialogueManager* DialogueManager;
    UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    UNPCManager* NPCManager;

    // BeginPlay() 호출 위해
    UPROPERTY()
    UUIManager* UIManager;
public:
    UPROPERTY(BlueprintReadOnly)
    AMain* MainPlayer;

    void SetMouseCursorVisibility(bool Value) { bShowMouseCursor = Value; }

    int32 WhichKeyDown(); // Find out pressed key, this will be SkillNum  

    UFUNCTION(BlueprintImplementableEvent)
    void FadeOutEvent();

};
