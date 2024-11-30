// Fill out your copyright notice in the Description page of Project Settings.


#include "Yaro/System/GameManager.h"
#include "Kismet/GameplayStatics.h"
#include "Tickable.h"
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
	
	DialogueManager = CreateManager<UDialogueManager>(this);
	UIManager = CreateManager<UUIManager>(this);
	NPCManager = CreateManager<UNPCManager>(this);

	TickerHandle = FTicker::GetCoreTicker().AddTicker(
	FTickerDelegate::CreateUObject(this, &UGameManager::Tick), 0.0f); // 0.0f: 매 프레임 호출

}

void UGameManager::Shutdown()
{
	Super::Shutdown();

	FTicker::GetCoreTicker().RemoveTicker(TickerHandle);
	TickerHandle.Reset();

	if (DialogueManager && IsValid(DialogueManager))
	{
		DialogueManager->RemoveFromRoot();
		DialogueManager = nullptr;
		UDialogueManager::Instance = nullptr;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("DialogueManager is already null or invalid during shutdown."));
	}

	if (UIManager && IsValid(UIManager))
	{
		UIManager->RemoveFromRoot();
		UIManager = nullptr;
		UUIManager::Instance = nullptr;

	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UIManager is already null or invalid during shutdown."));
	}

	if (NPCManager && IsValid(NPCManager))
	{
		NPCManager->RemoveFromRoot();
		NPCManager = nullptr;
		UNPCManager::Instance = nullptr;

	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("NPCManager is already null or invalid during shutdown."));
	}

}

void UGameManager::StartGameInstance()
{
	Super::StartGameInstance();
}

bool UGameManager::Tick(float DeltaTime)
{
	if (GetWorld()->GetName().Contains("Start"))
	{
		return true;
	}

	if (DialogueManager && IsValid(DialogueManager))
	{
		DialogueManager->Tick();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("DialogueManager is null or invalid during Tick."));
	}

	if (UIManager && IsValid(UIManager))
	{
		UIManager->Tick();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UIManager is null or invalid during Tick."));
	}
	return true; // true를 반환하면 계속 호출됨
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
	const int32 DialogueNum = DialogueManager->GetDialogueNum();

	if (DialogueNum >= 23 || !bIsSaveAllowed)
	{
		return;
	}

	if (DialogueManager->IsDialogueUIVisible() || DialogueNum == 21
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
	if (!SaveGameInstance)
	{
		return;
	}

	SavePlayerInfo(SaveGameInstance);

	SaveGameInstance->WorldName = FName(*GetWorld()->GetName());
	SaveGameInstance->DialogueNum = DialogueNum;

	if (DialogueNum < 23 && DialogueNum != 19)
	{
		SaveNPCLocations(SaveGameInstance);
	}

	if (DialogueNum < 4)
	{
		SaveGameInstance->NpcInfo.TeamMoveIndex = NPCManager->GetNPC("Vivi")->GetTeamMoveIndex();
	}

	SaveGameInstance->DeadEnemyList = DeadEnemies;

	if (Player->GetItemInHand() != nullptr)
	{
		SaveGameInstance->CharacterStats.ItemName = Player->GetItemInHand()->GetName();
	}

	UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveGameInstance->SaveName, SaveGameInstance->UserIndex);

	if (DialogueNum == 18 && UIManager->GetSystemMessageNum() == 13)
	{
		return;
	}

	GetWorld()->GetTimerManager().SetTimer(SaveTimer, this, &UGameManager::SaveGame, 1.f, false);
}

