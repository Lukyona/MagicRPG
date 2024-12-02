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
#include "Yaro/Character/Enemies/Golem.h"
#include "Yaro/MagicSkill.h"

const float FAR_DISTANCE_FROM_PLAYER = 500.f;
const float CLOSE_DISTANCE_FROM_PLAYER = 80.f;
const float MOVEMENT_THRESHOLD = 50.f; // 이전 위치와 비교했을 때, 캐릭터가 이동했다고 간주하는 최소 거리 차이
const int32 MAX_MOVE_FAIL_COUNT = 20;
const FVector TELEPORT_OFFSET = FVector(30.f, 30.f, 0.f);
//////////////////////////////////////////////////////////////////////////
// AYaroCharacter
AYaroCharacter::AYaroCharacter()
{
	AgroSphere = CreateSphereComponent("AgroSphere", 600.f, FVector::ZeroVector);
	CombatSphere = CreateSphereComponent("CombatSphere", 480.f, FVector(250.f, 0.f, 0.f));
	NotAllowSphere = CreateSphereComponent("NotAllowSphere", 150.f, FVector(50.f, 0.f, 0.f));
	
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

	// 캐릭터 메시와 캡슐 콜리전을 카메라에 영향이 없도록 만듬
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
}

void AYaroCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!AnimInstance) 
	{
		AnimInstance = GetMesh()->GetAnimInstance();
	}

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

void AYaroCharacter::SetMovementSpeed(bool bEnableRunning)
{
	if (bEnableRunning)
	{
		GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
	}
	else 
	{
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	}
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
	if (Player == nullptr) 
	{
		return;
	}

	if (CombatTarget == nullptr && !bAttacking && !bOverlappingCombatSphere && AgroTargets.Num() == 0) // 전투 중이 아닐 때
	{
		if (DialogueManager->IsDialogueUIVisible() || DialogueManager->GetDialogueState() == EDialogueState::InteractWithYellowStone) // 대화 중에는 대부분 이동X
		{
			GetWorldTimerManager().SetTimer(PlayerFollowTimer, this, &AYaroCharacter::MoveToPlayer, 0.5f);
			return;
		}

		for (auto NPC : NPCManager->GetNPCMap())
		{
			// 다른 npc의 인식 범위에 몬스터가 있으면 도와주러 감, 단 첫번째 던전은 제외
			if (NPC.Value->AgroTargets.Num() != 0
				&& NPC.Value->AgroTargets[0] &&
				NPC.Value->AgroTargets[0]->GetEnemyMovementStatus() != EEnemyMovementStatus::EMS_Dead
				&& !UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("first")) 
			{
				// 달리기 스피드로 변경
				GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
				MoveToTarget(NPC.Value->AgroTargets[0]);

				GetWorldTimerManager().SetTimer(PlayerFollowTimer, this, &AYaroCharacter::MoveToPlayer, 1.f);
				return;
			}
		}

		FVector CurrentPosition = GetActorLocation();
        float Distance = GetDistanceTo(Player);
        if (Distance >= FAR_DISTANCE_FROM_PLAYER) //일정 거리 이상 떨어져있다면 속도 높여 달리기
		{
			GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
		}
		else //가깝다면 속도 낮춰 걷기
		{
			GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
			MoveFailCounter = 0;
		}

		MoveTo(Player, CLOSE_DISTANCE_FROM_PLAYER);

		if (FVector::Dist(LastPosition, CurrentPosition) < MOVEMENT_THRESHOLD)
		{
			++MoveFailCounter;
		}
		else
		{
			MoveFailCounter = 0;
		}

		LastPosition = CurrentPosition;

		if (MoveFailCounter >= MAX_MOVE_FAIL_COUNT) TeleportToPlayer();

	}
	GetWorldTimerManager().SetTimer(PlayerFollowTimer, this, &AYaroCharacter::MoveToPlayer, 0.5f);
}

void AYaroCharacter::TeleportToPlayer()
{
	if (Player->GetMovementStatus() == EMovementStatus::EMS_Dead)
	{
		return;
	}

	SetActorLocation(Player->GetActorLocation() + TELEPORT_OFFSET);
}

void AYaroCharacter::MoveToTarget(AEnemy* Target) // 적 추적
{
	if (AIController)
	{
		MoveTo(Target, 200.f);
	}
}

