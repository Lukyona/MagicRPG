// Fill out your copyright notice in the Description page of Project Settings.


#include "Yaro/System/GameManager.h"
#include "Kismet/GameplayStatics.h"
#include "Yaro/System/DialogueManager.h"
#include "Yaro/System/YaroSaveGame.h"


void UGameManager::Init()
{
	Super::Init();

    // 다이얼로그 매니저 생성 및 초기화
    DialogueManager = NewObject<UDialogueManager>(this, UDialogueManager::StaticClass());
	DialogueManager->Init();
}

void UGameManager::SaveGame()
{
	if (DialogueManager->GetDialogueNum() >= 23) return;

	if (DialogueManager->IsDialogueUIVisible() || MainPlayerController->bFallenPlayer || MainAnimInstance->bIsInAir
		|| DialogueManager->GetDialogueNum() == 21)
	{
		GetWorld()->GetTimerManager().SetTimer(SaveTimer, this, &UGameManager::SaveGame, 1.f, false);

		return;
	}

	//UE_LOG(LogTemp, Log, TEXT("SaveGame"));

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

	SaveGameInstance->DialogueNum = MainPlayerController->DialogueNum;
	SaveGameInstance->CharacterStats.FallingCount = MainPlayerController->FallingCount;

	SaveGameInstance->CharacterStats.Location = GetActorLocation();
	SaveGameInstance->CharacterStats.Rotation = GetActorRotation();

	if (MainPlayerController->DialogueNum < 23)
	{
		SaveGameInstance->NpcInfo.MomoLocation = Momo->GetActorLocation();
		SaveGameInstance->NpcInfo.LukoLocation = Luko->GetActorLocation();
		SaveGameInstance->NpcInfo.VovoLocation = Vovo->GetActorLocation();
		SaveGameInstance->NpcInfo.ViviLocation = Vivi->GetActorLocation();
		SaveGameInstance->NpcInfo.ZiziLocation = Zizi->GetActorLocation();
	}

	if (MainPlayerController->DialogueNum < 4)
		SaveGameInstance->NpcInfo.TeamMoveIndex = Vivi->GetIndex();

	SaveGameInstance->DeadEnemyList = Enemies;

	if (ItemInHand)
	{
		SaveGameInstance->CharacterStats.ItemName = ItemInHand->GetName();
	}

	UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveGameInstance->SaveName, SaveGameInstance->UserIndex);

	if (MainPlayerController->DialogueNum == 18 && MainPlayerController->SystemMessageNum == 12) return;

	GetWorld()->GetTimerManager().SetTimer(SaveTimer, this, &AMain::SaveGame, 1.f, false);
}

void UGameManager::LoadGame()
{
	UYaroSaveGame* LoadGameInstance = Cast<UYaroSaveGame>(UGameplayStatics::CreateSaveGameObject(UYaroSaveGame::StaticClass()));

	LoadGameInstance = Cast<UYaroSaveGame>(UGameplayStatics::LoadGameFromSlot(LoadGameInstance->SaveName, LoadGameInstance->UserIndex));

	MainPlayerController = Cast<AMainPlayerController>(GetController());
	MainPlayerController->DialogueNum = LoadGameInstance->DialogueNum;
	MainPlayerController->FallingCount = LoadGameInstance->CharacterStats.FallingCount;

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


	Enemies = LoadGameInstance->DeadEnemyList;

	if (MainPlayerController->DialogueNum < 4)
	{
		Vovo->SetIndex(LoadGameInstance->NpcInfo.TeamMoveIndex);
		Vivi->SetIndex(LoadGameInstance->NpcInfo.TeamMoveIndex);
		Zizi->SetIndex(LoadGameInstance->NpcInfo.TeamMoveIndex);
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
		if (MainPlayerController->DialogueNum == 15 && UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("boss")) return;

		SetActorLocation(LoadGameInstance->CharacterStats.Location);
		SetActorRotation(LoadGameInstance->CharacterStats.Rotation);

		if (MainPlayerController->DialogueNum == 19) return;

		Momo->SetActorLocation(LoadGameInstance->NpcInfo.MomoLocation);
		Luko->SetActorLocation(LoadGameInstance->NpcInfo.LukoLocation);
		Vovo->SetActorLocation(LoadGameInstance->NpcInfo.VovoLocation);
		Vivi->SetActorLocation(LoadGameInstance->NpcInfo.ViviLocation);
		Zizi->SetActorLocation(LoadGameInstance->NpcInfo.ZiziLocation);
	}
}