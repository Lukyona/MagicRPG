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
	class UUIManager* UIManager;

	bool bIsSkipping = false;

	bool bIsSkippable = false; // ��ŵ ��������


	UPROPERTY()
		TArray<FString> DeadEnemies;

public:

	UPROPERTY(BlueprintReadWrite, Category = "PlayerData")
		TSubclassOf<APawn> PlayerClass;

	FTimerHandle SaveTimer; 

	UFUNCTION(BlueprintCallable)
	UDialogueManager* GetDialogueManager() const { return DialogueManager; }

	UFUNCTION(BlueprintCallable)
	UNPCManager* GetNPCManager() const { return NPCManager; }


	UFUNCTION(BlueprintCallable)
		UUIManager* GetUIManager() const { return UIManager; }

	UFUNCTION(BlueprintCallable)
	AMain* GetPlayer();

	UFUNCTION(BlueprintCallable)
	AMainPlayerController* GetMainPlayerController();

	// �迭�� const ������ ��ȯ�Ͽ� �迭�� ���簡 �Ͼ�� �ʵ���, ������ const�� �� �Լ��� ��ü�� ��� ������ �������� �ʴ� ���� �ǹ�
	const TArray<FString>& GetDeadEnemies() const { return DeadEnemies; }
	void AddDeadEnemy(FString EnemyName) { DeadEnemies.Add(EnemyName); }


	virtual void Init() override;


	UFUNCTION(BlueprintCallable)
		void SaveGame();

	UFUNCTION(BlueprintCallable)
		void LoadGame();


	bool IsSkipping() const { return bIsSkipping; }
	void SetIsSkipping(bool bSkipping) { bIsSkipping = bSkipping; }

	bool IsSkippable() const { return bIsSkippable; }
	void SetIsSkippable(bool bSkippable) { bIsSkippable = bSkippable; }


};