void UGameManager::LoadGame()
{
	UYaroSaveGame* LoadGameInstance = Cast<UYaroSaveGame>(UGameplayStatics::LoadGameFromSlot("Default", 0));
	if (!LoadGameInstance)
	{
		return;
	}

	DialogueManager->SetDialogueNum(LoadGameInstance->DialogueNum);

	LoadPlayerInfo(LoadGameInstance);

	DeadEnemies = LoadGameInstance->DeadEnemyList;

	const int32 DialogueNum = DialogueManager->GetDialogueNum();

	if (DialogueNum < 4)
	{
		NPCManager->GetNPC("Vovo")->SetTeamMoveIndex(LoadGameInstance->NpcInfo.TeamMoveIndex);
		NPCManager->GetNPC("Vivi")->SetTeamMoveIndex(LoadGameInstance->NpcInfo.TeamMoveIndex);
		NPCManager->GetNPC("Zizi")->SetTeamMoveIndex(LoadGameInstance->NpcInfo.TeamMoveIndex);
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

	if ((DialogueNum == 15 && UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("boss"))
		|| (DialogueNum == 4 && UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("second")))
	{
		return;
	}

	Player->SetActorLocation(LoadGameInstance->CharacterStats.Location);
	Player->SetActorRotation(LoadGameInstance->CharacterStats.Rotation);

	if (DialogueNum == 19) 
	{
		return;
	}

	LoadNPCLocations(LoadGameInstance);

	if (DialogueNum == 4)
	{
		bIsSaveAllowed = false;
	}
}

void UGameManager::SavePlayerInfo(UYaroSaveGame* SaveGameInstance)
{
	SaveGameInstance->PlayerGender = Player->GetStat(EPlayerStat::Gender);
	SaveGameInstance->CharacterStats.HP = Player->GetStat(EPlayerStat::HP);
	SaveGameInstance->CharacterStats.MP = Player->GetStat(EPlayerStat::MP);
	SaveGameInstance->CharacterStats.SP = Player->GetStat(EPlayerStat::SP);
	SaveGameInstance->CharacterStats.Level = Player->GetStat(EPlayerStat::Level);
	SaveGameInstance->CharacterStats.Exp = Player->GetStat(EPlayerStat::Exp);
	SaveGameInstance->CharacterStats.PotionNum = Player->GetStat(EPlayerStat::PotionNum);
	SaveGameInstance->CharacterStats.FallCount = Player->GetFallCount();
	SaveGameInstance->CharacterStats.Location = Player->GetActorLocation();
	SaveGameInstance->CharacterStats.Rotation = Player->GetActorRotation();
}

void UGameManager::LoadPlayerInfo(UYaroSaveGame* LoadGameInstance)
{
	Player->SetStat(EPlayerStat::HP, LoadGameInstance->CharacterStats.HP);
	Player->SetStat(EPlayerStat::MP, LoadGameInstance->CharacterStats.MP);
	Player->SetStat(EPlayerStat::SP, LoadGameInstance->CharacterStats.SP);
	Player->SetStat(EPlayerStat::Level, LoadGameInstance->CharacterStats.Level);
	Player->SetStat(EPlayerStat::Exp, LoadGameInstance->CharacterStats.Exp);
	Player->SetStat(EPlayerStat::PotionNum, LoadGameInstance->CharacterStats.PotionNum);
	Player->SetFallCount(LoadGameInstance->CharacterStats.FallCount);
}

void UGameManager::SaveNPCLocations(UYaroSaveGame* SaveGameInstance)
{
	SaveGameInstance->NpcInfo.MomoLocation = NPCManager->GetNPC("Momo")->GetActorLocation();
	SaveGameInstance->NpcInfo.LukoLocation = NPCManager->GetNPC("Luko")->GetActorLocation();
	SaveGameInstance->NpcInfo.VovoLocation = NPCManager->GetNPC("Vovo")->GetActorLocation();
	SaveGameInstance->NpcInfo.ViviLocation = NPCManager->GetNPC("Vivi")->GetActorLocation();
	SaveGameInstance->NpcInfo.ZiziLocation = NPCManager->GetNPC("Zizi")->GetActorLocation();
}

void UGameManager::LoadNPCLocations(UYaroSaveGame* LoadGameInstance)
{
	NPCManager->SetNPCLocation("Momo", LoadGameInstance->NpcInfo.MomoLocation);
	NPCManager->SetNPCLocation("Luko", LoadGameInstance->NpcInfo.LukoLocation);
	NPCManager->SetNPCLocation("Vovo", LoadGameInstance->NpcInfo.VovoLocation);
	NPCManager->SetNPCLocation("Vivi", LoadGameInstance->NpcInfo.ViviLocation);
	NPCManager->SetNPCLocation("Zizi", LoadGameInstance->NpcInfo.ZiziLocation);
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
	if (DialogueManager->IsDialogueUIVisible() || !IsSkippable() || IsSkipping() || Player->IsDead()) 
	{
		return;
	}

	const int32 DialogueNum = DialogueManager->GetDialogueNum();

	if (DialogueNum < 4) // first dungeon
	{
		if (DialogueNum <= 2) 
		{
			return;
		}
		SkipFirstDungeon.Broadcast();
	}
	else if (DialogueNum < 15)
	{
		if (DialogueNum <= 10) 
		{
			return;
		}
		SkipSecondDungeon.Broadcast();
	}
	else if (DialogueNum < 19)
	{
		if (DialogueNum <= 17) 
		{
			return;
		}
		SkipFinalDungeon.Broadcast();
	}
	else
	{
		return;
	}
}

void UGameManager::StartFirstDungeon()
{
	if (DialogueManager->GetDialogueNum() == 3 && UIManager->IsSystemMessageVisible())
	{
		UIManager->RemoveSystemMessage();

		MainPlayerController->SetCinematicMode(false, true, true);

		NPCManager->GetNPC("Vovo")->MoveToTeamPos();
		NPCManager->GetNPC("Vivi")->MoveToTeamPos();
		NPCManager->GetNPC("Zizi")->MoveToTeamPos();

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
	const int32 DialogueNum = DialogueManager->GetDialogueNum();
	if (DialogueNum >= 6 && !DialogueManager->IsDialogueUIVisible() && !Player->IsDead())
	{
		if (DialogueNum <= 8)
		{
			Player->SetActorLocation(SafeLocationList[0]);
		}
		else if (DialogueNum <= 11)
		{
			Player->SetActorLocation(SafeLocationList[1]);
		}
		else if (DialogueNum <= 15)
		{
			Player->SetActorLocation(SafeLocationList[2]);
		}
	}
}
