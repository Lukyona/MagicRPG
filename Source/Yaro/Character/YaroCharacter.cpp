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

void AYaroCharacter::MoveToPlayer()
{
	if (Player == nullptr) return;

	if (CombatTarget == nullptr && !bAttacking && !bOverlappingCombatSphere && AgroTargets.Num() == 0) // ���� ���� �ƴ� ��
	{
		if (DialogueManager->IsDialogueUIVisible()) // ��ȭ �߿��� ��κ� �̵�X
		{
			if (DialogueManager->GetDialogueNum() != 6) // Ư�� ���� �����ϰ� �÷��̾ ���󰡱�
			{
				GetWorldTimerManager().SetTimer(MoveTimer, this, &AYaroCharacter::MoveToPlayer, 0.5f);
				return;
			}
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

				//if(!GetWorldTimerManager().IsTimerActive(MoveTimer))
					GetWorldTimerManager().SetTimer(MoveTimer, this, &AYaroCharacter::MoveToPlayer, 1.f);

				return;
			}
		}

		// �÷��̾���� �Ÿ�
        float distance = GetDistanceTo(Player);

        if (distance >= 500.f) //���� �Ÿ� �̻� �������ִٸ� �ӵ� ���� �޸���
        {
			GetCharacterMovement()->MaxWalkSpeed = RunSpeed;

			// ù��° ���������� npc���� ������ ���ų� ���缭 ���࿡ ���ص��� �ʵ��� �ڷ���Ʈ ��� ����
			if (UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("first")
				&& !GetWorldTimerManager().IsTimerActive(TeleportTimer))
				Teleport();
        }
        else //�����ٸ� �ӵ� ���� �ȱ�
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
	else // ���� ��
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

	// �ð��� ���� ����ϸ� �÷��̾� ��ó�� �����̵�
	if (TeleportCount >= 25) SetActorLocation(Player->GetActorLocation() + FVector(15.f, 25.f, 0.f));


	float distance = (GetActorLocation() - Player->GetActorLocation()).Size();

	if (distance <= 400.f) // �÷��̾�� �Ÿ� �����
	{
		if (AIController)
		{
			TeleportCount = 0;
			GetWorldTimerManager().ClearTimer(TeleportTimer);
		}
	}
	else // �÷��̾�� �Ÿ��� �ָ� ��� �ڷ���Ʈ ī��Ʈ ����
	{
		GetWorldTimerManager().SetTimer(TeleportTimer, this, &AYaroCharacter::Teleport, 0.5f);
	}
}

