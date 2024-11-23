// Copyright Epic Games, Inc. All Rights Reserved.

#include "YaroCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h" 
#include "Engine/BlueprintGeneratedClass.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "AIController.h"

#include "Yaro/System/MainPlayerController.h"
#include "Yaro/System/GameManager.h"
#include "Yaro/System/DialogueManager.h"
#include "Yaro/System/NPCManager.h"
#include "Yaro/Structs/AttackSkillData.h"
#include "Yaro/Character/Main.h"
#include "Yaro/Character/Enemies/Enemy.h"
#include "Yaro/Character/Enemies/Boss.h"
#include "Yaro/MagicSkill.h"

const float FAR_DISTANCE_FROM_PLAYER = 500.f;
const float CLOSE_DISTANCE_FROM_PLAYER = 80.f;
const float MOVEMENT_THRESHOLD = 50.f; // ���� ��ġ�� ������ ��, ĳ���Ͱ� �̵��ߴٰ� �����ϴ� �ּ� �Ÿ� ����
const int32 MAX_MOVE_FAIL_COUNT = 20;
const FVector TELEPORT_OFFSET = FVector(30.f, 30.f, 0.f);
//////////////////////////////////////////////////////////////////////////
// AYaroCharacter
AYaroCharacter::AYaroCharacter()
{
	AgroSphere = CreateSphereComponent("AgroSphere", 600.f, FVector::ZeroVector);
	CombatSphere = CreateSphereComponent("CombatSphere", 480.f, FVector(250.f, 0.f, 0.f));
	NotAllowSphere = CreateSphereComponent("NotAllowSphere", 100.f, FVector(50.f, 0.f, 0.f));
	
	// positions in first dungeon. only vovo, vivi, zizi
	SetTeamMovePosList();
}

void AYaroCharacter::BeginPlay()
{
	Super::BeginPlay();
	GameManager = Cast<UGameManager>(GetWorld()->GetGameInstance());
	if (GameManager)
	{
		DialogueManager = GameManager->GetDialogueManager();
		NPCManager = GameManager->GetNPCManager();
	}

	AIController = Cast<AAIController>(GetController());

	BindComponentEvents();

	// ĳ���� �޽ÿ� ĸ�� �ݸ����� ī�޶� ������ ������ ����
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
}

void AYaroCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!AnimInstance) AnimInstance = GetMesh()->GetAnimInstance();

	if (!Player)
	{
		ACharacter* p = UGameplayStatics::GetPlayerCharacter(this, 0);
		Player = Cast<AMain>(p);
	}
}

USphereComponent* AYaroCharacter::CreateSphereComponent(FName Name, float Radius, FVector RelativeLocation)
{
	USphereComponent* Sphere = CreateDefaultSubobject<USphereComponent>(Name);
	Sphere->SetupAttachment(GetRootComponent());
	Sphere->InitSphereRadius(Radius);
	Sphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	Sphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Ignore);
	Sphere->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
	Sphere->SetRelativeLocation(RelativeLocation);
	return Sphere;
}

void AYaroCharacter::BindComponentEvents()
{
	AgroSphere->OnComponentBeginOverlap.AddDynamic(this, &AYaroCharacter::AgroSphereOnOverlapBegin);
	AgroSphere->OnComponentEndOverlap.AddDynamic(this, &AYaroCharacter::AgroSphereOnOverlapEnd);

	CombatSphere->OnComponentBeginOverlap.AddDynamic(this, &AYaroCharacter::CombatSphereOnOverlapBegin);
	CombatSphere->OnComponentEndOverlap.AddDynamic(this, &AYaroCharacter::CombatSphereOnOverlapEnd);

	NotAllowSphere->OnComponentBeginOverlap.AddDynamic(this, &AYaroCharacter::NotAllowSphereOnOverlapBegin);
	NotAllowSphere->OnComponentEndOverlap.AddDynamic(this, &AYaroCharacter::NotAllowSphereOnOverlapEnd);
}

void AYaroCharacter::SetTeamMovePosList()
{
	TeamMovePosList.Add(FVector(2517.f, 5585.f, 3351.f));
	TeamMovePosList.Add(FVector(2345.f, 4223.f, 2833.f));
	TeamMovePosList.Add(FVector(2080.f, 283.f, 2838.f));
	TeamMovePosList.Add(FVector(1550.f, -1761.f, 2843.f));
	TeamMovePosList.Add(FVector(1026.f, -1791.f, 2576.f));
}

