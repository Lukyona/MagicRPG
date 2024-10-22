// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Character/YaroCharacter.h"
#include "NPCManager.generated.h"

/**
 * 
 */
UCLASS()
class YARO_API UNPCManager : public UObject
{
	GENERATED_BODY()

	static UNPCManager* Instance;

	UPROPERTY()
	class UGameManager* GameManager;

	UPROPERTY()
	class UDialogueManager* DialogueManager;

	UPROPERTY()
		class UUIManager* UIManager;

	UPROPERTY()
		TMap<FString, class AYaroCharacter*> NPCMap;


	UPROPERTY()
		class AYaroCharacter* Momo;

	UPROPERTY()
		class AYaroCharacter* Luko;

	UPROPERTY()
		class AYaroCharacter* Vovo;

	UPROPERTY()
		class AYaroCharacter* Vivi;

	UPROPERTY()
		class AYaroCharacter* Zizi;

public:
	static UNPCManager* CreateInstance(UObject* Outer)
	{
		if (Instance == nullptr)
		{
			Instance = NewObject<UNPCManager>(Outer, UNPCManager::StaticClass());
		}
		return Instance;
	}

	void Init();

	FString EnumToString(const FString& EnumName, ENPCType EnumValue);

	void InitializeNPCs();

	void AddNPC(FString NPCName, AYaroCharacter* NPC);

	UFUNCTION(BlueprintCallable)
	void UpdateNPCPositions(int DialogueNum);

	UFUNCTION(BlueprintCallable)
		void SetPositionsForDialogue();

	UFUNCTION(BlueprintCallable)
		bool IsNPCInTalkRange();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	AYaroCharacter* GetNPC(ENPCType NPCType);

	AYaroCharacter* UNPCManager::GetNPC(FString NPCName) const;


	UFUNCTION(BlueprintCallable)
	const TMap<FString, AYaroCharacter*>& GetNPCMap() const { return NPCMap; }

	void MoveNPCToLocation(FString NPCName, FVector Location);
	void MoveNPCToLocation(AYaroCharacter* NPC, FVector Location);

	void SetNPCLocation(FString NPCName, FVector Location);
	void SetNPCLocation(AYaroCharacter* NPC, FVector Location);

	void AllNpcMoveToPlayer();

	void AllNpcLookAtPlayer();

	void AllNpcDisableLookAt();

	UFUNCTION(BlueprintCallable)
	void AllNpcStopFollowPlayer();


	UFUNCTION(BlueprintCallable)
		void OpenMouth(AYaroCharacter* npc);

	void CloseAllMouth();
};
