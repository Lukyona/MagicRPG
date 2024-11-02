// Fill out your copyright notice in the Description page of Project Settings.


#include "System/NPCManager.h"
#include "Containers/Map.h"
#include "EngineUtils.h"   // TActorIterator 사용을 위한 헤더
#include "AIController.h"
#include "GameFramework/Actor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/Main.h"
#include "System/GameManager.h"
#include "System/DialogueManager.h"
#include "System/UIManager.h"

UNPCManager* UNPCManager::Instance = nullptr;

void UNPCManager::Init()
{
	if (GameManager)
	{
		DialogueManager = GameManager->GetDialogueManager();
		UIManager = GameManager->GetUIManager();
	}
}

void UNPCManager::SetGameManager(UGameManager* Manager)
{
	this->GameManager = Manager;
}

FString UNPCManager::EnumToString(const FString& EnumName, ENPCType EnumValue)
{
    const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, *EnumName, true);
    if (!EnumPtr) return FString("Invalid");

    return EnumPtr->GetNameStringByIndex(static_cast<int32>(EnumValue));
}

void UNPCManager::InitializeNPCs(UWorld* World)
{
	NPCMap.Empty();

	if (!World->GetName().Contains("Start") && NPCMap.Num() == 0)
	{
		for (TActorIterator<AYaroCharacter> It(World); It; ++It)
		{
			AYaroCharacter* NPC = *It;
			if (NPC)
			{
				FString NPCName = EnumToString(TEXT("ENPCType"), NPC->GetType());
				AddNPC(NPCName, NPC);
			}
		}

		Momo = GetNPC("Momo");
		Luko = GetNPC("Luko");
		Vovo = GetNPC("Vovo");
		Vivi = GetNPC("Vivi");
		Zizi = GetNPC("Zizi");
	}
}

void UNPCManager::AddNPC(FString NPCName, class AYaroCharacter* NPC)
{
    NPCMap.Add(NPCName, NPC);
}

AYaroCharacter* UNPCManager::GetNPC(ENPCType NPCType)
{
	FString NPCName = EnumToString(TEXT("ENPCType"), NPCType);
    return NPCMap.Contains(NPCName) ? NPCMap[NPCName] : nullptr;
}

AYaroCharacter* UNPCManager::GetNPC(FString NPCName) const
{
	return NPCMap.Contains(NPCName) ? NPCMap[NPCName] : nullptr;
}

void UNPCManager::MoveNPCToLocation(FString NPCName, FVector Location)
{
	GetNPC(NPCName)->GetAIController()->MoveToLocation(Location);
}

void UNPCManager::MoveNPCToLocation(AYaroCharacter* NPC, FVector Location)
{
	NPC->GetAIController()->MoveToLocation(Location);

}
void UNPCManager::SetNPCLocation(FString NPCName, FVector Location)
{
	GetNPC(NPCName)->SetActorLocation(Location);

}
void UNPCManager::SetNPCLocation(AYaroCharacter* NPC, FVector Location)
{
	NPC->SetActorLocation(Location);
}

bool UNPCManager::IsNPCInTalkRange()
{
	for (auto NPC : NPCMap)
	{
		float distance = GameManager->GetPlayer()->GetDistanceTo(NPC.Value);

		if (distance >= 1300.f) // npc와 너무 멀면 대화 불가
			return false;
	}
	return true;
}

