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

	TWeakObjectPtr<UGameManager> WeakGameManager = this;

	// 비동기 작업으로 메인 스레드에서 초기화 진행
	AsyncTask(ENamedThreads::GameThread, [WeakGameManager]()
	{
		if (WeakGameManager.IsValid())
		{
			UGameManager* GameManager = WeakGameManager.Get();

			// DialogueManager 생성 및 유효성 체크
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

			// UIManager 생성 및 유효성 체크
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

			// NPCManager 생성 및 유효성 체크
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

	if (SaveTimer.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(SaveTimer);
		SaveTimer.Invalidate();
	}

	UYaroSaveGame* SaveGameInstance = Cast<UYaroSaveGame>(UGameplayStatics::CreateSaveGameObject(UYaroSaveGame::StaticClass()));

	SaveGameInstance->PlayerGender = Player->GetStat(EPlayerStat::Gender);
	SaveGameInstance->CharacterStats.HP = Player->GetStat(EPlayerStat::HP);
	SaveGameInstance->CharacterStats.MP = Player->GetStat(EPlayerStat::MP);
	SaveGameInstance->CharacterStats.SP = Player->GetStat(EPlayerStat::SP);
	SaveGameInstance->CharacterStats.Level = Player->GetStat(EPlayerStat::Level);
	SaveGameInstance->CharacterStats.Exp = Player->GetStat(EPlayerStat::Exp);
	SaveGameInstance->CharacterStats.PotionNum = Player->GetStat(EPlayerStat::PotionNum);

	SaveGameInstance->WorldName = FName(*GetWorld()->GetName());
	SaveGameInstance->DialogueNum = DialogueManager->GetDialogueNum();
	SaveGameInstance->CharacterStats.FallCount = Player->GetFallCount();

	SaveGameInstance->CharacterStats.Location = Player->GetActorLocation();
	SaveGameInstance->CharacterStats.Rotation = Player->GetActorRotation();

	if (DialogueManager->GetDialogueNum() < 23 && DialogueManager->GetDialogueNum() != 19)
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

	if (DialogueManager->GetDialogueNum() == 18 && UIManager->GetSystemMessageNum() == 13) return;

	GetWorld()->GetTimerManager().SetTimer(SaveTimer, this, &UGameManager::SaveGame, 1.f, false);
}

void UGameManager::LoadGame()
{
	UYaroSaveGame* LoadGameInstance = Cast<UYaroSaveGame>(UGameplayStatics::CreateSaveGameObject(UYaroSaveGame::StaticClass()));

	LoadGameInstance = Cast<UYaroSaveGame>(UGameplayStatics::LoadGameFromSlot(LoadGameInstance->SaveName, LoadGameInstance->UserIndex));

	DialogueManager->SetDialogueNum(LoadGameInstance->DialogueNum);
	Player->SetFallCount(LoadGameInstance->CharacterStats.FallCount);

	Player->SetStat(EPlayerStat::HP, LoadGameInstance->CharacterStats.HP);
	Player->SetStat(EPlayerStat::MP, LoadGameInstance->CharacterStats.MP);
	Player->SetStat(EPlayerStat::SP, LoadGameInstance->CharacterStats.SP);
	Player->SetStat(EPlayerStat::Level, LoadGameInstance->CharacterStats.Level);
	Player->SetStat(EPlayerStat::Exp, LoadGameInstance->CharacterStats.Exp);
	Player->SetStat(EPlayerStat::PotionNum, LoadGameInstance->CharacterStats.PotionNum);

	DeadEnemies = LoadGameInstance->DeadEnemyList;

	if (DialogueManager->GetDialogueNum() < 4)
	{
		NPCManager->GetNPC("Vovo")->SetIndex(LoadGameInstance->NpcInfo.TeamMoveIndex);
		NPCManager->GetNPC("Vivi")->SetIndex(LoadGameInstance->NpcInfo.TeamMoveIndex);
		NPCManager->GetNPC("Zizi")->SetIndex(LoadGameInstance->NpcInfo.TeamMoveIndex);
	}

	FString ItemName = LoadGameInstance->CharacterStats.ItemName;
	if (Player->GetItemMap() != nullptr)
	{
		if (ItemName.Contains("Yellow") && Player->GetItemMap()->Contains("YellowStone")) // 저장할 때 손에 돌을 집은 상태였으면
		{
			AItem* Item = GetWorld()->SpawnActor<AItem>(Player->GetItemMap()->FindRef("YellowStone"));
			Item->PickUp(Player);
		}
	}

	if (DialogueManager->GetDialogueNum() == 15 && UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("boss")) return;
	if (DialogueManager->GetDialogueNum() == 4 && UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("second")) return;

	Player->SetActorLocation(LoadGameInstance->CharacterStats.Location);
	Player->SetActorRotation(LoadGameInstance->CharacterStats.Rotation);

	if (DialogueManager->GetDialogueNum() == 19) return;

	NPCManager->SetNPCLocation("Momo", LoadGameInstance->NpcInfo.MomoLocation);
	NPCManager->SetNPCLocation("Luko", LoadGameInstance->NpcInfo.LukoLocation);
	NPCManager->SetNPCLocation("Vovo", LoadGameInstance->NpcInfo.VovoLocation);
	NPCManager->SetNPCLocation("Vivi", LoadGameInstance->NpcInfo.ViviLocation);
	NPCManager->SetNPCLocation("Zizi", LoadGameInstance->NpcInfo.ZiziLocation);

	if (DialogueManager->GetDialogueNum() == 4)
	{
		bIsSaveAllowed = false;
	}
}

void UGameManager::UpdateDeadEnemy(EEnemyType EnemyType)
{
	const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EEnemyType"), true);
	//FString type = EnumPtr->GetNameStringByIndex(static_cast<uint8>(EnemyType));

	if (DeadEnemies.Find(EnemyType))
	{
		DeadEnemies[EnemyType]++;
	}
	else
	{
		DeadEnemies.Add(EnemyType, 1);
	}
}

void UGameManager::SkipCombat() // 전투 스킵, 몬스터 제거
{
	if (DialogueManager->IsDialogueUIVisible() || !IsSkippable() || IsSkipping()
		|| Player->IsDead()) return;

	if (DialogueManager->GetDialogueNum() < 4) // first dungeon
	{
		if (DialogueManager->GetDialogueNum() <= 2) return;
		SkipFirstDungeon.Broadcast();
	}
	else if (DialogueManager->GetDialogueNum() < 15)
	{
		if (DialogueManager->GetDialogueNum() <= 10) return;
		SkipSecondDungeon.Broadcast();
	}
	else if (DialogueManager->GetDialogueNum() < 19)
	{
		if (DialogueManager->GetDialogueNum() <= 17) return;
		SkipFinalDungeon.Broadcast();
	}
	else return;
}

void UGameManager::StartFirstDungeon()
{
	if (DialogueManager->GetDialogueNum() == 3 && UIManager->IsSystemMessageVisible())
	{
		UIManager->RemoveSystemMessage();

		MainPlayerController->SetCinematicMode(false, true, true);

		NPCManager->GetNPC("Vovo")->MoveToLocation();
		NPCManager->GetNPC("Vivi")->MoveToLocation();
		NPCManager->GetNPC("Zizi")->MoveToLocation();

		NPCManager->GetNPC("Momo")->MoveToPlayer();
		NPCManager->GetNPC("Luko")->MoveToPlayer();

		FString SoundPath = TEXT("/Game/SoundEffectsAndBgm/the-buccaneers-haul.the-buccaneers-haul");
		USoundBase* LoadedSound = LoadObject<USoundBase>(nullptr, *SoundPath);
		if (LoadedSound)
		{
			UGameplayStatics::PlaySound2D(this, LoadedSound);
		}
	}
}

void UGameManager::EscapeToSafeLocation() // 두 번째 던전에서의 긴급 탈출
{
	if (DialogueManager->GetDialogueNum() >= 6 && !DialogueManager->IsDialogueUIVisible() && !Player->IsDead())
	{
		if (DialogueManager->GetDialogueNum() <= 8)
		{
			Player->SetActorLocation(FVector(4620.f, -3975.f, -2117.f));
		}
		else if (DialogueManager->GetDialogueNum() <= 11)
		{
			Player->SetActorLocation(FVector(5165.f, -2307.f, -2117.f));
		}
		else if (DialogueManager->GetDialogueNum() <= 15)
		{
			Player->SetActorLocation(FVector(2726.f, -3353.f, -500.f));
		}
	}
}
