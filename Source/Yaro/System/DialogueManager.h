// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DialogueManager.generated.h"

/**
 * 
 */
class UGameManager;
class UNPCManager;
class UUIManager;
class AMain;
class AMainPlayerController;
class UDialogueUI;
class AActor;
class UDataTable;
class AYaroCharacter;

UENUM(BlueprintType)
enum class EDialogueState : uint8
{
	Intro = 0,
	BeforeFirstDungeon = 2,
	FirstDungeonStarted = 3,
	FirstDungeonFinished = 3,
	MoveToBoat = 4,
	SecondDungeonStarted = 6,
	PlayerJumpToPlatform = 8,
	InteractWithYellowStone = 9,
	NPCMoveToBridge = 10,
	NPCCrossedBridge = 11,
	FoodTableEvent = 11,
	CombatWithSpiders = 12,
	AfterCombatWithSpiders = 13,
	CombatWithLittleMonsters = 14,
	SecondDungeonFinished = 15,
	RockEventInBossStage = 16,
	ReadyToFightWithBoss = 17,
	CombatWithBoss = 18,
	AfterCombatWithBoss = 18,
	BackToCave = 19,
	GetMissionItem = 20,
	AfterTookTheStone = 21,
	FinalDialogue = 22,
	FinalLine = 23,
};

UCLASS(BlueprintType)
class YARO_API UDialogueManager : public UObject
{
	GENERATED_BODY()

	friend class UGameManager;

	static UDialogueManager* Instance;

	UPROPERTY()
	UGameManager* GameManager;
	UPROPERTY()
	UNPCManager* NPCManager;
	UPROPERTY()
	UUIManager* UIManager;

	UPROPERTY()
	AMain* Player;
	UPROPERTY()
	AMainPlayerController* MainPlayerController;

	UPROPERTY()
	UDialogueUI* DialogueUI;
	UPROPERTY()
	TArray <UDataTable*> DialogueDatas;

	UPROPERTY()
	AActor* SpeechBubble;
	UPROPERTY()
	ACharacter* SpeakingTarget;

	int32 DialogueNum = 0; // 0 - intro

	bool bDialogueUIVisible = false;
	bool bSpeechBuubbleVisible = false;

	template<class T>
	UClass* LoadAsset(const FString& AssetPath)
	{
		TSoftClassPtr<T> AssetClass(FSoftObjectPath(AssetPath));
		if (!AssetClass.IsValid())
		{
			AssetClass.LoadSynchronous();
		}
		return AssetClass.IsValid() ? AssetClass.Get() : nullptr;
	}

public:
	static UDialogueManager* CreateInstance(UGameInstance* Outer)
	{
		if (Instance == nullptr)
		{
			Instance = NewObject<UDialogueManager>(Outer, UDialogueManager::StaticClass());
		}
		return Instance;
	}

	//Getters and Setters
	void SetGameManager(UGameManager* Manager)
	{
		GameManager = Manager;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure)
	UDialogueUI* GetDialogueUI() 
	{
		return DialogueUI; 
	}

	UFUNCTION(BlueprintCallable)
	int32 GetDialogueNum() const 
	{
		return DialogueNum; 
	}
	UFUNCTION(BlueprintCallable)
	void SetDialogueNum(int Value) 
	{
		DialogueNum = Value; 
	}

	UFUNCTION(BlueprintCallable)
	const EDialogueState GetDialogueState() const
	{
		return static_cast<EDialogueState>(DialogueNum);
	}

	UFUNCTION(BlueprintCallable)
	bool IsDialogueUIVisible() const 
	{
		return bDialogueUIVisible; 
	}
	void SetDialogueUIVisible(bool bVisible) 
	{
		bDialogueUIVisible = bVisible; 
	}

	//Core Methods
	void BeginPlay();
	void Tick();

	UFUNCTION(BlueprintCallable)
	void CheckDialogueStartCondition();

	void TriggerNextDialogue();

	UFUNCTION(BlueprintCallable)
	void DisplayDialogueUI();
	UFUNCTION(BlueprintCallable)
	void RemoveDialogueUI();

	void DialogueEndEvents();

	UFUNCTION(BlueprintCallable)
	void DisplaySpeechBuubble(AYaroCharacter* NPC);
	void RemoveSpeechBuubble();

};
