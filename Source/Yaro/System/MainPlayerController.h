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
    class UGameManager* GameManager;
    UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    class UDialogueManager* DialogueManager;
    UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    class UNPCManager* NPCManager;

    // BeginPlay() 호출 위해
    UPROPERTY()
    class UUIManager* UIManager;
public:
    UPROPERTY(BlueprintReadWrite)
    AMain* MainPlayer;

    void SetMouseCursorVisibility(bool Value) { bShowMouseCursor = Value; }

    int32 WhichKeyDown(); // Find out pressed key, this will be SkillNum  

    UFUNCTION(BlueprintImplementableEvent)
    void FadeOutEvent();

};
