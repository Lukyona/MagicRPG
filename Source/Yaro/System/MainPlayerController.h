// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MainPlayerController.generated.h"

UCLASS()
class YARO_API AMainPlayerController : public APlayerController
{
    GENERATED_BODY()

protected:
    UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    class UGameManager* GameManager;

    UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
        class UDialogueManager* DialogueManager;

    UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
        class UUIManager* UIManager;

    UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
        class UNPCManager* NPCManager;

public:

    UPROPERTY(BlueprintReadWrite)
        class AMain* MainPlayer;

    void SetMouseCursorVisibility(bool Value) { bShowMouseCursor = Value; }

    int WhichKeyDown(); // Find out pressed key, this will be SkillNum  

    UFUNCTION(BlueprintImplementableEvent, Category = "Fade Events")
        void FadeOut();

protected:
    virtual void BeginPlay() override;

    virtual void Tick(float DeltaTime) override;
};
