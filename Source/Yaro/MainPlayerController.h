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
	TSubclassOf<UUserWidget> WPauseMenu;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Widgets")
	UUserWidget* PauseMenu;

	bool bPauseMenuVisible;

	void DisplayPauseMenu();
	void RemovePauseMenu();
	void TogglePauseMenu();

	UFUNCTION(BlueprintCallable)
	void DisplayDialogueUI();

	void RemoveDialogueUI();

	UPROPERTY(VisibleAnywhere)
	bool bDialogueUIVisible;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	class UDialogueUI* DialogueUI;

	UPROPERTY(VisibleAnywhere, Category = "Dialogue")
	TSubclassOf<class UUserWidget> DialogueUIClass;

	FORCEINLINE UDialogueUI* GetDialogueUI() { return DialogueUI; };

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Dialogue")
	int DialogueNum; // 0 - intro

	void DialogueEvents();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
    TSubclassOf<UUserWidget> WFadeInOut;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Widgets")
    UUserWidget* FadeInOut;

    UFUNCTION(BlueprintImplementableEvent, Category = "Fade Events")
    void FadeOut();

	void FadeAndDialogue();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fade Events")
	bool bFadeOn = false;

    UFUNCTION(BlueprintCallable)
	void SetPositions();

protected:
	// Dialogue data tables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	class UDataTable* IntroDialogue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	class UDataTable* DungeonDialogue1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    class UDataTable* DungeonDialogue2;

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;
};
