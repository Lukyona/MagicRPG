// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MainPlayerController.generated.h"

UCLASS()
class YARO_API AMainPlayerController : public APlayerController
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    class UGameManager* GameManager;

    UPROPERTY()
        class UDialogueManager* DialogueManager;

    UPROPERTY()
        class UUIManager* UIManager;

public:

    UPROPERTY(BlueprintReadWrite)
        class AMain* Main;

    void SetMouseCursorVisibility(bool Value) { bShowMouseCursor = Value; }

    int WhichKeyDown(); // Find out pressed key, this will be SkillNum  


protected:
    virtual void BeginPlay() override;

    virtual void Tick(float DeltaTime) override;
};
