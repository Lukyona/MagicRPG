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
		CleanupManager(DialogueManager);
	}

	if (UIManager && IsValid(UIManager))
	{
		CleanupManager(UIManager);
	}

	if (NPCManager && IsValid(NPCManager))
	{
		CleanupManager(NPCManager);
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
	const EDialogueState DialogueState = static_cast<EDialogueState>(DialogueManager->GetDialogueNum());

	if (DialogueState >= EDialogueState::FinalLine || !bIsSaveAllowed)
	{
		return;
	}

	if (DialogueManager->IsDialogueUIVisible() || DialogueState == EDialogueState::AfterTookTheStone
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
	SaveGameInstance->DialogueNum = DialogueManager->GetDialogueNum();

	if (DialogueState < EDialogueState::FinalLine && DialogueState != EDialogueState::BackToCave)
	{
		SaveNPCLocations(SaveGameInstance);
	}

	if (DialogueState < EDialogueState::MoveToBoat)
	{
		SaveGameInstance->NpcInfo.TeamMoveIndex = NPCManager->GetNPC("Vivi")->GetTeamMoveIndex();
	}

	SaveGameInstance->DeadEnemyList = DeadEnemies;

	if (Player->GetItemInHand() != nullptr)
	{
		SaveGameInstance->CharacterStats.ItemName = Player->GetItemInHand()->GetName();
	}

	UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveGameInstance->SaveName, SaveGameInstance->UserIndex);

	if (DialogueState == EDialogueState::AfterCombatWithBoss && UIManager->GetSystemMessageNum() == 13)
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

	const EDialogueState DialogueState = static_cast<EDialogueState>(DialogueManager->GetDialogueNum());

	if (DialogueState < EDialogueState::MoveToBoat)
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

	if ((DialogueState == EDialogueState::SecondDungeonFinished && UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("boss"))
		|| (DialogueState == EDialogueState::MoveToBoat && UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("second")))
	{
		return;
	}

	Player->SetActorLocation(LoadGameInstance->CharacterStats.Location);
	Player->SetActorRotation(LoadGameInstance->CharacterStats.Rotation);

	if (DialogueState == EDialogueState::BackToCave)
	{
		return;
	}

	LoadNPCLocations(LoadGameInstance);

	if (DialogueState == EDialogueState::MoveToBoat)
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

	const EDialogueState DialogueState = static_cast<EDialogueState>(DialogueManager->GetDialogueNum());

	if (DialogueState < EDialogueState::MoveToBoat) // first dungeon
	{
		if (DialogueState <= EDialogueState::BeforeFirstDungeon)
		{
			return;
		}
		SkipFirstDungeon.Broadcast();
	}
	else if (DialogueState < EDialogueState::SecondDungeonFinished)
	{
		if (DialogueState <= EDialogueState::NPCMoveToBridge)
		{
			return;
		}
		SkipSecondDungeon.Broadcast();
	}
	else if (DialogueState < EDialogueState::BackToCave)
	{
		if (DialogueState <= EDialogueState::ReadyToFightWithBoss)
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
	const EDialogueState DialogueState = static_cast<EDialogueState>(DialogueManager->GetDialogueNum());

	if (DialogueState == EDialogueState::FirstDungeonStarted
		&& UIManager->IsSystemMessageVisible())
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
	const EDialogueState DialogueState = static_cast<EDialogueState>(DialogueManager->GetDialogueNum());
	if (DialogueState >= EDialogueState::FirstDungeonStarted
		&& !DialogueManager->IsDialogueUIVisible() && !Player->IsDead())
	{
		if (DialogueState <= EDialogueState::PlayerJumpToPlatform)
		{
			Player->SetActorLocation(SafeLocationList[0]);
		}
		else if (DialogueState <= EDialogueState::NPCCrossedBridge)
		{
			Player->SetActorLocation(SafeLocationList[1]);
		}
		else if (DialogueState <= EDialogueState::SecondDungeonFinished)
		{
			Player->SetActorLocation(SafeLocationList[2]);
		}
	}
}