void AYaroCharacter::MoveTo(ACharacter* GoalActor, float AcceptanceRadius)
{
	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalActor(GoalActor);
	MoveRequest.SetAcceptanceRadius(AcceptanceRadius);
	FNavPathSharedPtr NavPath;
	AIController->MoveTo(MoveRequest, &NavPath);
}

void AYaroCharacter::MoveToPlayer()
{
	if (Player == nullptr) return;

	if (CombatTarget == nullptr && !bAttacking && !bOverlappingCombatSphere && AgroTargets.Num() == 0) // ���� ���� �ƴ� ��
	{
		if (DialogueManager->IsDialogueUIVisible() || DialogueManager->GetDialogueNum() == 9) // ��ȭ �߿��� ��κ� �̵�X
		{
			GetWorldTimerManager().SetTimer(PlayerFollowTimer, this, &AYaroCharacter::MoveToPlayer, 0.5f);
			return;
		}

		for (auto NPC : NPCManager->GetNPCMap())
		{
			// �ٸ� npc�� �ν� ������ ���Ͱ� ������ �����ַ� ��, �� ù��° ������ ����
			if (NPC.Value->AgroTargets.Num() != 0
				&& NPC.Value->AgroTargets[0]->GetEnemyMovementStatus() != EEnemyMovementStatus::EMS_Dead
				&& !UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("first")) 
			{
				// �޸��� ���ǵ�� ����
				GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
				MoveToTarget(NPC.Value->AgroTargets[0]);

				GetWorldTimerManager().SetTimer(PlayerFollowTimer, this, &AYaroCharacter::MoveToPlayer, 1.f);
				return;
			}
		}

		FVector CurrentPosition = GetActorLocation();
        float Distance = GetDistanceTo(Player);
        if (Distance >= FAR_DISTANCE_FROM_PLAYER) //���� �Ÿ� �̻� �������ִٸ� �ӵ� ���� �޸���
			GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
		else //�����ٸ� �ӵ� ���� �ȱ�
		{
			GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
			MoveFailCounter = 0;
		}

		MoveTo(Player, CLOSE_DISTANCE_FROM_PLAYER);

		if (FVector::Dist(LastPosition, CurrentPosition) < MOVEMENT_THRESHOLD)
			MoveFailCounter++;
		else
			MoveFailCounter = 0;

		LastPosition = CurrentPosition;

		if (MoveFailCounter >= MAX_MOVE_FAIL_COUNT) TeleportToPlayer();

	}

	GetWorldTimerManager().SetTimer(PlayerFollowTimer, this, &AYaroCharacter::MoveToPlayer, 0.5f);
}

void AYaroCharacter::TeleportToPlayer()
{
	if (Player->GetMovementStatus() == EMovementStatus::EMS_Dead) return;

	SetActorLocation(Player->GetActorLocation() + TELEPORT_OFFSET);
}

void AYaroCharacter::MoveToTarget(AEnemy* Target) // �� ����
{
	if (AIController)
	{
		MoveTo(Target, 300.f);
	}
}

void AYaroCharacter::AgroSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		AEnemy* Enemy = Cast<AEnemy>(OtherActor);
		if (Enemy)
		{
			if (AgroTargets.Contains(Enemy)) return;

			AgroTargets.Add(Enemy); // �νĵ� Ÿ�� �迭�� �߰�

			if (!CombatTarget) // ���� ���� Ÿ���� ���ٸ�
			{
				MoveToTarget(Enemy); // ���� �νĵ� �� ����
			}
		}
	}
}

void AYaroCharacter::AgroSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		AEnemy* Enemy = Cast<AEnemy>(OtherActor);
		if (Enemy)
		{
			if (!UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("boss") || (UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("boss") && Enemy->GetEnemyMovementStatus() == EEnemyMovementStatus::EMS_Dead))
			{
				if (AgroTargets.Contains(Enemy))
					AgroTargets.Remove(Enemy); //Ÿ���� ���� �� �迭���� ����
			}

			if (!CombatTarget || Enemy == CombatTarget)
			{
				CombatTarget = nullptr;

				if (AgroTargets.Num() == 0) // ���� ���� ���� �ƹ��� ����
				{
					bOverlappingCombatSphere = false; // Ȯ�� �ʿ�

					for (auto NPC : NPCManager->GetNPCMap())
					{
						// �ٸ� npc�� �ν� ������ ���Ͱ� ������ �����ַ� ��, ù��° ���� ����
						if (NPC.Value->AgroTargets.Num() != 0
							&& !UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("first")) 
						{
							// �� ���Ͱ� ���� ���¸� ����
							if (NPC.Value->AgroTargets[0]->GetEnemyMovementStatus() == EEnemyMovementStatus::EMS_Dead) return;

							MoveToTarget(NPC.Value->AgroTargets[0]);
						}
					}
				}
				else // �ν� ������ �ٸ� ���Ͱ� ��������
				{
					bOverlappingCombatSphere = true;
					CombatTarget = AgroTargets[0]; // Ÿ�� ���� �� ����
					Attack();
				}
			}
		}
	}
}

