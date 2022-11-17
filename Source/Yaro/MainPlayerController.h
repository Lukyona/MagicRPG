// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MainPlayerController.generated.h"

UCLASS()
class YARO_API AMainPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AMainPlayerController();

	UPROPERTY(BlueprintReadWrite)
	class AMain* Main;

	//Player can Targeting, then TargetArrow appear on Targeted enemy
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<UUserWidget> WTargetArrow;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Widgets")
	UUserWidget* TargetArrow;

	bool bTargetArrowVisible;

	void DisplayTargetArrow();
	void RemoveTargetArrow();

	FVector EnemyLocation;

	int WhichKeyDown(); // Find out pressed key, this will be SkillNum


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<UUserWidget> WEnemyHPBar;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Widgets")
	UUserWidget* EnemyHPBar;

	bool bEnemyHPBarVisible;

	void DisplayEnemyHPBar();
	void RemoveEnemyHPBar();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<UUserWidget> WMenu;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Widgets")
	UUserWidget* Menu;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool bMenuVisible;

	void DisplayMenu();
	void RemoveMenu();
	void ToggleMenu();

	UFUNCTION(BlueprintCallable)
	void DisplayDialogueUI();

    UFUNCTION(BlueprintCallable)
	void RemoveDialogueUI();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool bDialogueUIVisible;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	class UDialogueUI* DialogueUI;

	UPROPERTY(VisibleAnywhere, Category = "Dialogue")
	TSubclassOf<class UUserWidget> DialogueUIClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	int DialogueNum = 0; // 0 - intro

    UFUNCTION(BlueprintCallable)
	void DialogueEvents();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
    TSubclassOf<UUserWidget> WFadeInOut;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Widgets")
    UUserWidget* FadeInOut;

    UFUNCTION(BlueprintImplementableEvent, Category = "Fade Events")
    void FadeOut();

	UFUNCTION(BlueprintCallable)
	void FadeAndDialogue();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fade Events")
	bool bFadeOn = false;

    UFUNCTION(BlueprintCallable)
	void SetPositions();

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    FText SystemText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
    TSubclassOf<UUserWidget> WSystemMessage;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Widgets")
    UUserWidget* SystemMessage;

    UFUNCTION(BlueprintCallable)
    void DisplaySystemMessage();

    UFUNCTION(BlueprintCallable)
    void RemoveSystemMessage();

	FString text;

	bool SystemMessageOn = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    bool bSystemMessageVisible;

	UPROPERTY(BlueprintReadWrite)
	int SystemMessageNum;

    UFUNCTION(BlueprintCallable)
	void SetSystemMessage();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
    TSubclassOf<UUserWidget> WManual;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Widgets")
    UUserWidget* Manual;

	void DisplayManual();

    void RemoveManual();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool bManualVisible = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool bFallenPlayer = false; // 플레이어가 던전 범위 밖으로 추락했을 때 true

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int FallingCount = 0;

	UFUNCTION(BlueprintImplementableEvent)
	void CalculateDialogueDistance();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool bCalculateOn;


	bool bSpeechBuubbleVisible;

	UFUNCTION(BlueprintCallable)
	void DisplaySpeechBuubble(class AYaroCharacter* npc);

	void RemoveSpeechBuubble();

	FVector SBLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool bCanDisplaySpeechBubble = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class AActor> SpeechBubble_BP;

	class AActor* SpeechBubble;

	FTimerHandle Timer;


protected:
	// Dialogue data tables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	class UDataTable* IntroDialogue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	class UDataTable* DungeonDialogue1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    class UDataTable* DungeonDialogue2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    class UDataTable* DungeonDialogue3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    class UDataTable* DungeonDialogue4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    class UDataTable* DungeonDialogue5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    class UDataTable* DungeonDialogue6;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	class UDataTable* FinalDialogue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    class UDataTable* SpawnDialogue;

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;
};
