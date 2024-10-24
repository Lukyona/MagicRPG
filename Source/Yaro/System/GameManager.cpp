// Fill out your copyright notice in the Description page of Project Settings.


#include "Yaro/System/GameManager.h"
#include "Kismet/GameplayStatics.h"
#include "Yaro/System/DialogueManager.h"
#include "Yaro/System/NPCManager.h"
#include "Yaro/System/UIManager.h"
#include "Yaro/System/PlayerStatsManager.h"
#include "Yaro/System/YaroSaveGame.h"
#include "Yaro/System/MainPlayerController.h"
#include "Yaro/Character/Main.h"
#include "Yaro/Character/YaroCharacter.h"
#include "Item.h"


void UGameManager::Init()
{
	Super::Init();

    // 매니저들 생성
    DialogueManager = UDialogueManager::CreateInstance(this);
	if (DialogueManager) DialogueManager->SetGameManager(this);

	UIManager = UUIManager::CreateInstance(this);
	if (UIManager) UIManager->SetGameManager(this);

	NPCManager = UNPCManager::CreateInstance(this);
	if (NPCManager)
	{
		NPCManager->SetGameManager(this);
		NPCManager->Init();
	}
}

void UGameManager::Shutdown()
{
	Super::Shutdown();
}

void UGameManager::StartGameInstance()
{
	Super::StartGameInstance();
}

UUIManager* UGameManager::GetUIManager() const
{
	if (UIManager) return UIManager;
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("nonneoe"));

	}
	return nullptr;

}

AMain* UGameManager::GetPlayer()
{
	if (!Player)
	{
		APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (PlayerController)
		{
			Player = Cast<AMain>(PlayerController->GetPawn());
		}
	}
	return Player;
}

AMainPlayerController* UGameManager::GetMainPlayerController()
{
	if (!MainPlayerController)
	{
		APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (PlayerController)
		{
			MainPlayerController = Cast<AMainPlayerController>(PlayerController);
		}
	}
	return MainPlayerController;
}

void UGameManager::SaveGame()
{
	if (DialogueManager->GetDialogueNum() >= 23) return;

	if (DialogueManager->IsDialogueUIVisible() || DialogueManager->GetDialogueNum() == 21
		|| Player->IsInAir() || Player->IsFallenInDungeon())
	{
		GetWorld()->GetTimerManager().SetTimer(SaveTimer, this, &UGameManager::SaveGame, 1.f, false);
		return;
	}


	UYaroSaveGame* SaveGameInstance = Cast<UYaroSaveGame>(UGameplayStatics::CreateSaveGameObject(UYaroSaveGame::StaticClass()));

	SaveGameInstance->PlayerGender = Player->GetStat("Gender");
	SaveGameInstance->CharacterStats.HP = Player->GetStat("HP");
	SaveGameInstance->CharacterStats.MaxHP = Player->GetStat("MaxHP");
	SaveGameInstance->CharacterStats.MP = Player->GetStat("MP");
	SaveGameInstance->CharacterStats.MaxMP = Player->GetStat("MaxMP");
	SaveGameInstance->CharacterStats.SP = Player->GetStat("SP");
	SaveGameInstance->CharacterStats.MaxSP = Player->GetStat("MaxSP");
	SaveGameInstance->CharacterStats.Level = Player->GetStat("Level");
	SaveGameInstance->CharacterStats.Exp = Player->GetStat("Exp");
	SaveGameInstance->CharacterStats.MaxExp = Player->GetStat("MaxExp");
	SaveGameInstance->CharacterStats.PotionNum = Player->GetStat("PotionNum");

	SaveGameInstance->DialogueNum = DialogueManager->GetDialogueNum();
	SaveGameInstance->CharacterStats.FallCount = Player->GetFallCount();

	SaveGameInstance->CharacterStats.Location = Player->GetActorLocation();
	SaveGameInstance->CharacterStats.Rotation = Player->GetActorRotation();

	if (DialogueManager->GetDialogueNum() < 23)
	{
		SaveGameInstance->NpcInfo.MomoLocation = NPCManager->GetNPC("Momo")->GetActorLocation();
		SaveGameInstance->NpcInfo.LukoLocation = NPCManager->GetNPC("Luko")->GetActorLocation();
		SaveGameInstance->NpcInfo.VovoLocation = NPCManager->GetNPC("Vovo")->GetActorLocation();
		SaveGameInstance->NpcInfo.ViviLocation = NPCManager->GetNPC("Vivi")->GetActorLocation();
		SaveGameInstance->NpcInfo.ZiziLocation = NPCManager->GetNPC("Zizi")->GetActorLocation();
	}

	if (DialogueManager->GetDialogueNum() < 4)
		SaveGameInstance->NpcInfo.TeamMoveIndex = NPCManager->GetNPC("Vivi")->GetIndex();

	SaveGameInstance->DeadEnemyList = DeadEnemies;

	if (Player->GetItemInHand() != nullptr)
	{
		SaveGameInstance->CharacterStats.ItemName = Player->GetItemInHand()->GetName();
	}

	UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveGameInstance->SaveName, SaveGameInstance->UserIndex);

	if (DialogueManager->GetDialogueNum() == 18 && UIManager->GetSystemMessageNum() == 12) return;

	GetWorld()->GetTimerManager().SetTimer(SaveTimer, this, &UGameManager::SaveGame, 1.f, false);
}