void AYaroCharacter::CombatSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor)
    {
        AEnemy* Enemy = Cast<AEnemy>(OtherActor);
        if (Enemy)
        {
			if (CombatTargets.Contains(Enemy)) return;

			CombatTargets.Add(Enemy); // ���� Ÿ�� �迭�� �߰�

			if (CombatTarget) return; // �̹� ���� ���� Ÿ���� �ִٸ� ����

			bOverlappingCombatSphere = true;
			CombatTarget = Enemy;
			AIController->StopMovement(); // �̵��� ���߰� ����
			Attack();          
        }
    }
}

void AYaroCharacter::CombatSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor)
    {
        AEnemy* Enemy = Cast<AEnemy>(OtherActor);
        if (Enemy)
        {
			if (CombatTargets.Contains(Enemy))
				CombatTargets.Remove(Enemy); //Ÿ���� ���� �� �迭���� ����

			if (Enemy == CombatTarget) // ���� ���� Ÿ���� ���� �������� ����
			{
				CombatTarget = nullptr;			

				if (CombatTargets.Num() != 0) // ���� ������ �ٸ� ���� ����
				{
					CombatTarget = CombatTargets[0];
					Attack();
				}
				else // ���� ���� ������ ���� ����
				{
					bOverlappingCombatSphere = false;

					// �ν� ������ ����ִ� ���Ͱ� ���� ��
					if (AgroTargets.Num() != 0 
						&& AgroTargets[0]->GetEnemyMovementStatus() != EEnemyMovementStatus::EMS_Dead)
					{
						MoveToTarget(AgroTargets[0]);
					}
				}
			}
        }
    }
}

void AYaroCharacter::Attack()
{
	if ((!bAttacking) && (CombatTarget) && (CombatTarget->GetEnemyMovementStatus() != EEnemyMovementStatus::EMS_Dead))
	{
		if (NPCType == ENPCType::Vovo && bIsHealTime)
		{
			GetWorldTimerManager().SetTimer(AttackTimer, this, &AYaroCharacter::Attack, 2.f);
			return;
		}

		int32 MinSkillNum = 0;
		int32 MaxSkillNum = 0;
		if (bCanCastStorm) // npc can cast storm magic
			MinSkillNum = 1;
		else // npc can't cast storm magic
			MinSkillNum = 2;

		if (DialogueManager->GetDialogueNum() <= 5)
			MaxSkillNum = 3;
		else if (DialogueManager->GetDialogueNum() <= 14)
			MaxSkillNum = 4;
		else
			MaxSkillNum = 5;

		SetSkillNum(FMath::RandRange(MinSkillNum, MaxSkillNum));
		
        if (GetSkillNum() == 1) // ���� ������ ����
        {
            bCanCastStorm = false;
            FTimerHandle StormTimer;
            GetWorldTimerManager().SetTimer(StormTimer, this, &AYaroCharacter::EnableStormCasting, 8.f, false);  // 8�� �� �ٽ� ���� ��� ����
        }

		ToSpawn = AttackSkillData->FindRow<FAttackSkillData>("Attack", "")->Skills[GetSkillNum() - 1];

		bAttacking = true;
		SetInterpToEnemy(true); // �� �������� ȸ�� ����

		if (AnimInstance && CombatMontage)
		{
			AnimInstance->Montage_Play(CombatMontage);
			AnimInstance->Montage_JumpToSection(FName("Attack"), CombatMontage);
			Spawn();
		}
	}
	else
	{
		AttackEnd();
	}
}

