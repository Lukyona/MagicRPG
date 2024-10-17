// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "GameManager.generated.h"

/**
 * 
 */
UCLASS()
class YARO_API UGameManager : public UGameInstance
{
	GENERATED_BODY()

	class AMain* Player;
	class AMainPlayerController* MainPlayerController;

	class UDialogueManager* DialogueManager;
	class UNPCManager* NPCManager;

public:

	UPROPERTY(BlueprintReadWrite, Category = "PlayerData")
		TSubclassOf<APawn> PlayerClass;

	FTimerHandle SaveTimer; 


	UDialogueManager* GetDialogueManager() const { return DialogueManager; }
	UNPCManager* GetNPCManager() const { return NPCManager; }
	AMain* GetPlayer();
	AMainPlayerController* GetMainPlayerController();

	
	virtual void Init() override;


	UFUNCTION(BlueprintCallable)
		void SaveGame();

	UFUNCTION(BlueprintCallable)
		void LoadGame();




};
