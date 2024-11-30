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
class UYaroSaveGame;

 // �������Ʈ���� ������ ���̳��� ��Ƽĳ��Ʈ������
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDele_Dynamic);

enum class EDialogueState : int32
{
	BeforeFirstDungeon = 2,
	FirstDungeonStarted = 3,
	MoveToBoat = 4,
	SecondDungeonStarted = 6,
	PlayerJumpToPlatform = 8,
	NPCMoveToBridge = 10,
	NPCCrossedBridge = 11,
	AfterCombatWithSpiders = 13,
	SecondDungeonFinished = 15,
	ReadyToFightWithBoss = 17,
	AfterCombatWithBoss = 18,
	BackToCave = 19,
	AfterTookTheStone = 21,
	FinalLine = 23,
};

UCLASS()
class YARO_API UGameManager : public UGameInstance
{
	GENERATED_BODY()

	virtual void Init() override;
	virtual void Shutdown() override;
	virtual void StartGameInstance() override;

	UPROPERTY()
	AMain* Player = nullptr;
	UPROPERTY()
	AMainPlayerController* MainPlayerController = nullptr;
	//UPROPERTY()
	UDialogueManager* DialogueManager = nullptr;
	//UPROPERTY()
	UNPCManager* NPCManager = nullptr;
	//UPROPERTY()
	UUIManager* UIManager = nullptr;

	bool bIsSkipping = false;
	bool bIsSkippable = false; // ��ŵ ��������

	UPROPERTY()
	TMap<EEnemyType, int32> DeadEnemies;

	FTimerHandle SaveTimer;

	bool bIsSaveAllowed = true;

	FDelegateHandle TickerHandle;

	TArray<FVector> SafeLocationList =
	{
		FVector(4620.f, -3975.f, -2117.f),
		FVector(5165.f, -2307.f, -2117.f),
		FVector(2726.f, -3353.f, -500.f)
	};


	template<typename T>
	static T* CreateManager(UGameManager* GameManager)
	{
		T* Manager = T::CreateInstance(GameManager);
		if (Manager && IsValid(Manager))
		{
			Manager->SetGameManager(GameManager);
			Manager->AddToRoot();
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to create %s or it is invalid."), *T::StaticClass()->GetName());
		}

		return Manager;
	}

	template <typename T>
	void CleanupManager(T*& Manager)
	{
		if (Manager && IsValid(Manager))
		{
			Manager->RemoveFromRoot();
			Manager = nullptr;
			T::Instance = nullptr;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s is already null or invalid."), *T::StaticClass()->GetName());
		}
	}

public:
	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<APawn> PlayerClass;

	// Getters and Setters
	UFUNCTION(BlueprintCallable, BlueprintPure)
	UDialogueManager* GetDialogueManager() const 
	{
		return DialogueManager; 
	}

	UFUNCTION(BlueprintCallable, BlueprintPure)
	UNPCManager* GetNPCManager() 
	{
		return NPCManager; 
	}

	UFUNCTION(BlueprintCallable, BlueprintPure)
	UUIManager* GetUIManager() 
	{
		return UIManager; 
	}

	UFUNCTION(BlueprintCallable, BlueprintPure)
	AMain* GetPlayer();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	AMainPlayerController* GetMainPlayerController();

	void SetIsSaveAllowed(bool Value) 
	{
		bIsSaveAllowed = Value; 
	}


	// Core Methods
	bool Tick(float DeltaTime);

	UFUNCTION(BlueprintCallable)
	void SaveGame();
	UFUNCTION(BlueprintCallable)
	void LoadGame();

	void SavePlayerInfo(UYaroSaveGame* SaveGameInstance);
	void LoadPlayerInfo(UYaroSaveGame* LoadGameInstance);
	void SaveNPCLocations(UYaroSaveGame* SaveGameInstance);
	void LoadNPCLocations(UYaroSaveGame* LoadGameInstance);


	UFUNCTION(BlueprintCallable)
	bool IsSkipping() const 
	{
		return bIsSkipping; 
	}
	UFUNCTION(BlueprintCallable)
	void SetIsSkipping(bool bSkipping) 
	{
		bIsSkipping = bSkipping; 
	}

	UFUNCTION(BlueprintCallable)
	bool IsSkippable() const 
	{
		return bIsSkippable; 
	}
	UFUNCTION(BlueprintCallable)
	void SetIsSkippable(bool bSkippable) 
	{
		bIsSkippable = bSkippable; 
	}

	void SkipCombat();

	void StartFirstDungeon();

	// �迭�� const ������ ��ȯ�Ͽ� �迭�� ���簡 �Ͼ�� �ʵ���, ������ const�� �� �Լ��� ��ü�� ��� ������ �������� �ʴ� ���� �ǹ�
	UFUNCTION(BlueprintCallable, BlueprintPure)
	const TMap<EEnemyType, int32>& GetDeadEnemies() 
	{
		return DeadEnemies; 
	}
	void UpdateDeadEnemy(EEnemyType EnemyType);

	void EscapeToSafeLocation(); // press E key, spawn player at the other location


	UPROPERTY(BlueprintAssignable, BlueprintCallable) // ���� �������Ʈ���� ���ε���
		FDele_Dynamic PlaneUp;

	UPROPERTY(BlueprintAssignable, BlueprintCallable) // ���� �������Ʈ���� ���ε���
		FDele_Dynamic Ending;

	UPROPERTY(BlueprintAssignable, BlueprintCallable) // ���� �������Ʈ���� ���ε���
		FDele_Dynamic SkipFirstDungeon;

	UPROPERTY(BlueprintAssignable, BlueprintCallable) // ���� �������Ʈ���� ���ε���
		FDele_Dynamic SkipSecondDungeon;

	UPROPERTY(BlueprintAssignable, BlueprintCallable) // ���� �������Ʈ���� ���ε���
		FDele_Dynamic SkipFinalDungeon;
};
