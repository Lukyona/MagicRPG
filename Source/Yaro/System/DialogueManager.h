// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DialogueManager.generated.h"

/**
 * 
 */
UCLASS()
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

	int DialogueNum = 0; // 0 - intro

	bool bDialogueUIVisible;

	bool bSpeechBuubbleVisible;



	FVector SBLocation;

	//UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		//bool bCanDisplaySpeechBubble = false;

	TSubclassOf<class AActor> SpeechBubble_BP;

	UPROPERTY()
	class AActor* SpeechBubble;

	UPROPERTY()
	TArray <class UDataTable*> DialogueDatas;

public:
	static UDialogueManager* CreateInstance(UObject* Outer)
	{
		if (Instance == nullptr)
		{
			Instance = NewObject<UDialogueManager>(Outer, UDialogueManager::StaticClass());
		}
		return Instance;
	}

	void Init();

	void Tick();

	UFUNCTION(BlueprintCallable)
		void CheckDialogueStartCondition();

	UFUNCTION(BlueprintCallable)
		int GetDialogueNum() const { return DialogueNum; }

	void SetDialogueNum(int Value) { DialogueNum = Value; }

	UFUNCTION(BlueprintCallable)
		bool IsDialogueUIVisible() const { return bDialogueUIVisible; }

	void SetDialogueUIVisible(bool bVisible) { bDialogueUIVisible = bVisible; }

	UFUNCTION(BlueprintCallable)
		void DisplayDialogueUI();

	UFUNCTION(BlueprintCallable)
		void RemoveDialogueUI();


	UPROPERTY(BlueprintReadOnly)
		class UDialogueUI* DialogueUI;

	UPROPERTY(VisibleAnywhere)
		TSubclassOf<class UUserWidget> DialogueUIClass;


	UFUNCTION(BlueprintCallable)
		void DialogueEvents();

	UFUNCTION(BlueprintCallable)
		void DisplaySpeechBuubble(class AYaroCharacter* npc);

	void RemoveSpeechBuubble();

};
