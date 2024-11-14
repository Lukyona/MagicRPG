// Copyright Epic Games, Inc. All Rights Reserved.

#include "YaroCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h" //GetPlayerCharacter
#include "AIController.h"
#include "Yaro/Character/Main.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Yaro/Character/Enemies/Enemy.h"
#include "Components/ArrowComponent.h"
#include "Yaro/MagicSkill.h"
#include "Yaro/Structs/AttackSkillData.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Components/CapsuleComponent.h"
#include "Yaro/System/MainPlayerController.h"
#include "Yaro/System/GameManager.h"
#include "Yaro/System/DialogueManager.h"
#include "Yaro/System/NPCManager.h"

//////////////////////////////////////////////////////////////////////////
// AYaroCharacter
AYaroCharacter::AYaroCharacter()
{
	SetAgroSphere();
	SetCombatSphere();
	SetNotAllowSphere();
	
	// positions in first dungeon. only vovo, vivi, zizi
	SetPosList();
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

	AgroSphere->OnComponentBeginOverlap.AddDynamic(this, &AYaroCharacter::AgroSphereOnOverlapBegin);
	AgroSphere->OnComponentEndOverlap.AddDynamic(this, &AYaroCharacter::AgroSphereOnOverlapEnd);

    CombatSphere->OnComponentBeginOverlap.AddDynamic(this, &AYaroCharacter::CombatSphereOnOverlapBegin);
    CombatSphere->OnComponentEndOverlap.AddDynamic(this, &AYaroCharacter::CombatSphereOnOverlapEnd);

	NotAllowSphere->OnComponentBeginOverlap.AddDynamic(this, &AYaroCharacter::NotAllowSphereOnOverlapBegin);
	NotAllowSphere->OnComponentEndOverlap.AddDynamic(this, &AYaroCharacter::NotAllowSphereOnOverlapEnd);

	// 캐릭터 메시와 캡슐 콜리전을 카메라에 영향이 없도록 만듬
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

void AYaroCharacter::MoveToPlayer()
{
	if (Player == nullptr) return;

	if (CombatTarget == nullptr && !bAttacking && !bOverlappingCombatSphere && AgroTargets.Num() == 0) // 전투 중이 아닐 때
	{
		if (DialogueManager->IsDialogueUIVisible()) // 대화 중에는 대부분 이동X
		{
			if (DialogueManager->GetDialogueNum() != 6) // 특정 시점 제외하고 플레이어를 따라가기
			{
				GetWorldTimerManager().SetTimer(MoveTimer, this, &AYaroCharacter::MoveToPlayer, 0.5f);
				return;
			}
		}

		for (auto NPC : NPCManager->GetNPCMap())
		{
			// 다른 npc의 인식 범위에 몬스터가 있으면 도와주러 감, 단 첫번째 던전은 제외
			if (NPC.Value->AgroTargets.Num() != 0
				&& NPC.Value->AgroTargets[0]->GetEnemyMovementStatus() != EEnemyMovementStatus::EMS_Dead
				&& !UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("first")) 
			{
				// 달리기 스피드로 변경
				GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
				MoveToTarget(NPC.Value->AgroTargets[0]);

				//if(!GetWorldTimerManager().IsTimerActive(MoveTimer))
					GetWorldTimerManager().SetTimer(MoveTimer, this, &AYaroCharacter::MoveToPlayer, 1.f);

				return;
			}
		}

		// 플레이어와의 거리
        float distance = GetDistanceTo(Player);

        if (distance >= 500.f) //일정 거리 이상 떨어져있다면 속도 높여 달리기
        {
			GetCharacterMovement()->MaxWalkSpeed = RunSpeed;

			// 첫번째 던전에서는 npc들이 지형에 끼거나 멈춰서 진행에 방해되지 않도록 텔레포트 기능 실행
			if (UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("first")
				&& !GetWorldTimerManager().IsTimerActive(TeleportTimer))
				Teleport();
        }
        else //가깝다면 속도 낮춰 걷기
        {
			GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
			TeleportCount = 0;
        }

        FAIMoveRequest MoveRequest;
        MoveRequest.SetGoalActor(Player);
        MoveRequest.SetAcceptanceRadius(80.f);

        FNavPathSharedPtr NavPath;
        AIController->MoveTo(MoveRequest, &NavPath);
	}
	else // 전투 중
	{
		if(UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("first"))
			GetWorldTimerManager().ClearTimer(TeleportTimer); 
	}

	GetWorldTimerManager().SetTimer(MoveTimer, this, &AYaroCharacter::MoveToPlayer, 0.5f);
}

void AYaroCharacter::Teleport()
{
	if (Player->GetMovementStatus() == EMovementStatus::EMS_Dead)
	{
		TeleportCount = 0;
		return;
	}

	TeleportCount += 1;

	// 시간이 오래 경과하면 플레이어 근처로 순간이동
	if (TeleportCount >= 25) SetActorLocation(Player->GetActorLocation() + FVector(15.f, 25.f, 0.f));


	float distance = (GetActorLocation() - Player->GetActorLocation()).Size();

	if (distance <= 400.f) // 플레이어와 거리 가까움
	{
		if (AIController)
		{
			TeleportCount = 0;
			GetWorldTimerManager().ClearTimer(TeleportTimer);
		}
	}
	else // 플레이어와 거리가 멀면 계속 텔레포트 카운트 증감
	{
		GetWorldTimerManager().SetTimer(TeleportTimer, this, &AYaroCharacter::Teleport, 0.5f);
	}
}

void AYaroCharacter::MoveToTarget(AEnemy* Target) // 적 추적
{
	if (AIController)
	{
		FAIMoveRequest MoveRequest;
		MoveRequest.SetGoalActor(Target);
		MoveRequest.SetAcceptanceRadius(300.0f);

		FNavPathSharedPtr NavPath;

		AIController->MoveTo(MoveRequest, &NavPath);
	}
}

void AYaroCharacter::AgroSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		AEnemy* Enemy = Cast<AEnemy>(OtherActor);
		if (Enemy)
		{
			//UE_LOG(LogTemp, Log, TEXT("Yaro AgroSphereOnOverlapBegin %s"), *this->GetName());

			for (int i = 0; i < AgroTargets.Num(); i++)
			{
				if (Enemy == AgroTargets[i]) //already exist
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
				for (int i = 0; i < AgroTargets.Num(); i++)
				{
					if (Enemy == AgroTargets[i]) //already exist
						AgroTargets.Remove(Enemy); //타겟팅 가능 몹 배열에서 제거
				}
			}

			//UE_LOG(LogTemp, Log, TEXT("AgroSphereOnOverlapEnd %s"), *this->GetName());

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
							if (NPC.Value->AgroTargets[0]->GetEnemyMovementStatus() == EEnemyMovementStatus::EMS_Dead) return;
							//UE_LOG(LogTemp, Log, TEXT("AgroSphereOnOverlapEnd Go Help %s"), *this->GetName());

							MoveToTarget(NPC.Value->AgroTargets[0]);
						}
					}
				}
				else // 인식 범위에 다른 몬스터가 남아있음
				{
					//UE_LOG(LogTemp, Log, TEXT("I Have AgroTargets %s"), *this->GetName());
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
			for (int i = 0; i < CombatTargets.Num(); i++)
			{
				if (Enemy == CombatTargets[i]) //already exist
					return;
			}
			CombatTargets.Add(Enemy); // 전투 타겟 배열에 추가

			if (CombatTarget) return; // 이미 전투 중인 타겟이 있다면 리턴

            //UE_LOG(LogTemp, Log, TEXT("CombatSphereOnOverlapBegin %s"), *this->GetName());

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
            //UE_LOG(LogTemp, Log, TEXT("CombatSphereOnOverlapEnd %s"), *this->GetName());	

			for (int i = 0; i < CombatTargets.Num(); i++)
			{
				if (Enemy == CombatTargets[i]) //already exist
					CombatTargets.Remove(Enemy); //타겟팅 가능 몹 배열에서 제거
			}

			if (Enemy == CombatTarget) // 현재 전투 타겟이 전투 범위에서 나감
			{
				CombatTarget = nullptr;			

				if (CombatTargets.Num() != 0) // 전투 범위에 다른 몬스터 있음
				{
					//UE_LOG(LogTemp, Log, TEXT("I Have CombatTargets %s"), *this->GetName());

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

void AYaroCharacter::SetPosList()
{
	Pos.Add(FVector(2517.f, 5585.f, 3351.f));
	Pos.Add(FVector(2345.f, 4223.f, 2833.f));
	Pos.Add(FVector(2080.f, 283.f, 2838.f));
	Pos.Add(FVector(1550.f, -1761.f, 2843.f));
	Pos.Add(FVector(1026.f, -1791.f, 2576.f));
}

void AYaroCharacter::Attack()
{
	if ((!bAttacking) && (CombatTarget) && (CombatTarget->GetEnemyMovementStatus() != EEnemyMovementStatus::EMS_Dead))
	{
        //UE_LOG(LogTemp, Log, TEXT("Attack,  %s"), *this->GetName());
		if (NPCType == ENPCType::Vovo && bIsHealTime)
		{
			GetWorldTimerManager().SetTimer(AttackTimer, this, &AYaroCharacter::Attack, 2.f);
			return;
		}

		if (bCanCastStrom) // npc can cast strom magic
		{
			if (DialogueManager->GetDialogueNum() <= 5)
				SetSkillNum(FMath::RandRange(1, 3));
			else if (DialogueManager->GetDialogueNum() <= 14)
				SetSkillNum(FMath::RandRange(1, 4));
			else
				SetSkillNum(FMath::RandRange(1, 5));
		}
		else // npc can't cast strom magic
		{
			if (DialogueManager->GetDialogueNum() <= 5)
				SetSkillNum(FMath::RandRange(2, 3));
			else if (DialogueManager->GetDialogueNum() <= 14)
				SetSkillNum(FMath::RandRange(2, 4));
			else
				SetSkillNum(FMath::RandRange(2, 5));
		}
		
        if (GetSkillNum() == 1) // 현재 공격이 스톰
        {
            bCanCastStrom = false;
            FTimerHandle StormTimer;
            GetWorldTimerManager().SetTimer(StormTimer, this, &AYaroCharacter::CanCastStormMagic, 8.f, false);  // 8초 뒤 다시 스톰 사용 가능
        }


		ToSpawn = AttackSkillData->FindRow<FAttackSkillData>("Attack", "")->Skills[GetSkillNum() - 1];

		bAttacking = true;
		SetInterpToEnemy(true); // 적 방향으로 회전 가능

		if (AnimInstance && CombatMontage)
		{
			// 공격 몽타주 재생
			AnimInstance->Montage_Play(CombatMontage);
			AnimInstance->Montage_JumpToSection(FName("Attack"), CombatMontage);
			// 마법 스폰
			Spawn();
		}
	}
	else
	{
		AttackEnd();
        //UE_LOG(LogTemp, Log, TEXT("AttackEnd... %s"), * this->GetName());
	}
}

void AYaroCharacter::AttackEnd()
{
	bAttacking = false;
	SetInterpToEnemy(false);
    //UE_LOG(LogTemp, Log, TEXT("AttackEnd %s"), *this->GetName());

	if (bOverlappingCombatSphere) 
	{
		// 현재 전투 타겟이 죽음
		if (CombatTarget && CombatTarget->GetEnemyMovementStatus() == EEnemyMovementStatus::EMS_Dead)
		{
			//UE_LOG(LogTemp, Log, TEXT("CombatTarget died %s"), *this->GetName());
			for (int i = 0; i < CombatTargets.Num(); i++)
			{
				if (CombatTarget == CombatTargets[i]) //already exist
				{
					CombatTargets.Remove(CombatTarget); // 전투 가능 몹 배열에서 제거
				}
			}			

			for (int i = 0; i < AgroTargets.Num(); i++)
			{
				if (CombatTarget == AgroTargets[i]) //already exist
				{
					AgroTargets.Remove(CombatTarget); //인식된 몹 배열에서 제거
				}
			}

			CombatTarget = nullptr;

			if (CombatTargets.Num() == 0) // 전투 범위에 몬스터가 없음
			{
				bOverlappingCombatSphere = false;
                //UE_LOG(LogTemp, Log, TEXT("AttackEnd - No CombatTargets %s"), *this->GetName());
				
				if (AgroTargets.Num() != 0) // 인식 범위에 몬스터가 있음
				{
					MoveToTarget(AgroTargets[0]);
					//UE_LOG(LogTemp, Log, TEXT("AttackEnd - but havve AgroTargets %s"), *this->GetName());
				}
				else // 인식 범위에 몬스터 없음
				{
					for (auto NPC : NPCManager->GetNPCMap())
					{
						// 다른 npc의 인식 범위에 몬스터가 있으면 도와주러 감, 첫번째 던전 제외
						if (NPC.Value->AgroTargets.Num() != 0
							&& !UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("first")) 
						{
							//UE_LOG(LogTemp, Log, TEXT("AttackEnd - Go Help %s"), *this->GetName());
							MoveToTarget(NPC.Value->AgroTargets[0]);
						}
					}
				}
			}
			else // 전투 범위에 몬스터가 있으면
			{
				// 전투 타겟 새로 설정
				CombatTarget = CombatTargets[0];
                //UE_LOG(LogTemp, Log, TEXT("AttackEnd - i have CombatTarget %s"), * this->GetName());
			}
		}

		if (CombatTarget)
			GetWorldTimerManager().SetTimer(AttackTimer, this, &AYaroCharacter::Attack, AttackDelay);
	}
	else // 전투 범위에 아무도 없음
	{
		if (AgroTargets.Num() != 0) // 인식 범위에 몬스터 존재
		{
			//UE_LOG(LogTemp, Log, TEXT("AttackEnd2 - No Combat Yes Agro %s"), *this->GetName());
			for (int i = 0; i < AgroTargets.Num(); i++)
			{
				// 인식 범위에 있는 몬스터가 살아있다면
				if (AgroTargets[i]->GetEnemyMovementStatus() != EEnemyMovementStatus::EMS_Dead)
				{
					if (GetDistanceTo(AgroTargets[i]) >= 450.f) // 거리가 멀면 추적
					{
						MoveToTarget(AgroTargets[i]);
						//UE_LOG(LogTemp, Log, TEXT("AttackEnd2 - Far Move %s"), *this->GetName());
						return;
					}
					else // 거리가 멀지 않으면 전투 타겟으로 바로 설정 후 공격
					{
						//UE_LOG(LogTemp, Log, TEXT("AttackEnd2 - Close %s"), *this->GetName());
						CombatTarget = AgroTargets[i];
						GetWorldTimerManager().SetTimer(AttackTimer, this, &AYaroCharacter::Attack, AttackDelay);
						return;
					}
				}
			}
		}
	}
}

void AYaroCharacter::SetAgroSphere()
{
	AgroSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AgroSphere"));
	AgroSphere->SetupAttachment(GetRootComponent());
	AgroSphere->InitSphereRadius(600.f);
	AgroSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	AgroSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Ignore);
	AgroSphere->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
}

void AYaroCharacter::SetCombatSphere()
{
	CombatSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatSphere"));
	CombatSphere->SetupAttachment(GetRootComponent());
	CombatSphere->InitSphereRadius(480.f);
	CombatSphere->SetRelativeLocation(FVector(250.f, 0.f, 0.f));
	CombatSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	CombatSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Ignore);
	CombatSphere->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
}

void AYaroCharacter::SetNotAllowSphere()
{
	NotAllowSphere = CreateDefaultSubobject<USphereComponent>(TEXT("NotAllowSphere"));
	NotAllowSphere->SetupAttachment(GetRootComponent());
	NotAllowSphere->InitSphereRadius(100.f);
	NotAllowSphere->SetRelativeLocation(FVector(50.f, 0.f, 0.f));
	NotAllowSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	NotAllowSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Ignore);
	NotAllowSphere->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
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

				if (MagicAttack.IsValid() && CombatTarget)
				{ // 타겟 적과 캐스터 정보 전달
					MagicAttack->Target = CombatTarget;
					MagicAttack->Caster = this;
				}
			}
		}), 0.6f, false); // 0.6초 뒤 실행, 반복X
	}
}