void UNPCManager::SetPositionsForDialogue()
{
	AllNpcStopFollowPlayer();
	for (auto NPC : NPCMap)
	{
		if(NPC.Key != "Momo" && NPC.Key != "Luko")
			NPC.Value->ClearTeamMoveTimer();
	}

	int dialogueNum = DialogueManager->GetDialogueNum();
	AMain* Player = GameManager->GetPlayer();

	if (dialogueNum == 3) // after golem battle
	{
		Player->SetActorLocation(FVector(646.f, -1747.f, 2578.f));
		Player->SetActorRotation(FRotator(0.f, 57.f, 0.f)); // y(pitch), z(yaw), x(roll)

		Momo->SetActorLocation(FVector(594.f, -1543.f, 2531.f));
		Momo->SetActorRotation(FRotator(0.f, 280.f, 0.f));

		Luko->SetActorLocation(FVector(494.f, -1629.f, 2561.f));
		Luko->SetActorRotation(FRotator(0.f, 6.f, 0.f));

		Vovo->SetActorLocation(FVector(903.f, -1767.f, 2574.f));
		Vovo->SetActorRotation(FRotator(0.f, 165.f, 0.f));

		Vivi->SetActorLocation(FVector(790.f, -1636.f, 2566.f));
		Vivi->SetActorRotation(FRotator(00.f, 180.f, 0.f));

		Zizi->SetActorLocation(FVector(978.f, -1650.f, 2553.f));
		Zizi->SetActorRotation(FRotator(0.f, 187.f, 0.f));
	}

	if (dialogueNum == 11)
	{
		Player->SetActorLocation(FVector(-137.f, -2833.f, -117.f));
		Player->SetActorRotation(FRotator(0.f, 170.f, 0.f)); // y(pitch), z(yaw), x(roll)

		Momo->SetActorLocation(FVector(-329.f, -2512.f, -122.f));
		Momo->SetActorRotation(FRotator(0.f, 80.f, 0.f));

		Luko->SetActorLocation(FVector(-242.f, -2692.f, -117.f));
		Luko->SetActorRotation(FRotator(0.f, 85.f, 0.f));

		Vovo->SetActorLocation(FVector(-313.f, -2755.f, -117.5f));
		Vovo->SetActorRotation(FRotator(0.f, 83.f, 0.f));

		Vivi->SetActorLocation(FVector(-343.f, -2849.f, -117.5f));
		Vivi->SetActorRotation(FRotator(0.f, 77.f, 0.f));

		Zizi->SetActorLocation(FVector(-323.f, -2949.f, -115.f));
		Zizi->SetActorRotation(FRotator(0.f, 85.f, 0.f));
	}

	if (dialogueNum == 16)
	{
		Player->SetActorLocation(FVector(-35.f, 3549.f, 184.f));
		Player->SetActorRotation(FRotator(0.f, 265.f, 0.f)); // y(pitch), z(yaw), x(roll)

		Momo->SetActorLocation(FVector(-86.f, 3263.f, 177.f));
		Momo->SetActorRotation(FRotator(0.f, 272.f, 0.f));

		Luko->SetActorLocation(FVector(184.f, 3317.f, 182.f));
		Luko->SetActorRotation(FRotator(0.f, 260.f, 0.f));

		Vovo->SetActorLocation(FVector(-140.f, 3370.f, 182.f));
		Vovo->SetActorRotation(FRotator(0.f, 270.f, 0.f));

		Vivi->SetActorLocation(FVector(105.f, 3176.f, 182.f));
		Vivi->SetActorRotation(FRotator(0.f, 271.f, 0.f));

		Zizi->SetActorLocation(FVector(68.f, 3398.f, 184.f));
		Zizi->SetActorRotation(FRotator(0.f, 268.f, 0.f));

	}

	if (dialogueNum == 18)
	{
		Player->SetActorLocation(FVector(16.f, -2734.f, 582.f));
		Player->SetActorRotation(FRotator(0.f, 270.f, 0.f)); // y(pitch), z(yaw), x(roll)

		Momo->SetActorLocation(FVector(-23.f, -3006.f, 603.f));
		Momo->SetActorRotation(FRotator(0.f, 82.f, 0.f));

		Luko->SetActorLocation(FVector(146.f, -2832.f, 582.f));
		Luko->SetActorRotation(FRotator(0.f, 187.f, 0.f));

		Vovo->SetActorLocation(FVector(-105.f, -2942.f, 581.f));
		Vovo->SetActorRotation(FRotator(0.f, 10.f, 0.f));

		Vivi->SetActorLocation(FVector(104.f, -2982.f, 581.f));
		Vivi->SetActorRotation(FRotator(0.f, 120.f, 0.f));

		Zizi->SetActorLocation(FVector(-125.f, -2804.f, 582.f));
		Zizi->SetActorRotation(FRotator(0.f, 0.f, 0.f));

	}

	if (dialogueNum == 19)
	{
		Player->SetActorLocation(FVector(4658.f, 41.f, 148.f));
		Player->SetActorRotation(FRotator(-3.f, 180.f, 0.f)); // y(pitch), z(yaw), x(roll)

		Momo->SetActorLocation(FVector(4247.f, 94.f, 98.f));
		Momo->SetActorRotation(FRotator(0.f, 180.f, 0.f));

		Luko->SetActorLocation(FVector(4345.f, -119.f, 108.f));
		Luko->SetActorRotation(FRotator(0.f, 180.f, 0.f));

		Vovo->SetActorLocation(FVector(4633.f, 121.f, 151.f));
		Vovo->SetActorRotation(FRotator(0.f, 180.f, 0.f));

		Vivi->SetActorLocation(FVector(4634.f, -105.f, 152.f));
		Vivi->SetActorRotation(FRotator(0.f, 170.f, 0.f));

		Zizi->SetActorLocation(FVector(4493.f, -26.f, 156.f));
		Zizi->SetActorRotation(FRotator(0.f, 175.f, 0.f));
	}

	if (dialogueNum == 20)
	{
		Player->SetActorLocation(FVector(-4446.f, -20.f, 401.f));
		Player->SetActorRotation(FRotator(2.f, 180.f, 0.f)); // y(pitch), z(yaw), x(roll)

		Momo->SetActorLocation(FVector(-4660.f, 118.f, 393.f));
		Momo->SetActorRotation(FRotator(0.f, 296.f, 0.f));

		Luko->SetActorLocation(FVector(-4545.f, -281.f, 401.f));
		Luko->SetActorRotation(FRotator(0.f, 97.f, 0.f));

		Vovo->SetActorLocation(FVector(-4429.f, 103.f, 396.f));
		Vovo->SetActorRotation(FRotator(0.f, 219.f, 0.f));

		Vivi->SetActorLocation(FVector(-4355.f, -195.f, 405.f));
		Vivi->SetActorRotation(FRotator(0.f, 145.f, 0.f));

		Zizi->SetActorLocation(FVector(-4695.f, -190.f, 394.f));
		Zizi->SetActorRotation(FRotator(0.f, 49.f, 0.f));
	}

}

