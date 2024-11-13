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

	bool bIsSkippable = false; // 스킵 가능한지


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

	// 배열을 const 참조로 반환하여 배열의 복사가 일어나지 않도록, 마지막 const는 이 함수가 객체의 멤버 변수를 변경하지 않는 것을 의미
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
