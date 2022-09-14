// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MainPlayerController.generated.h"

/**
 * 
 */
//UENUM(BlueprintType)
//enum class EDialogueState :uint8
//{
//	EDS_Speak		UMETA(DeplayName = "NPC_Speak"),
//	EDS_Reply			UMETA(DeplayName = "Reply")
//};

UCLASS()
class YARO_API AMainPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AMainPlayerController();

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


	void DisplayDialogueUI();
	void RemoveDialogueUI();
	void ToggleDialogueUI();

	UPROPERTY(VisibleAnywhere)
	bool bDialogueUIVisible;


	UPROPERTY(EditAnywhere, Category = "Dialogue")
	class UDialogueUI* DialogueUI;

	UPROPERTY(VisibleAnywhere, Category = "Dialogue")
	TSubclassOf<class UUserWidget> DialogueUIClass;

	FORCEINLINE UDialogueUI* GetDialogueUI() { return DialogueUI; };

	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue")
	//EDialogueState DialogueState;

	//UFUNCTION(BlueprintCallable)
	//void SetDialogueState(EDialogueState State);


protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	class UDataTable* IntroDialogue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	class UDataTable* DungeonDialogue1;

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;
};
