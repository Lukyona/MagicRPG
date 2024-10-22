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

	bool bIsSkippable = false; // 스킵 가능한지


	UPROPERTY()
		TArray<FString> DeadEnemies;

public:

	UPROPERTY(BlueprintReadWrite)
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

	// 배열을 const 참조로 반환하여 배열의 복사가 일어나지 않도록, 마지막 const는 이 함수가 객체의 멤버 변수를 변경하지 않는 것을 의미
	const TArray<FString>& GetDeadEnemies() const { return DeadEnemies; }
	void AddDeadEnemy(FString EnemyName) { DeadEnemies.Add(EnemyName); }


	virtual void Init() override;


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
