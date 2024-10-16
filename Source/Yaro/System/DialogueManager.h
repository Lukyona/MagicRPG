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

	
	int DialogueNum = 0; // 0 - intro

	bool bDialogueUIVisible;

	bool bSpeechBuubbleVisible;



	FVector SBLocation;

	//UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		//bool bCanDisplaySpeechBubble = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSubclassOf<class AActor> SpeechBubble_BP;

	class AActor* SpeechBubble;

	TArray<UDataTable*> DialogueDatas;

public:
	UDialogueManager();

	void Init();

	void Tick();


	UFUNCTION(BlueprintCallable)
		int GetDialogueNum() { return DialogueNum; }

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