void UGameManager::LoadGame()
{
	UYaroSaveGame* LoadGameInstance = Cast<UYaroSaveGame>(UGameplayStatics::CreateSaveGameObject(UYaroSaveGame::StaticClass()));

	LoadGameInstance = Cast<UYaroSaveGame>(UGameplayStatics::LoadGameFromSlot(LoadGameInstance->SaveName, LoadGameInstance->UserIndex));

	DialogueManager->SetDialogueNum(LoadGameInstance->DialogueNum);
	Player->SetFallCount(LoadGameInstance->CharacterStats.FallCount);

	Player->SetStat("HP", LoadGameInstance->CharacterStats.HP);
	Player->SetStat("MaxHP", LoadGameInstance->CharacterStats.MaxHP);
	Player->SetStat("MP", LoadGameInstance->CharacterStats.MP);
	Player->SetStat("MaxMP", LoadGameInstance->CharacterStats.MaxMP);
	Player->SetStat("SP", LoadGameInstance->CharacterStats.SP);
	Player->SetStat("MaxSP", LoadGameInstance->CharacterStats.MaxSP);
	Player->SetStat("Level", LoadGameInstance->CharacterStats.Level);
	Player->SetStat("Exp", LoadGameInstance->CharacterStats.Exp);
	Player->SetStat("MaxExp", LoadGameInstance->CharacterStats.MaxExp);
	Player->SetStat("PotionNum", LoadGameInstance->CharacterStats.PotionNum);


	DeadEnemies = LoadGameInstance->DeadEnemyList;

	if (DialogueManager->GetDialogueNum() < 4)
	{
		NPCManager->GetNPC("Vovo")->SetIndex(LoadGameInstance->NpcInfo.TeamMoveIndex);
		NPCManager->GetNPC("Vivi")->SetIndex(LoadGameInstance->NpcInfo.TeamMoveIndex);
		NPCManager->GetNPC("Zizi")->SetIndex(LoadGameInstance->NpcInfo.TeamMoveIndex);
	}

	Player->InitializeStats();

	FString ItemName = LoadGameInstance->CharacterStats.ItemName;
	if (Player->GetItemMap() != nullptr)
	{
		if (ItemName.Contains("Yellow") && Player->GetItemMap()->Contains("YellowStone")) // 저장할 때 손에 돌을 집은 상태였으면
		{
			AItem* Item = GetWorld()->SpawnActor<AItem>(Player->GetItemMap()->FindRef("YellowStone"));
			Item->PickUp(Player);
		}
	}


	if (DialogueManager->GetDialogueNum() != 5)
	{
		if (DialogueManager->GetDialogueNum() == 15 && UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("boss")) return;

		Player->SetActorLocation(LoadGameInstance->CharacterStats.Location);
		Player->SetActorRotation(LoadGameInstance->CharacterStats.Rotation);

		if (DialogueManager->GetDialogueNum() == 19) return;

		NPCManager->SetNPCLocation("Momo", LoadGameInstance->NpcInfo.MomoLocation);
		NPCManager->SetNPCLocation("Luko", LoadGameInstance->NpcInfo.LukoLocation);
		NPCManager->SetNPCLocation("Vovo", LoadGameInstance->NpcInfo.VovoLocation);
		NPCManager->SetNPCLocation("Vivi", LoadGameInstance->NpcInfo.ViviLocation);
		NPCManager->SetNPCLocation("Zizi", LoadGameInstance->NpcInfo.ZiziLocation);
	}
}