void AYaroCharacter::AttackEnd()
{
	bAttacking = false;
	SetInterpToEnemy(false);

	if (bOverlappingCombatSphere) 
	{
		// ���� ���� Ÿ���� ����
		if (CombatTarget && CombatTarget->GetEnemyMovementStatus() == EEnemyMovementStatus::EMS_Dead)
		{
			if(CombatTargets.Contains(CombatTarget))
				CombatTargets.Remove(CombatTarget);

			if (AgroTargets.Contains(CombatTarget))
				AgroTargets.Remove(CombatTarget);

			CombatTarget = nullptr;

			if (CombatTargets.Num() == 0) // ���� ������ ���Ͱ� ����
			{
				bOverlappingCombatSphere = false;
				
				if (AgroTargets.Num() != 0) // �ν� ������ ���Ͱ� ����
				{
					MoveToTarget(AgroTargets[0]);
				}
				else // �ν� ������ ���� ����
				{
					for (auto NPC : NPCManager->GetNPCMap())
					{
						// �ٸ� npc�� �ν� ������ ���Ͱ� ������ �����ַ� ��, ù��° ���� ����
						if (NPC.Value->AgroTargets.Num() != 0
							&& !UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("first")) 
						{
							MoveToTarget(NPC.Value->AgroTargets[0]);
						}
					}
				}
			}
			else // ���� ������ ���Ͱ� ������
			{
				// ���� Ÿ�� ���� ����
				CombatTarget = CombatTargets[0];
			}
		}

		if (CombatTarget)
			GetWorldTimerManager().SetTimer(AttackTimer, this, &AYaroCharacter::Attack, AttackDelay);
	}
	else // ���� ������ �ƹ��� ����
	{
		if (AgroTargets.Num() != 0) // �ν� ������ ���� ����
		{
			for (int i = 0; i < AgroTargets.Num(); i++)
			{
				// �ν� ������ �ִ� ���Ͱ� ����ִٸ�
				if (AgroTargets[i]->GetEnemyMovementStatus() != EEnemyMovementStatus::EMS_Dead)
				{
					if (GetDistanceTo(AgroTargets[i]) >= 450.f) // �Ÿ��� �ָ� ����
					{
						MoveToTarget(AgroTargets[i]);
						return;
					}
					else // �Ÿ��� ���� ������ ���� Ÿ������ �ٷ� ���� �� ����
					{
						CombatTarget = AgroTargets[i];
						GetWorldTimerManager().SetTimer(AttackTimer, this, &AYaroCharacter::Attack, AttackDelay);
						return;
					}
				}
			}
		}
	}
}

void AYaroCharacter::Spawn()
{
	if (ToSpawn)
	{
		GetWorld()->GetTimerManager().SetTimer(MagicSpawnTimer, FTimerDelegate::CreateLambda([&]()
		{
			UWorld* world = GetWorld();
			if (world)
			{
				FActorSpawnParameters spawnParams;
				spawnParams.Owner = this;
				FRotator rotator = this->GetActorRotation();

				// �⺻ ���� ��ġ�� ���� ȭ��ǥ ��ġ(ĳ���� ��)
				FVector spawnLocation = AttackArrow->GetComponentTransform().GetLocation();

				if (CombatTarget)
				{
					// ���/����/������ ��� 3�� ��ų�� �� ��ġ���� ����
					if (GetSkillNum() == 3 && 
						(NPCType == ENPCType::Momo) || NPCType == ENPCType::Luko || NPCType == ENPCType::Vovo)
					{
						spawnLocation = CombatTarget->GetActorLocation();
					}
						
					//��� ���� 4�� ��ų �� ��ġ���� ����
					if (GetSkillNum() == 4 && NPCType != ENPCType::Momo)
					{
						spawnLocation = CombatTarget->GetActorLocation();
					}			
				}		

				MagicAttack = world->SpawnActor<AMagicSkill>(ToSpawn, spawnLocation, rotator, spawnParams);

				if (MagicAttack.IsValid())
				{
					MagicAttack->SetInstigator(AIController);

					if(CombatTarget)
						MagicAttack->SetTarget(CombatTarget);
				}
			}
		}), 0.6f, false); // 0.6�� �� ����, �ݺ�X
	}
}

