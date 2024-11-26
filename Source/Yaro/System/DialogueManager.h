// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DialogueManager.generated.h"

/**
 * 
 */
class UGameManager;
class UNPCManager;
class UUIManager;
class AMain;
class AMainPlayerController;
class UDialogueUI;
class AActor;
class UDataTable;
class AYaroCharacter;

UCLASS(BlueprintType)
class YARO_API UDialogueManager : public UObject
{
	GENERATED_BODY()

	static UDialogueManager* Instance;

	UPROPERTY()
	UGameManager* GameManager;

	UPROPERTY()
	UNPCManager* NPCManager;

	UPROPERTY()
	UUIManager* UIManager;

	UPROPERTY()
	AMain* Player;

	UPROPERTY()
		AMainPlayerController* MainPlayerController;

	UPROPERTY()
		UDialogueUI* DialogueUI;

	int DialogueNum = 0; // 0 - intro

	bool bDialogueUIVisible;

	bool bSpeechBuubbleVisible;

	ACharacter* SpeakingTarget;

	UPROPERTY()
	AActor* SpeechBubble;

	UPROPERTY()
	TArray <UDataTable*> DialogueDatas;

public:
	static UDialogueManager* CreateInstance(UGameInstance* Outer)
	{
		if (Instance == nullptr)
		{
			Instance = NewObject<UDialogueManager>(Outer, UDialogueManager::StaticClass());
			Instance->AddToRoot();
		}
		return Instance;
	}

	void BeginPlay();
	void Tick();


	UFUNCTION(BlueprintCallable)
		void CheckDialogueStartCondition();

	void TriggerNextDialogue();

	void SetGameManager(UGameManager* Manager);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		UDialogueUI* GetDialogueUI() { return DialogueUI; }

	UFUNCTION(BlueprintCallable)
		int GetDialogueNum() const { return DialogueNum; }

	UFUNCTION(BlueprintCallable)
	void SetDialogueNum(int Value) { DialogueNum = Value; }

	UFUNCTION(BlueprintCallable)
		bool IsDialogueUIVisible() const { return bDialogueUIVisible; }

	void SetDialogueUIVisible(bool bVisible) { bDialogueUIVisible = bVisible; }

	UFUNCTION(BlueprintCallable)
		void DisplayDialogueUI();

	UFUNCTION(BlueprintCallable)
		void RemoveDialogueUI();

		void DialogueEndEvents();

	UFUNCTION(BlueprintCallable)
		void DisplaySpeechBuubble(AYaroCharacter* NPC);

	void RemoveSpeechBuubble();

};