void AYaroCharacter::MoveToTarget(AEnemy* Target) // �� ����
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
				for (int i = 0; i < AgroTargets.Num(); i++)
				{
					if (Enemy == AgroTargets[i]) //already exist
						AgroTargets.Remove(Enemy); //Ÿ���� ���� �� �迭���� ����
				}
			}

			//UE_LOG(LogTemp, Log, TEXT("AgroSphereOnOverlapEnd %s"), *this->GetName());

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
							//UE_LOG(LogTemp, Log, TEXT("AgroSphereOnOverlapEnd Go Help %s"), *this->GetName());

							MoveToTarget(NPC.Value->AgroTargets[0]);
						}
					}
				}
				else // �ν� ������ �ٸ� ���Ͱ� ��������
				{
					//UE_LOG(LogTemp, Log, TEXT("I Have AgroTargets %s"), *this->GetName());
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
			for (int i = 0; i < CombatTargets.Num(); i++)
			{
				if (Enemy == CombatTargets[i]) //already exist
					return;
			}
			CombatTargets.Add(Enemy); // ���� Ÿ�� �迭�� �߰�

			if (CombatTarget) return; // �̹� ���� ���� Ÿ���� �ִٸ� ����

            //UE_LOG(LogTemp, Log, TEXT("CombatSphereOnOverlapBegin %s"), *this->GetName());

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
            //UE_LOG(LogTemp, Log, TEXT("CombatSphereOnOverlapEnd %s"), *this->GetName());	

			for (int i = 0; i < CombatTargets.Num(); i++)
			{
				if (Enemy == CombatTargets[i]) //already exist
					CombatTargets.Remove(Enemy); //Ÿ���� ���� �� �迭���� ����
			}

			if (Enemy == CombatTarget) // ���� ���� Ÿ���� ���� �������� ����
			{
				CombatTarget = nullptr;			

				if (CombatTargets.Num() != 0) // ���� ������ �ٸ� ���� ����
				{
					//UE_LOG(LogTemp, Log, TEXT("I Have CombatTargets %s"), *this->GetName());

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
		
        if (GetSkillNum() == 1) // ���� ������ ����
        {
            bCanCastStrom = false;
            FTimerHandle StormTimer;
            GetWorldTimerManager().SetTimer(StormTimer, this, &AYaroCharacter::CanCastStormMagic, 8.f, false);  // 8�� �� �ٽ� ���� ��� ����
        }


		ToSpawn = AttackSkillData->FindRow<FAttackSkillData>("Attack", "")->Skills[GetSkillNum() - 1];

		bAttacking = true;
		SetInterpToEnemy(true); // �� �������� ȸ�� ����

		if (AnimInstance && CombatMontage)
		{
			// ���� ��Ÿ�� ���
			AnimInstance->Montage_Play(CombatMontage);
			AnimInstance->Montage_JumpToSection(FName("Attack"), CombatMontage);
			// ���� ����
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
		// ���� ���� Ÿ���� ����
		if (CombatTarget && CombatTarget->GetEnemyMovementStatus() == EEnemyMovementStatus::EMS_Dead)
		{
			//UE_LOG(LogTemp, Log, TEXT("CombatTarget died %s"), *this->GetName());
			for (int i = 0; i < CombatTargets.Num(); i++)
			{
				if (CombatTarget == CombatTargets[i]) //already exist
				{
					CombatTargets.Remove(CombatTarget); // ���� ���� �� �迭���� ����
				}
			}			

			for (int i = 0; i < AgroTargets.Num(); i++)
			{
				if (CombatTarget == AgroTargets[i]) //already exist
				{
					AgroTargets.Remove(CombatTarget); //�νĵ� �� �迭���� ����
				}
			}

			CombatTarget = nullptr;

			if (CombatTargets.Num() == 0) // ���� ������ ���Ͱ� ����
			{
				bOverlappingCombatSphere = false;
                //UE_LOG(LogTemp, Log, TEXT("AttackEnd - No CombatTargets %s"), *this->GetName());
				
				if (AgroTargets.Num() != 0) // �ν� ������ ���Ͱ� ����
				{
					MoveToTarget(AgroTargets[0]);
					//UE_LOG(LogTemp, Log, TEXT("AttackEnd - but havve AgroTargets %s"), *this->GetName());
				}
				else // �ν� ������ ���� ����
				{
					for (auto NPC : NPCManager->GetNPCMap())
					{
						// �ٸ� npc�� �ν� ������ ���Ͱ� ������ �����ַ� ��, ù��° ���� ����
						if (NPC.Value->AgroTargets.Num() != 0
							&& !UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("first")) 
						{
							//UE_LOG(LogTemp, Log, TEXT("AttackEnd - Go Help %s"), *this->GetName());
							MoveToTarget(NPC.Value->AgroTargets[0]);
						}
					}
				}
			}
			else // ���� ������ ���Ͱ� ������
			{
				// ���� Ÿ�� ���� ����
				CombatTarget = CombatTargets[0];
                //UE_LOG(LogTemp, Log, TEXT("AttackEnd - i have CombatTarget %s"), * this->GetName());
			}
		}

		if (CombatTarget)
			GetWorldTimerManager().SetTimer(AttackTimer, this, &AYaroCharacter::Attack, AttackDelay);
	}
	else // ���� ������ �ƹ��� ����
	{
		if (AgroTargets.Num() != 0) // �ν� ������ ���� ����
		{
			//UE_LOG(LogTemp, Log, TEXT("AttackEnd2 - No Combat Yes Agro %s"), *this->GetName());
			for (int i = 0; i < AgroTargets.Num(); i++)
			{
				// �ν� ������ �ִ� ���Ͱ� ����ִٸ�
				if (AgroTargets[i]->GetEnemyMovementStatus() != EEnemyMovementStatus::EMS_Dead)
				{
					if (GetDistanceTo(AgroTargets[i]) >= 450.f) // �Ÿ��� �ָ� ����
					{
						MoveToTarget(AgroTargets[i]);
						//UE_LOG(LogTemp, Log, TEXT("AttackEnd2 - Far Move %s"), *this->GetName());
						return;
					}
					else // �Ÿ��� ���� ������ ���� Ÿ������ �ٷ� ���� �� ����
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

				if (MagicAttack.IsValid() && CombatTarget)
				{ // Ÿ�� ���� ĳ���� ���� ����
					MagicAttack->Target = CombatTarget;
					MagicAttack->Caster = this;
				}
			}
		}), 0.6f, false); // 0.6�� �� ����, �ݺ�X
	}
}

void AYaroCharacter::CanCastStormMagic()
{
	bCanCastStrom = true;
}

void AYaroCharacter::MoveToLocation() // Vivi, Vovo, Zizi�� ����
{	
	if (AIController)
	{
		AIController->MoveToLocation(Pos[index]);
	}

	GetWorld()->GetTimerManager().SetTimer(TeamMoveTimer, FTimerDelegate::CreateLambda([&]() {

		if (!CombatTarget && !bOverlappingCombatSphere) // ���� ���� �ƴ� ��
		{
            float distance = (GetActorLocation() - Pos[index]).Size();

			if (index <= 3)
			{
				if (AIController && distance <= 70.f) // ��ǥ ������ ����� ��
				{
					index++;
					AIController->MoveToLocation(Pos[index]); // ���� ��ǥ �������� �̵�
				}
				else // ��ǥ ������ �� ��
				{
					AIController->MoveToLocation(Pos[index]); // ��ǥ �������� ����ؼ� �̵�
				}
			}

			if (index == 4 && distance <= 70.f) // �������� �̵� ��
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

	int TargetIndex = MagicAttack->index;
	
	// �ν� ������ �ƹ��� ���� ���¿��� ���� ������ ������ ����
	if (TargetIndex == 11 && AgroTargets.Num() == 0) 
	{
		AEnemy* BossEnemy = Cast<AEnemy>(UGameplayStatics::GetActorOfClass(GetWorld(), Boss));
		GetWorldTimerManager().ClearTimer(MoveTimer);
		AIController->StopMovement();
		MoveToTarget(BossEnemy); // �������� �̵�
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

void AYaroCharacter::MoveToSafeLocation() // ������ ��ġ�� �̵�, ���Ϳ��� ���� �Ÿ� ����
{
	if (DangerousTargets.Num() != 0) 
	{
		GetWorldTimerManager().SetTimer(SafeDistanceTimer, this, &AYaroCharacter::MoveToSafeLocation, 3.f);
	}
	else // ���Ϳ��� ���� �Ÿ� Ȯ����
	{
		GetWorldTimerManager().ClearTimer(SafeDistanceTimer);
		return;
	}

	float value = FMath::RandRange(-200.f, 200.f);
	// ���� ��ġ���� ������ �������� ���� �̵�
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

void AYaroCharacter::Smile() // ���� ǥ��
{
	this->bSmile = true;
}

void AYaroCharacter::UsualFace() // ��� ǥ��
{
	this->bSmile = false;
}