// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Character/Enemies/Enemy.h"
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
		TMap<EEnemyType, int32> DeadEnemies;

	FTimerHandle SaveTimer;

	bool bIsSaveAllowed = true;

public:

	UPROPERTY(BlueprintReadWrite)
		TSubclassOf<APawn> PlayerClass;


	UFUNCTION(BlueprintCallable, BlueprintPure)
	UDialogueManager* GetDialogueManager() const { return DialogueManager; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
		UNPCManager* GetNPCManager() { return NPCManager; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
		UUIManager* GetUIManager() { return UIManager; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	AMain* GetPlayer();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	AMainPlayerController* GetMainPlayerController();

	// �迭�� const ������ ��ȯ�Ͽ� �迭�� ���簡 �Ͼ�� �ʵ���, ������ const�� �� �Լ��� ��ü�� ��� ������ �������� �ʴ� ���� �ǹ�
	UFUNCTION(BlueprintCallable, BlueprintPure)
	const TMap<EEnemyType, int32>& GetDeadEnemies() { return DeadEnemies; }
	void UpdateDeadEnemy(EEnemyType EnemyType) { DeadEnemies[EnemyType]++; }


	virtual void Init() override;
	virtual void Shutdown() override;
	virtual void StartGameInstance() override;


	UFUNCTION(BlueprintCallable)
		void SaveGame();

	UFUNCTION(BlueprintCallable)
		void LoadGame();


	UFUNCTION(BlueprintCallable)
	bool IsSkipping() const { return bIsSkipping; }

	UFUNCTION(BlueprintCallable)
	void SetIsSkipping(bool bSkipping) { bIsSkipping = bSkipping; }

	UFUNCTION(BlueprintCallable)
	bool IsSkippable() const { return bIsSkippable; }

	UFUNCTION(BlueprintCallable)
	void SetIsSkippable(bool bSkippable) { bIsSkippable = bSkippable; }

	void SetIsSaveAllowed(bool Value) { bIsSaveAllowed = Value; }

};
