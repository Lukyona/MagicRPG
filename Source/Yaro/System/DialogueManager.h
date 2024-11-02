// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DialogueManager.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class YARO_API UDialogueManager : public UObject
{
	GENERATED_BODY()

	static UDialogueManager* Instance;

	UPROPERTY()
	class UGameManager* GameManager;

	UPROPERTY()
	class UNPCManager* NPCManager;

	UPROPERTY()
	class UUIManager* UIManager;

	UPROPERTY()
	class AMain* Player;

	UPROPERTY()
		class AMainPlayerController* MainPlayerController;

	UPROPERTY()
		class UDialogueUI* DialogueUI;

	int DialogueNum = 0; // 0 - intro

	bool bDialogueUIVisible;

	bool bSpeechBuubbleVisible;



	FVector SBLocation;

	//UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		//bool bCanDisplaySpeechBubble = false;

	UPROPERTY()
	class AActor* SpeechBubble;

	UPROPERTY()
	TArray <class UDataTable*> DialogueDatas;

public:
	static UDialogueManager* CreateInstance(UGameInstance* Outer)
	{
		if (Instance == nullptr)
		{
			Instance = NewObject<UDialogueManager>(Outer, UDialogueManager::StaticClass());
		}
		return Instance;
	}

	void BeginPlay();
	void Tick();

	void SetGameManager(UGameManager* Manager);

	UFUNCTION(BlueprintCallable)
		void CheckDialogueStartCondition();

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


	UFUNCTION(BlueprintCallable)
		void DialogueEvents();

	UFUNCTION(BlueprintCallable)
		void DisplaySpeechBuubble(class AYaroCharacter* npc);

	void RemoveSpeechBuubble();

};
