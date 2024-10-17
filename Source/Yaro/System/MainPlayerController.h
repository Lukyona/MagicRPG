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

	UPROPERTY(BlueprintReadWrite)
	class AMain* Main;

	UPROPERTY()
	class UGameManager* GameManager;

	UPROPERTY()
	class UDialogueManager* DialogueManager; 


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
