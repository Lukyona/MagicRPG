// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Character/Enemies/Enemy.h"
#include "GameManager.generated.h"

/**
 * 
 */
class AMain;
class AMainPlayerController;
class UDialogueManager;
class UNPCManager;
class UUIManager;

 // 블루프린트에서 쓰려면 다이나믹 멀티캐스트여야함
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDele_Dynamic);

UCLASS()
class YARO_API UGameManager : public UGameInstance
{
	GENERATED_BODY()

	virtual void Init() override;
	virtual void Shutdown() override;
	virtual void StartGameInstance() override;

	UPROPERTY()
	AMain* Player;
	UPROPERTY()
	AMainPlayerController* MainPlayerController;
	UPROPERTY()
	UDialogueManager* DialogueManager;
	UPROPERTY()
	UNPCManager* NPCManager;
	UPROPERTY()
	UUIManager* UIManager;

	bool bIsSkipping = false;
	bool bIsSkippable = false; // 스킵 가능한지

	UPROPERTY()
	TMap<EEnemyType, int32> DeadEnemies;

	FTimerHandle SaveTimer;

	bool bIsSaveAllowed = true;

public:
	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<APawn> PlayerClass;

	template<typename T>
	static T* CreateManager(UGameManager* GameManager)
	{
		T* Manager = T::CreateInstance(GameManager);
		if (Manager && IsValid(Manager))
		{
			Manager->SetGameManager(GameManager);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to create %s or it is invalid."), *T::StaticClass()->GetName());
		}
		return Manager;
	}

	// Getters and Setters
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

	void SetIsSaveAllowed(bool Value) { bIsSaveAllowed = Value; }


	// Core Methods
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

	void SkipCombat();

	void StartFirstDungeon();

	// 배열을 const 참조로 반환하여 배열의 복사가 일어나지 않도록, 마지막 const는 이 함수가 객체의 멤버 변수를 변경하지 않는 것을 의미
	UFUNCTION(BlueprintCallable, BlueprintPure)
	const TMap<EEnemyType, int32>& GetDeadEnemies() { return DeadEnemies; }
	void UpdateDeadEnemy(EEnemyType EnemyType);

	void EscapeToSafeLocation(); // press E key, spawn player at the other location


	UPROPERTY(BlueprintAssignable, BlueprintCallable) // 레벨 블루프린트에서 바인딩함
		FDele_Dynamic PlaneUp;

	UPROPERTY(BlueprintAssignable, BlueprintCallable) // 레벨 블루프린트에서 바인딩함
		FDele_Dynamic Ending;

	UPROPERTY(BlueprintAssignable, BlueprintCallable) // 레벨 블루프린트에서 바인딩함
		FDele_Dynamic SkipFirstDungeon;

	UPROPERTY(BlueprintAssignable, BlueprintCallable) // 레벨 블루프린트에서 바인딩함
		FDele_Dynamic SkipSecondDungeon;

	UPROPERTY(BlueprintAssignable, BlueprintCallable) // 레벨 블루프린트에서 바인딩함
		FDele_Dynamic SkipFinalDungeon;
};
