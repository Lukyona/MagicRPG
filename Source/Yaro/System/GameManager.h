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

public:

	UPROPERTY(BlueprintReadWrite, Category = "PlayerData")
		TSubclassOf<APawn> PlayerClass;

	UPROPERTY()
		class UDialogueManager* DialogueManager;  // ���̾�α� �Ŵ����� ���� ����

	UDialogueManager* GetDialogueManager() const { return DialogueManager; }
	
	virtual void Init() override;


	UFUNCTION(BlueprintCallable)
		void SaveGame();

	UFUNCTION(BlueprintCallable)
		void LoadGame();




};