void UNPCManager::UpdateNPCPositions(int DialogueNum) // 저장된 진행도에 따른 npc들 이동 및 위치 설정
{
	uint32 EnemyCount = 0;
	uint32 DeadEnemiesNum = GameManager->GetDeadEnemies().Num();

	switch (DialogueNum)
	{
	case 3: // after golem died
		for (auto Enemy : GameManager->GetDeadEnemies())
		{
			if (Enemy.Contains("Golem") && DeadEnemiesNum == 9)
			{
				MoveNPCToLocation(Momo, FVector(594.f, -1543.f, 2531.f));
				MoveNPCToLocation(Luko, FVector(494.f, -1629.f, 2561.f));
				MoveNPCToLocation(Vovo, FVector(903.f, -1767.f, 2574.f));
				MoveNPCToLocation(Vivi, FVector(790.f, -1636.f, 2566.f));
				MoveNPCToLocation(Zizi, FVector(978.f, -1650.f, 2553.f));

				DialogueManager->DisplayDialogueUI();
				return;
			}
		}
		Luko->MoveToPlayer();
		Momo->MoveToPlayer();
		Vovo->MoveToLocation();
		Vivi->MoveToLocation();
		Zizi->MoveToLocation();
		//NpcGo = true;
		break;
	case 4: // npc move to boat and wait player
		Momo->SetActorRotation(FRotator(0.f, 85.f, 0.f));
		Luko->SetActorRotation(FRotator(0.f, 103.f, 0.f));
		Vivi->SetActorRotation(FRotator(0.f, 97.f, 0.f));
		Zizi->SetActorRotation(FRotator(0.f, 94.f, 0.f));
		Vovo->SetActorRotation(FRotator(0.f, 91.f, 0.f));

		MoveNPCToLocation(Momo, FVector(660.f, 1035.f, 1840.f));
		MoveNPCToLocation(Luko, FVector(598.f, 1030.f, 1840.f));
		MoveNPCToLocation(Vovo, FVector(630.f, 970.f, 1840.f));
		MoveNPCToLocation(Vivi, FVector(710.f, 995.f, 1840.f));
		MoveNPCToLocation(Zizi, FVector(690.f, 930.f, 1840.f));
		break;
	case 9: //if ItemInHand is null, the stone have to put on the floor (this is check in blueprint)
		UIManager->SetSystemMessage(10);
		AllNpcLookAtPlayer();
		AllNpcStopFollowPlayer();
		MoveNPCToLocation(Momo, FVector(5307.f, -3808.f, -2122.f));
		MoveNPCToLocation(Luko, FVector(5239.f, -3865.f, -2117.f));
		MoveNPCToLocation(Vovo, FVector(5433.f, -3855.f, -2117.f));
		MoveNPCToLocation(Vivi, FVector(5392.f, -3686.f, -2117.f));
		MoveNPCToLocation(Zizi, FVector(5538.f, -3696.f, -2115.f));
		break;
	case 12:
		for (auto Enemy : GameManager->GetDeadEnemies())
		{
			if (Enemy.Contains("spider")) // Event enemies in second dungeon
				EnemyCount++;

			if (EnemyCount == 5)
			{
				DialogueManager->CheckDialogueStartCondition();
			}
		}
		break;
	case 14:
		for (auto Enemy : GameManager->GetDeadEnemies())
		{
			if (Enemy.Contains("monster")) // Final enemies in second dungeon
				EnemyCount++;

			if (EnemyCount == 3) DialogueManager->DisplayDialogueUI();
		}
		break;
	case 16:
		MoveNPCToLocation(Vivi, FVector(105.f, 3176.f, 182.f));
		MoveNPCToLocation(Momo, FVector(-86.f, 3263.f, 177.f));
		MoveNPCToLocation(Luko, FVector(184.f, 3317.f, 182.f));
		MoveNPCToLocation(Vovo, FVector(-140.f, 3370.f, 182.f));
		MoveNPCToLocation(Zizi, FVector(68.f, 3398.f, 184.f));
		//풀어MainPlayerController->DialogueUI->SelectedReply = 2;
		break;
	case 17:
		MoveNPCToLocation(Vivi, FVector(100.f, 1997.f, 182.f));
		MoveNPCToLocation(Momo, FVector(-86.f, 2150.f, 177.f));
		MoveNPCToLocation(Luko, FVector(171.f, 2130.f, 182.f));
		MoveNPCToLocation(Vovo, FVector(-160.f, 2060.f, 182.f));
		MoveNPCToLocation(Zizi, FVector(18.f, 2105.f, 184.f));
		break;
	case 18:
		if (DeadEnemiesNum == 5)
			DialogueManager->DisplayDialogueUI();
		break;
	case 19:
		if (DeadEnemiesNum == 5)//보스맵, 포탈로 이동
		{
			MoveNPCToLocation(Momo, FVector(8.f, -3585.f, 684.f));
			MoveNPCToLocation(Luko, FVector(8.f, -3585.f, 684.f));
			MoveNPCToLocation(Vovo, FVector(8.f, -3585.f, 684.f));
			MoveNPCToLocation(Vivi, FVector(8.f, -3585.f, 684.f));
			MoveNPCToLocation(Zizi, FVector(8.f, -3585.f, 684.f));
			UIManager->SetSystemMessage(13);
		}
		else if (DeadEnemiesNum == 0) //동굴
		{
			DialogueManager->DisplayDialogueUI();
		}
		break;
	case 20: // 돌 쪽으로 이동
		MoveNPCToLocation(Momo, FVector(-4660.f, 118.f, 393.f));
		MoveNPCToLocation(Luko, FVector(-4545.f, -241.f, 401.f));
		MoveNPCToLocation(Vovo, FVector(-4429.f, 103.f, 396.f));
		MoveNPCToLocation(Vivi, FVector(-4355.f, -195.f, 405.f));
		MoveNPCToLocation(Zizi, FVector(-4695.f, -190.f, 394.f));
		Momo->SetActorRotation(FRotator(0.f, 296.f, 0.f));
		Luko->SetActorRotation(FRotator(0.f, 97.f, 0.f));
		Vovo->SetActorRotation(FRotator(0.f, 219.f, 0.f));
		Vivi->SetActorRotation(FRotator(0.f, 145.f, 0.f));
		Zizi->SetActorRotation(FRotator(0.f, 49.f, 0.f));
		break;
	case 22:
		Momo->GetCharacterMovement()->MaxWalkSpeed = 600.f;
		Vivi->GetCharacterMovement()->MaxWalkSpeed = 500.f;
		Zizi->GetCharacterMovement()->MaxWalkSpeed = 500.f;
		Vovo->GetCharacterMovement()->MaxWalkSpeed = 450.f;
		Luko->GetCharacterMovement()->MaxWalkSpeed = 450.f;
		MoveNPCToLocation(Momo, FVector(508.f, 120.f, 100.f));
		MoveNPCToLocation(Luko, FVector(311.f, -78.f, 103.f));
		MoveNPCToLocation(Vovo, FVector(469.f, -22.f, 103.f));
		MoveNPCToLocation(Vivi, FVector(267.f, 65.f, 101.f));
		MoveNPCToLocation(Zizi, FVector(591.f, 28.f, 104.f));
		break;
	case 23:
		MoveNPCToLocation(Zizi, FVector(2560.f, 330.f, 157.f));
		MoveNPCToLocation(Vovo, FVector(2570.f, 335.f, 154.f));
		MoveNPCToLocation(Luko, FVector(1517.f, 335.f, 155.f));
		MoveNPCToLocation(Momo, FVector(1530.f, 330.f, 150.f));
		MoveNPCToLocation(Vivi, FVector(625.f, 330.f, 153.f));
		break;
	}

}