void AYaroCharacter::MoveToTeamPos() // Vivi, Vovo, Zizi�� ����
{	
	if (AIController)
	{
		AIController->MoveToLocation(TeamMovePosList[TeamMoveIndex]);
	}

	GetWorld()->GetTimerManager().SetTimer(TeamMoveTimer, FTimerDelegate::CreateLambda([&]() {

		if (!CombatTarget && !bOverlappingCombatSphere) // ���� ���� �ƴ� ��
		{
            float distance = (GetActorLocation() - TeamMovePosList[TeamMoveIndex]).Size();

			if (TeamMoveIndex <= 3)
			{
				if (AIController && distance <= 70.f) // ��ǥ ������ ����� ��
				{
					TeamMoveIndex++;
				}
				AIController->MoveToLocation(TeamMovePosList[TeamMoveIndex]); // ��ǥ �������� ����ؼ� �̵�
			}

			if (TeamMoveIndex == 4 && distance <= 70.f) // �������� �̵� ��
			{
				// ��ǥ ���� �̵��� ��� ������, ��� Ȥ�� �÷��̾ �񷽰� ���� ���̸� �� ������ �̵�
				if (NPCManager->GetNPC("Momo")->CombatTarget && NPCManager->GetNPC("Momo")->CombatTarget->GetName().Contains("Golem"))
				{
					GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
					AIController->MoveToActor(NPCManager->GetNPC("Momo")->CombatTarget);
				}
                if (Player->GetCombatTarget() && Player->GetCombatTarget()->GetName().Contains("Golem"))
                {
					GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
                    AIController->MoveToActor(Player->GetCombatTarget());
                }
			}
		}
	}), 0.5f, true);
}

float AYaroCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	// ���� �������� ���� ����
	if (!UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("boss")) 
		return DamageAmount;
	
	MagicAttack = Cast<AMagicSkill>(DamageCauser);
	if (MagicAttack == nullptr) return DamageAmount;
	
	// �ν� ������ �ƹ��� ���� ���¿��� ������ ����
	if ((MagicAttack->GetCaster() == ECasterType::Boss || MagicAttack->GetCaster() == ECasterType::Enemy) && AgroTargets.Num() == 0)
	{
		AEnemy* Enemy = Cast<AEnemy>(MagicAttack->GetInstigator()->GetPawn());
		ClearPlayerFollowTimer();
		AIController->StopMovement();
		MoveToTarget(Enemy); // �������� �̵�
	}
	
	return DamageAmount;
}

void AYaroCharacter::NotAllowSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		AEnemy* Enemy = Cast<AEnemy>(OtherActor);
		if (Enemy)
		{
			if (DangerousTargets.Contains(Enemy)) return;
			
			DangerousTargets.Add(Enemy);

			// �̹� Ÿ�̸� ���� ���̸� ����
			if (GetWorldTimerManager().IsTimerActive(SafeDistanceTimer)) return;

			// 2�� �� ���� ���� Ȯ���ϱ� ���� �̵�
			GetWorldTimerManager().SetTimer(SafeDistanceTimer, this, &AYaroCharacter::MoveToSafeLocation, 2.f);
		}
	}
}

void AYaroCharacter::NotAllowSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		AEnemy* Enemy = Cast<AEnemy>(OtherActor);
		if (Enemy)
		{
			if (DangerousTargets.Contains(Enemy))
				DangerousTargets.Remove(Enemy);
		}
	}
}

void AYaroCharacter::MoveToSafeLocation() // ������ ��ġ�� �̵�, ���Ϳ��� ���� �Ÿ� ����
{
	if (DangerousTargets.Num() != 0) 
	{
		GetWorldTimerManager().SetTimer(SafeDistanceTimer, this, &AYaroCharacter::MoveToSafeLocation, 3.f);
	}
	else // ���Ϳ��� ���� �Ÿ� Ȯ����
	{
		GetWorldTimerManager().ClearTimer(SafeDistanceTimer);
		SafeDistanceTimer.Invalidate();
		return;
	}

	float Value = FMath::RandRange(-200.f, 200.f);
	// ���� ��ġ���� ������ �������� ���� �̵�
	FVector SafeLocation = GetActorLocation() + FVector(Value, Value, 0.f);
	AIController->MoveToLocation(SafeLocation);
}

void AYaroCharacter::ClearTeamMoveTimer()
{
	GetWorldTimerManager().ClearTimer(TeamMoveTimer);
	TeamMoveTimer.Invalidate();
}

void AYaroCharacter::ClearPlayerFollowTimer()
{
	GetWorldTimerManager().ClearTimer(PlayerFollowTimer);
	PlayerFollowTimer.Invalidate();
}

void AYaroCharacter::ClearAllTimer()
{
	ClearTeamMoveTimer();
	ClearPlayerFollowTimer();
	GetWorldTimerManager().ClearTimer(MagicSpawnTimer);
	MagicSpawnTimer.Invalidate();
}