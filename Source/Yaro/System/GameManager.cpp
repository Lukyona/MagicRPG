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


void UGameManager::Init()
{
	Super::Init();

    // 다이얼로그 매니저 생성 및 초기화
    DialogueManager = UDialogueManager::CreateInstance(this);
	if(DialogueManager) DialogueManager->Init();

	NPCManager = UNPCManager::CreateInstance(this);
	if (NPCManager) NPCManager->Init();

	UIManager = UUIManager::CreateInstance(this);
	if (UIManager) UIManager->Init();
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

	SaveGameInstance->PlayerGender = Gender;
	SaveGameInstance->CharacterStats.HP = HP;
	SaveGameInstance->CharacterStats.MaxHP = MaxHP;
	SaveGameInstance->CharacterStats.MP = MP;
	SaveGameInstance->CharacterStats.MaxMP = MaxMP;
	SaveGameInstance->CharacterStats.SP = SP;
	SaveGameInstance->CharacterStats.MaxSP = MaxSP;
	SaveGameInstance->CharacterStats.Level = Level;
	SaveGameInstance->CharacterStats.Exp = Exp;
	SaveGameInstance->CharacterStats.MaxExp = MaxExp;
	SaveGameInstance->CharacterStats.PotionNum = PotionNum;

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

	if (ItemInHand)
	{
		SaveGameInstance->CharacterStats.ItemName = ItemInHand->GetName();
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

	HP = LoadGameInstance->CharacterStats.HP;
	MaxHP = LoadGameInstance->CharacterStats.MaxHP;
	MP = LoadGameInstance->CharacterStats.MP;
	MaxMP = LoadGameInstance->CharacterStats.MaxMP;
	SP = LoadGameInstance->CharacterStats.SP;
	MaxSP = LoadGameInstance->CharacterStats.MaxSP;
	Level = LoadGameInstance->CharacterStats.Level;
	Exp = LoadGameInstance->CharacterStats.Exp;
	MaxExp = LoadGameInstance->CharacterStats.MaxExp;
	PotionNum = LoadGameInstance->CharacterStats.PotionNum;


	DeadEnemies = LoadGameInstance->DeadEnemyList;

	if (DialogueManager->GetDialogueNum() < 4)
	{
		NPCManager->GetNPC("Vovo")->SetIndex(LoadGameInstance->NpcInfo.TeamMoveIndex);
		NPCManager->GetNPC("Vivi")->SetIndex(LoadGameInstance->NpcInfo.TeamMoveIndex);
		NPCManager->GetNPC("Zizi")->SetIndex(LoadGameInstance->NpcInfo.TeamMoveIndex);
	}

	if (SP < MaxSP && !recoverySP)
	{
		recoverySP = true;
		GetWorldTimerManager().SetTimer(SPTimer, this, &AMain::RecoverySP, SPDelay, true);
	}

	if (HP < MaxHP)
		GetWorldTimerManager().SetTimer(HPTimer, this, &AMain::RecoveryHP, HPDelay, true);

	if (MP < MaxMP)
		GetWorldTimerManager().SetTimer(MPTimer, this, &AMain::RecoveryMP, MPDelay, true);


	if (ObjectStorage)
	{
		if (Storage)
		{
			FString ItemName = LoadGameInstance->CharacterStats.ItemName;

			if (ItemName.Contains("Yellow") && Storage->ItemMap.Contains("YellowStone")) // 저장할 때 손에 돌을 집은 상태였으면
			{
				AItem* Item = GetWorld()->SpawnActor<AItem>(Storage->ItemMap["YellowStone"]);
				Item->PickUp(this);
			}
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