void AYaroCharacter::AgroSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		AEnemy* Enemy = Cast<AEnemy>(OtherActor);
		if (Enemy)
		{
			if (AgroTargets.Contains(Enemy))
			{
				return;
			}

			AgroTargets.Add(Enemy); // 인식된 타겟 배열에 추가

			if (!CombatTarget) // 현재 전투 타겟이 없다면
			{
				MoveToTarget(Enemy); // 지금 인식된 적 추적
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
				{
					AgroTargets.Remove(Enemy); 
				}
			}

			if (!CombatTarget || Enemy == CombatTarget)
			{
				CombatTarget = nullptr;

				if (AgroTargets.Num() == 0) // 범위 내에 적이 아무도 없음
				{
					bOverlappingCombatSphere = false; // 확인 필요

					for (auto NPC : NPCManager->GetNPCMap())
					{
						// 다른 npc의 인식 범위에 몬스터가 있으면 도와주러 감, 첫번째 던전 제외
						if (NPC.Value->AgroTargets.Num() != 0
							&& !UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("first")) 
						{
							// 그 몬스터가 죽은 상태면 리턴
							if (NPC.Value->AgroTargets[0]->GetEnemyMovementStatus() == EEnemyMovementStatus::EMS_Dead) 
							{
								return;
							}

							MoveToTarget(NPC.Value->AgroTargets[0]);
						}
					}
				}
				else // 인식 범위에 다른 몬스터가 남아있음
				{
					bOverlappingCombatSphere = true;
					CombatTarget = AgroTargets[0]; // 타겟 설정 후 공격
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
			if (CombatTargets.Contains(Enemy)) 
			{
				return;
			}

			CombatTargets.Add(Enemy); // 전투 타겟 배열에 추가

			if (CombatTarget) 
			{
				return; // 이미 전투 중인 타겟이 있다면 리턴
			}

			bOverlappingCombatSphere = true;
			CombatTarget = Enemy;
			AIController->StopMovement(); // 이동을 멈추고 공격
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
			{
				CombatTargets.Remove(Enemy); 
			}

			if (Enemy == CombatTarget) // 현재 전투 타겟이 전투 범위에서 나감
			{
				CombatTarget = nullptr;			

				if (CombatTargets.Num() != 0) // 전투 범위에 다른 몬스터 있음
				{
					CombatTarget = CombatTargets[0];
					Attack();
				}
				else // 현재 전투 범위에 몬스터 없음
				{
					bOverlappingCombatSphere = false;

					// 인식 범위에 살아있는 몬스터가 있을 때
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

		if (DialogueManager->GetDialogueState() < EDialogueState::SecondDungeonStarted) // First Dungeon
			MaxSkillNum = 3;
		else if (DialogueManager->GetDialogueState() <= EDialogueState::CombatWithLittleMonsters) // Second Dungeon
			MaxSkillNum = 4;
		else
			MaxSkillNum = 5;

		SetSkillNum(FMath::RandRange(MinSkillNum, MaxSkillNum));
		
        if (GetSkillNum() == 1) // 현재 공격이 스톰
        {
            bCanCastStorm = false;
            FTimerHandle StormTimer;
            GetWorldTimerManager().SetTimer(StormTimer, this, &AYaroCharacter::EnableStormCasting, 8.f, false);  // 8초 뒤 다시 스톰 사용 가능
        }

		ToSpawn = AttackSkillData->FindRow<FAttackSkillData>("Attack", "")->Skills[GetSkillNum() - 1];

		bAttacking = true;
		SetInterpToEnemy(true); // 적 방향으로 회전 가능

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
		// 현재 전투 타겟이 죽음
		if (CombatTarget && CombatTarget->GetEnemyMovementStatus() == EEnemyMovementStatus::EMS_Dead)
		{
			if(CombatTargets.Contains(CombatTarget))
			{
				CombatTargets.Remove(CombatTarget);
			}

			if (AgroTargets.Contains(CombatTarget))
			{
				AgroTargets.Remove(CombatTarget);
			}

			CombatTarget = nullptr;

			if (CombatTargets.Num() == 0) // 전투 범위에 몬스터가 없음
			{
				bOverlappingCombatSphere = false;
				
				if (AgroTargets.Num() != 0) // 인식 범위에 몬스터가 있음
				{
					MoveToTarget(AgroTargets[0]);
				}
				else // 인식 범위에 몬스터 없음
				{
					for (auto NPC : NPCManager->GetNPCMap())
					{
						// 다른 npc의 인식 범위에 몬스터가 있으면 도와주러 감, 첫번째 던전 제외
						if (NPC.Value->AgroTargets.Num() != 0
							&& !UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("first")) 
						{
							MoveToTarget(NPC.Value->AgroTargets[0]);
						}
					}
				}
			}
			else // 전투 범위에 몬스터가 있으면
			{
				// 전투 타겟 새로 설정
				CombatTarget = CombatTargets[0];
			}
		}

		if (CombatTarget)
		{
			GetWorldTimerManager().SetTimer(AttackTimer, this, &AYaroCharacter::Attack, AttackDelay);
		}
	}
	else // 전투 범위에 아무도 없음
	{
		if (AgroTargets.Num() != 0) // 인식 범위에 몬스터 존재
		{
			for (int i = 0; i < AgroTargets.Num(); i++)
			{
				// 인식 범위에 있는 몬스터가 살아있다면
				if (AgroTargets[i]->GetEnemyMovementStatus() != EEnemyMovementStatus::EMS_Dead)
				{
					if (GetDistanceTo(AgroTargets[i]) >= 450.f) // 거리가 멀면 추적
					{
						MoveToTarget(AgroTargets[i]);
						return;
					}
					else // 거리가 멀지 않으면 전투 타겟으로 바로 설정 후 공격
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

				// 기본 스폰 위치는 공격 화살표 위치(캐릭터 앞)
				FVector spawnLocation = AttackArrow->GetComponentTransform().GetLocation();

				if (CombatTarget)
				{
					// 모모/루코/보보의 경우 3번 스킬은 적 위치에서 스폰
					if (GetSkillNum() == 3 && 
						(NPCType == ENPCType::Momo) || NPCType == ENPCType::Luko || NPCType == ENPCType::Vovo)
					{
						spawnLocation = CombatTarget->GetActorLocation();
					}
						
					//모모 제외 4번 스킬 적 위치에서 스폰
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
					{
						MagicAttack->SetTarget(CombatTarget);
					}
				}
			}
		}), 0.6f, false); // 0.6초 뒤 실행, 반복X
	}
}

void AYaroCharacter::MoveToTeamPos() // Vivi, Vovo, Zizi만 실행
{	
	if (AIController)
	{
		if (TeamMoveIndex == 5)
		{
			CheckGolem();
			return;
		}
		else
		{
			AIController->MoveToLocation(TeamMovePosList[TeamMoveIndex]);
		}
	}

	GetWorld()->GetTimerManager().SetTimer(TeamMoveTimer, FTimerDelegate::CreateLambda([&]() {

		if (!CombatTarget && !bOverlappingCombatSphere && TeamMoveIndex <= 4) // 전투 중이 아닐 때
		{
            float distance = (GetActorLocation() - TeamMovePosList[TeamMoveIndex]).Size();

			if (TeamMoveIndex <= 3)
			{
				if (AIController && distance <= 70.f) // 목표 지점과 가까울 때
				{
					++TeamMoveIndex;
				}
				AIController->MoveToLocation(TeamMovePosList[TeamMoveIndex]); // 목표 지점으로 계속해서 이동
			}

			if (TeamMoveIndex == 4 && distance <= 70.f) // 골렘쪽으로 이동 중
			{
				++TeamMoveIndex;
				// 목표 지점 이동을 모두 끝내면 골렘에게로 이동
				CheckGolem();
				GetWorldTimerManager().ClearTimer(TeamMoveTimer);
			}
		}
	}), 0.5f, true);
}

float AYaroCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	// 보스 스테이지 제외 리턴
	if (!UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("boss")) 
	{
		return DamageAmount;
	}
	
	MagicAttack = Cast<AMagicSkill>(DamageCauser);
	if (MagicAttack == nullptr) 
	{
		return DamageAmount;
	}
	
	// 인식 범위에 아무도 없는 상태에서 공격을 받음
	if ((MagicAttack->GetCaster() == ECasterType::Boss || MagicAttack->GetCaster() == ECasterType::Enemy) && AgroTargets.Num() == 0)
	{
		AEnemy* Enemy = Cast<AEnemy>(MagicAttack->GetInstigator()->GetPawn());
		ClearPlayerFollowTimer();
		AIController->StopMovement();
		MoveToTarget(Enemy); // 보스에게 이동
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
			if (DangerousTargets.Contains(Enemy)) 
			{
				return;
			}
			
			DangerousTargets.Add(Enemy);

			// 이미 타이머 실행 중이면 리턴
			if (GetWorldTimerManager().IsTimerActive(SafeDistanceTimer)) 
			{
				return;
			}

			// 2초 뒤 안전 범위 확보하기 위해 이동
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
			{
				DangerousTargets.Remove(Enemy);
			}
		}
	}
}