void UNPCManager::AllNpcMoveToPlayer()
{
	for (auto NPC : NPCMap)
	{
		NPC.Value->MoveToPlayer();
	}
}

void UNPCManager::AllNpcLookAtPlayer()
{
	for (auto NPC : NPCMap)
	{
		NPC.Value->SetTargetCharacter(Cast<ACharacter>(GameManager->GetPlayer()));
		NPC.Value->SetInterpToCharacter(true);
	}
}

void UNPCManager::AllNpcDisableLookAt()
{
	for (auto NPC : NPCMap)
	{
		NPC.Value->SetInterpToCharacter(false);
	}
}

void UNPCManager::AllNpcStopFollowPlayer()
{
	for (auto NPC : NPCMap)
	{
		NPC.Value->GetAIController()->StopMovement();
		NPC.Value->GetWorld()->GetTimerManager().ClearTimer(NPC.Value->GetMoveTimer());
	}
}

void UNPCManager::OpenMouth(AYaroCharacter* Character)
{
	if (!Character) return;

	for (auto NPC : NPCMap)
	{
		if (!NPC.Value) continue;
		NPC.Value->SetSpeakingStatus(false);
	}
	Character->SetSpeakingStatus(true);
}

void UNPCManager::CloseAllMouth()
{
	for (auto NPC : NPCMap)
	{
		if (!NPC.Value) continue;
		NPC.Value->SetSpeakingStatus(false);
	}
}