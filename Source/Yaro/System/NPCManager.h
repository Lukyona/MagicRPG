// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Character/YaroCharacter.h"
#include "NPCManager.generated.h"

/**
 * 
 */

class UGameManager;
class UDialogueManager;
class UUIManager;
class AYaroCharacter;

UCLASS()
class YARO_API UNPCManager : public UObject
{
	GENERATED_BODY()

	friend class UGameManager;

	static UNPCManager* Instance;

	UPROPERTY()
	UGameManager* GameManager;
	UPROPERTY()
	UDialogueManager* DialogueManager;
	UPROPERTY()
	UUIManager* UIManager;

	//NPC Management
	UPROPERTY()
	TMap<FString, AYaroCharacter*> NPCMap;
	UPROPERTY()
	AYaroCharacter* Momo;
	UPROPERTY()
	AYaroCharacter* Luko;
	UPROPERTY()
	AYaroCharacter* Vovo;
	UPROPERTY()
	AYaroCharacter* Vivi;
	UPROPERTY()
	AYaroCharacter* Zizi;

public:
	static UNPCManager* CreateInstance(UGameInstance* Outer)
	{
		if (Instance == nullptr)
		{
			Instance = NewObject<UNPCManager>(Outer, UNPCManager::StaticClass());
		}
		return Instance;
	}

	//Initialization
	void Init();
	UFUNCTION(BlueprintCallable)
	void InitializeNPCs(UWorld* World);

	//Utilities
	FString EnumToString(const FString& EnumName, ENPCType EnumValue);

	//NPC Management
	void AddNPC(FString NPCName, AYaroCharacter* NPC);

	UFUNCTION(BlueprintCallable)
	void UpdateNPCPositions(int32 DialogueNum);

	UFUNCTION(BlueprintCallable)
	void SetPositionsForDialogue();

	UFUNCTION(BlueprintCallable)
	void AllNpcMoveToPlayer();
	UFUNCTION(BlueprintCallable)
	void AllNpcLookAtPlayer();
	void AllNpcDisableLookAt();
	UFUNCTION(BlueprintCallable)
	void AllNpcStopFollowPlayer();

	UFUNCTION(BlueprintCallable)
	void OpenMouth(AYaroCharacter* npc);
	void CloseAllMouth();

	void MoveNPCToLocation(ENPCType NPC, FVector Location);
	void SetNPCLocation(ENPCType NPC, FVector Location);

	//Getters and Setters
	void SetGameManager(UGameManager* Manager)
	{
		GameManager = Manager;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsNPCInTalkRange();

	UFUNCTION(BlueprintCallable)
	const TMap<FString, AYaroCharacter*>& GetNPCMap() const 
	{
		return NPCMap; 
	}

	UFUNCTION(BlueprintCallable, BlueprintPure)
	AYaroCharacter* GetNPC(ENPCType NPCType);

	void SetAllNpcMovementSpeed(bool bEnableRunning);

};