void AYaroCharacter::CanCastStormMagic()
{
	bCanCastStrom = true;
}

void AYaroCharacter::MoveToLocation() // Vivi, Vovo, Zizi만 실행
{	
	if (AIController)
	{
		AIController->MoveToLocation(Pos[index]);
	}

	GetWorld()->GetTimerManager().SetTimer(TeamMoveTimer, FTimerDelegate::CreateLambda([&]() {

		if (!CombatTarget && !bOverlappingCombatSphere) // 전투 중이 아닐 때
		{
            float distance = (GetActorLocation() - Pos[index]).Size();

			if (index <= 3)
			{
				if (AIController && distance <= 70.f) // 목표 지점과 가까울 때
				{
					index++;
					AIController->MoveToLocation(Pos[index]); // 다음 목표 지점으로 이동
				}
				else // 목표 지점과 멀 때
				{
					AIController->MoveToLocation(Pos[index]); // 목표 지점으로 계속해서 이동
				}
			}

			if (index == 4 && distance <= 70.f) // 골렘쪽으로 이동 중
			{
				// 목표 지점 이동을 모두 끝내고, 모모 혹은 플레이어가 골렘과 전투 중이면 골렘 쪽으로 이동
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
	// 보스 스테이지 제외 리턴
	if (!UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("boss")) 
		return DamageAmount;
	
	MagicAttack = Cast<AMagicSkill>(DamageCauser);
	if (MagicAttack == nullptr) return DamageAmount;

	int TargetIndex = MagicAttack->index;
	
	// 인식 범위에 아무도 없는 상태에서 보스 몬스터의 공격을 받음
	if (TargetIndex == 11 && AgroTargets.Num() == 0) 
	{
		AEnemy* BossEnemy = Cast<AEnemy>(UGameplayStatics::GetActorOfClass(GetWorld(), Boss));
		GetWorldTimerManager().ClearTimer(MoveTimer);
		AIController->StopMovement();
		MoveToTarget(BossEnemy); // 보스에게 이동
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
			for (int i = 0; i < DangerousTargets.Num(); i++)
			{
				if (Enemy == DangerousTargets[i]) //already exist
					return;
			}
			DangerousTargets.Add(Enemy);

			// 이미 타이머 실행 중이면 리턴
			if (GetWorldTimerManager().IsTimerActive(SafeDistanceTimer)) return;

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
			for (int i = 0; i < DangerousTargets.Num(); i++)
			{
				if (Enemy == DangerousTargets[i]) //already exist
				{
					DangerousTargets.Remove(Enemy);
				}
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
		return;
	}

	float value = FMath::RandRange(-200.f, 200.f);
	// 현재 위치에서 랜덤한 방향으로 조금 이동
	FVector SafeLocation = GetActorLocation() + FVector(value, value, 0.f);
	AIController->MoveToLocation(SafeLocation);
	//UE_LOG(LogTemp, Log, TEXT("movwsafeg %s"), *this->GetName());
}

void AYaroCharacter::ClearTeamMoveTimer()
{
	GetWorldTimerManager().ClearTimer(TeamMoveTimer);
}

void AYaroCharacter::ClearAllTimer()
{
	GetWorldTimerManager().ClearTimer(TeamMoveTimer);
	GetWorldTimerManager().ClearTimer(MoveTimer);
	GetWorldTimerManager().ClearTimer(MagicSpawnTimer);
}

void AYaroCharacter::Smile() // 웃는 표정
{
	this->bSmile = true;
}

void AYaroCharacter::UsualFace() // 평소 표정
{
	this->bSmile = false;
}