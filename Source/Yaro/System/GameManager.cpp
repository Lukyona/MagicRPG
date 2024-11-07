// Fill out your copyright notice in the Description page of Project Settings.


#include "Yaro/System/GameManager.h"
#include "Kismet/GameplayStatics.h"
#include "Yaro/System/DialogueManager.h"
#include "Yaro/System/NPCManager.h"
#include "Yaro/System/UIManager.h"
#include "Yaro/System/YaroSaveGame.h"
#include "Yaro/System/MainPlayerController.h"
#include "Yaro/Character/Main.h"
#include "Yaro/Character/YaroCharacter.h"
#include "Item.h"


void UGameManager::Init()
{
	Super::Init();

    // �Ŵ����� ����
	TWeakObjectPtr<UGameManager> WeakGameManager = this;

	// �񵿱� �۾����� ���� �����忡�� �߰� �ʱ�ȭ ����
	AsyncTask(ENamedThreads::GameThread, [WeakGameManager]()
		{
			if (WeakGameManager.IsValid())
			{
				UGameManager* GameManager = WeakGameManager.Get();

				// DialogueManager ���� �� ��ȿ�� üũ
				if (GameManager)
				{
					GameManager->DialogueManager = NewObject<UDialogueManager>(GameManager, UDialogueManager::StaticClass());
					if (GameManager->DialogueManager && IsValid(GameManager->DialogueManager))
					{
						GameManager->DialogueManager->AddToRoot();
						GameManager->DialogueManager->SetGameManager(GameManager);
					}
					else
					{
						UE_LOG(LogTemp, Error, TEXT("Failed to create DialogueManager or DialogueManager is invalid."));
					}
				}

				// UIManager ���� �� ��ȿ�� üũ
				if (GameManager)
				{
					GameManager->UIManager = NewObject<UUIManager>(GameManager, UUIManager::StaticClass());
					if (GameManager->UIManager && IsValid(GameManager->UIManager))
					{
						GameManager->UIManager->AddToRoot();
						GameManager->UIManager->SetGameManager(GameManager);
					}
					else
					{
						UE_LOG(LogTemp, Error, TEXT("Failed to create UIManager or UIManager is invalid."));
					}
				}

				// NPCManager ���� �� ��ȿ�� üũ
				if (GameManager)
				{
					GameManager->NPCManager = NewObject<UNPCManager>(GameManager, UNPCManager::StaticClass());
					if (GameManager->NPCManager && IsValid(GameManager->NPCManager))
					{
						GameManager->NPCManager->AddToRoot();
						GameManager->NPCManager->SetGameManager(GameManager);
						GameManager->NPCManager->Init();
					}
					else
					{
						UE_LOG(LogTemp, Error, TEXT("Failed to create NPCManager or NPCManager is invalid."));
					}
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("GameManager is not valid in AsyncTask."));
			}
		});
	/*
	AsyncTask(ENamedThreads::GameThread, [this]() // �׻� ���� �����忡�� �����ϵ���, ��Ƽ������ ũ���� ����
		{
			DialogueManager = UDialogueManager::CreateInstance(this);
			if (DialogueManager)
			{
				DialogueManager->SetGameManager(this);
				DialogueManager->AddToRoot(); // ���� ������ �Ŵ������� ��������� ������ �� ũ���� �߻�
			}

			UIManager = UUIManager::CreateInstance(this);
			if (UIManager)
			{
				UIManager->SetGameManager(this);
				UIManager->AddToRoot();
			}

			NPCManager = UNPCManager::CreateInstance(this);
			if (NPCManager)
			{
				NPCManager->SetGameManager(this);
				NPCManager->AddToRoot();
				NPCManager->Init();
			}
		});*/
}

void UGameManager::Shutdown()
{
	Super::Shutdown();
	if(IsValid(DialogueManager)) DialogueManager->RemoveFromRoot();
	if (IsValid(UIManager)) UIManager->RemoveFromRoot();
	if(IsValid(NPCManager)) NPCManager->RemoveFromRoot();

}

void UGameManager::StartGameInstance()
{
	Super::StartGameInstance();
}

AMain* UGameManager::GetPlayer()
{
	ACharacter* PlayerPawn = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (PlayerPawn)
	{
		Player = Cast<AMain>(PlayerPawn);
	}
	return Player;
}

AMainPlayerController* UGameManager::GetMainPlayerController()
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PlayerController)
	{
		MainPlayerController = Cast<AMainPlayerController>(PlayerController);
	}
	return MainPlayerController;
}

void UGameManager::SaveGame()
{
	if (DialogueManager->GetDialogueNum() >= 23 || !bIsSaveAllowed) return;

	if (DialogueManager->IsDialogueUIVisible() || DialogueManager->GetDialogueNum() == 21
		|| Player->IsInAir() || Player->IsFallenInDungeon())
	{
		GetWorld()->GetTimerManager().SetTimer(SaveTimer, this, &UGameManager::SaveGame, 2.f, false);
		return;
	}

	UYaroSaveGame* SaveGameInstance = Cast<UYaroSaveGame>(UGameplayStatics::CreateSaveGameObject(UYaroSaveGame::StaticClass()));

	SaveGameInstance->PlayerGender = Player->GetStat(EPlayerStat::Gender);
	SaveGameInstance->CharacterStats.HP = Player->GetStat(EPlayerStat::HP);
	SaveGameInstance->CharacterStats.MaxHP = Player->GetStat(EPlayerStat::MaxHP);
	SaveGameInstance->CharacterStats.MP = Player->GetStat(EPlayerStat::MP);
	SaveGameInstance->CharacterStats.MaxMP = Player->GetStat(EPlayerStat::MaxMP);
	SaveGameInstance->CharacterStats.SP = Player->GetStat(EPlayerStat::SP);
	SaveGameInstance->CharacterStats.MaxSP = Player->GetStat(EPlayerStat::MaxSP);
	SaveGameInstance->CharacterStats.Level = Player->GetStat(EPlayerStat::Level);
	SaveGameInstance->CharacterStats.Exp = Player->GetStat(EPlayerStat::Exp);
	SaveGameInstance->CharacterStats.MaxExp = Player->GetStat(EPlayerStat::MaxExp);
	SaveGameInstance->CharacterStats.PotionNum = Player->GetStat(EPlayerStat::PotionNum);

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

	Player->SetStat(EPlayerStat::HP, LoadGameInstance->CharacterStats.HP);
	Player->SetStat(EPlayerStat::MaxHP, LoadGameInstance->CharacterStats.MaxHP);
	Player->SetStat(EPlayerStat::MP, LoadGameInstance->CharacterStats.MP);
	Player->SetStat(EPlayerStat::MaxMP, LoadGameInstance->CharacterStats.MaxMP);
	Player->SetStat(EPlayerStat::SP, LoadGameInstance->CharacterStats.SP);
	Player->SetStat(EPlayerStat::MaxSP, LoadGameInstance->CharacterStats.MaxSP);
	Player->SetStat(EPlayerStat::Level, LoadGameInstance->CharacterStats.Level);
	Player->SetStat(EPlayerStat::Exp, LoadGameInstance->CharacterStats.Exp);
	Player->SetStat(EPlayerStat::MaxExp, LoadGameInstance->CharacterStats.MaxExp);
	Player->SetStat(EPlayerStat::PotionNum, LoadGameInstance->CharacterStats.PotionNum);

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
		if (ItemName.Contains("Yellow") && Player->GetItemMap()->Contains("YellowStone")) // ������ �� �տ� ���� ���� ���¿�����
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
	else // ��� �̵� ��
	{
		bIsSaveAllowed = false;
	}
}