void AYaroCharacter::MoveToSafeLocation() // 안전한 위치로 이동, 몬스터와의 안전 거리 유지
{
	if (DangerousTargets.Num() != 0) 
	{
		GetWorldTimerManager().SetTimer(SafeDistanceTimer, this, &AYaroCharacter::MoveToSafeLocation, 3.f);
	}
	else // 몬스터와의 안전 거리 확보됨
	{
		GetWorldTimerManager().ClearTimer(SafeDistanceTimer);
		SafeDistanceTimer.Invalidate();
		return;
	}

	float Value = FMath::RandRange(-200.f, 200.f);
	// 현재 위치에서 랜덤한 방향으로 조금 이동
	FVector SafeLocation = GetActorLocation() + FVector(Value, Value, 0.f);
	AIController->MoveToLocation(SafeLocation);
}

void AYaroCharacter::CheckGolem()
{
	AEnemy* Golem = Cast<AEnemy>(UGameplayStatics::GetActorOfClass(GetWorld(), AGolem::StaticClass()));
	if (!Golem)
	{
		return;
	}

	TArray<AActor*> OverlappingActors;
	CombatSphere->GetOverlappingActors(OverlappingActors, AGolem::StaticClass());
	if (OverlappingActors.Contains(Golem))
	{
		CombatTargets.Add(Golem); // 전투 타겟 배열에 추가
		AgroTargets.Add(Golem); // 전투 타겟 배열에 추가

		if (CombatTarget) 
		{
			return; // 이미 전투 중인 타겟이 있다면 리턴
		}
		
		bOverlappingCombatSphere = true;
		CombatTarget = Golem;
		AIController->StopMovement(); // 이동을 멈추고 공격
		Attack();
		return;
	}

	MoveToTarget(Golem);
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
