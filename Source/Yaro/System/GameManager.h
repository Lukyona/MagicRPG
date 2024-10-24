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

	UPROPERTY(BlueprintReadWrite)
		TSubclassOf<APawn> PlayerClass;

	FTimerHandle SaveTimer; 

	UFUNCTION(BlueprintCallable, BlueprintPure)
	UDialogueManager* GetDialogueManager() const { return DialogueManager; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
		UNPCManager* GetNPCManager() {
		if (NPCManager) return NPCManager; else { UE_LOG(LogTemp, Warning, TEXT("GetNPCManager nulll")); return nullptr; }
	}


	UFUNCTION(BlueprintCallable, BlueprintPure)
		UUIManager* GetUIManager() const; //{ return UIManager; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	AMain* GetPlayer();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	AMainPlayerController* GetMainPlayerController();

	// �迭�� const ������ ��ȯ�Ͽ� �迭�� ���簡 �Ͼ�� �ʵ���, ������ const�� �� �Լ��� ��ü�� ��� ������ �������� �ʴ� ���� �ǹ�
	UFUNCTION(BlueprintCallable, BlueprintPure)
	const TArray<FString>& GetDeadEnemies() { return DeadEnemies; }
	void AddDeadEnemy(FString EnemyName) { DeadEnemies.Add(EnemyName); }


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


};
