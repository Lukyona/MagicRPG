// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Animation/AnimInstance.h"
#include "Sound/SoundCue.h"
#include "Engine/EngineTypes.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "AIController.h"

#include "Yaro/System/MainPlayerController.h"
#include "Yaro/System/GameManager.h"
#include "Yaro/System/DialogueManager.h"
#include "Yaro/System/NPCManager.h"
#include "Yaro/Character/Main.h"
#include "Yaro/Character/YaroCharacter.h"
#include "Yaro/MagicSkill.h"

const int32 MAX_RETURN_COUNT = 20;
const int32 MAX_CHASE_COUNT = 20;
// Sets default values
AEnemy::AEnemy()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	EnemyMovementStatus = EEnemyMovementStatus::EMS_Idle;

	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	GameManager = Cast<UGameManager>(GetWorld()->GetGameInstance());
	if (GameManager)
	{
		NPCManager = GameManager->GetNPCManager();
	}

	AIController = Cast<AAIController>(GetController());
	AnimInstance = GetMesh()->GetAnimInstance();

	InitialLocation = GetActorLocation(); 
	InitialRotation = GetActorRotation();

	Health = MaxHealth;

	BindSphereComponentEvents();
	if(!bIsRangedAttacker)
	{
		BindWeaponCollisionEvents();
	}

	SetMain();
}

USphereComponent* AEnemy::CreateSphereComponent(FName Name, float Radius)
{
	USphereComponent* Sphere = CreateDefaultSubobject<USphereComponent>(Name);
	Sphere->SetupAttachment(GetRootComponent());
	Sphere->InitSphereRadius(Radius);
	Sphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	Sphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Ignore);
	Sphere->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);

	return Sphere;
}

void AEnemy::BindSphereComponentEvents()
{
	AgroSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::AgroSphereOnOverlapBegin);
	AgroSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::AgroSphereOnOverlapEnd);

	CombatSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::CombatSphereOnOverlapBegin);
	CombatSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::CombatSphereOnOverlapEnd);
}

UBoxComponent* AEnemy::CreateCollision(FName Name, FName SocketName)
{
	UBoxComponent* Collision = CreateDefaultSubobject<UBoxComponent>(Name);
	Collision->SetupAttachment(GetMesh(), SocketName);
	Collision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Collision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	Collision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	Collision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	return Collision;
}

void AEnemy::CreateWeaponCollisions()
{
	CreateCollision("CombatCollision", "EnemySocket_1");
	if (bHasSecondCollision)
	{
		CreateCollision("CombatCollision2", "EnemySocket_2");
	}
}

void AEnemy::BindWeaponCollisionEvents()
{
	CombatCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::WeaponCollisionOnOverlapBegin);
	CombatCollision->OnComponentEndOverlap.AddDynamic(this, &AEnemy::WeaponCollisionOnOverlapEnd);
	if (bHasSecondCollision && CombatCollision2)
	{
		CombatCollision2->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::WeaponCollisionOnOverlapBegin);
		CombatCollision2->OnComponentEndOverlap.AddDynamic(this, &AEnemy::WeaponCollisionOnOverlapEnd);
	}
}

void AEnemy::CreateSpheresAndCollisions()
{
	AgroSphere = CreateSphereComponent("AgroSphere", AgroSphereRadius);
	CombatSphere = CreateSphereComponent("CombatSphere", CombatSphereRadius);
	if(!bIsRangedAttacker)
	{
		CreateWeaponCollisions();
	}
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    if (!Main) 
	{
		SetMain();
	}

	if (bInterpToTarget && CombatTarget)
	{
		FRotator LookAtYaw = GetLookAtRotationYaw(CombatTarget->GetActorLocation());
		FRotator InterpRotation = FMath::RInterpTo(GetActorRotation(), LookAtYaw, DeltaTime, InterpSpeed); //smooth transition
		SetActorRotation(InterpRotation);
	}
}

void AEnemy::SetMain()
{
	Main = Cast<AMain>(UGameplayStatics::GetPlayerCharacter(this, 0));
}

FRotator AEnemy::GetLookAtRotationYaw(FVector Target)
{
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Target);
	FRotator LookAtRotationYaw(0.f, LookAtRotation.Yaw, 0.f);
	return LookAtRotationYaw;
}

bool AEnemy::IsValidTarget(AActor* Target)
{
	return Target && !Cast<AEnemy>(Target) && !IsDead();
}

void AEnemy::AgroSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsValidTarget(OtherActor))
	{
		return;
	}
	
	if (AgroSound && AgroTargets.Num() == 0) // �ν� ������ �ƹ��� �������� �ν� ���� ���
	{
		if(!UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("boss"))
		{
			UGameplayStatics::PlaySound2D(this, AgroSound);
		}
	}

	AStudent* Target = Cast<AStudent>(OtherActor);
	if (!Target || AgroTargets.Contains(Target)) 
	{
		return;
	}

	AgroTargets.Add(Target); // Add to target list

	if (Target == Main)
	{
		MoveToTarget(Main);
	}
	else // npc�� �ν� ���� ���� ����
	{	
		if (!CombatTarget && AgroTarget != Main) //�ٸ� npc�� ���� ���� �ƴϾ���ϰ�, �÷��̾ ���󰡴� ���� �ƴϾ����
		{
			MoveToTarget(Target);
		}				
	}
}

void AEnemy::AgroSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!IsValidTarget(OtherActor)) 
	{
		return;
	}

	AStudent* Target = Cast<AStudent>(OtherActor);
	if (Target)
	{
		if (!UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("boss"))
		{
			if(AgroTargets.Contains(Target))
			{
				AgroTargets.Remove(Target);
			}
		}
			
		if (AgroTarget != nullptr && AgroTarget != Main) //npc�� �Ѿư��� ���̸�(�ν� ���� ���� �͵� npc)
		{
			AgroTarget = nullptr;
			if (CombatTargets.Num() != 0) //���� ������ �ٸ� ��������(npc) ������
			{
				CombatTarget = CombatTargets[0];
				if (EnemyMovementStatus == EEnemyMovementStatus::EMS_Attacking)
				{
					AttackEnd();
				}
			}
			else if (AgroTargets.Num() != 0) //������������ �ƹ��� ������ �νĹ����� �������� ������
			{
				MoveToTarget(AgroTargets[0]);			
			}
		}
		else //�÷��̾ �Ѿư��� ��
		{
			if (Target == Main) //�÷��̾ �ν� ���� ������ ����
			{
				AgroTarget = nullptr;
			}
		}

		if (AgroTargets.Num() == 0) // no one's in agrosphere
		{
			AIController->StopMovement();
			MoveToInitialLocation();
		}
	}
}

void AEnemy::CombatSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsValidTarget(OtherActor)) 
	{
		return;
	}

	AStudent* Target = Cast<AStudent>(OtherActor);
	if (Target)
	{			
		if (CombatTargets.Contains(Target)) 
		{
			return;
		}

		CombatTargets.Add(Target); // Add to target list

		if (CombatTarget && Target != Main) //����Ÿ���� �ִµ�, ���������� �ٸ� npc�� ������ ����
		{
			return;
		}

		if ((AgroTarget == Main && Target == Main) || AgroTarget != Main) //npc�� �Ѿư��� ��(������ ���������� ����) Ȥ�� �÷��̾ �Ѵ� �߿� ���������� �÷��̾ ������ ��
		{
			AgroTarget = nullptr;
			CombatTarget = Target;
			Attack();		
		}
	}
}

void AEnemy::CombatSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!IsValidTarget(OtherActor)) 
	{
		return;
	}

	AStudent* Target = Cast<AStudent>(OtherActor);
	if (!Target) 
	{
		return;
	}

	AnimInstance->Montage_Stop(0.1f, CombatMontage);
		
	if (Target == CombatTarget)
	{
		CombatTarget = nullptr; //����Ÿ���� ���������� ����
		if (EnemyMovementStatus == EEnemyMovementStatus::EMS_Attacking)
		{
			AttackEnd();
		}
	}

	if(CombatTargets.Contains(Target))
	{
		CombatTargets.Remove(Target);
	}

	if(Target == Main && Main->GetMovementStatus() != EMovementStatus::EMS_Dead) // �÷��̾ ����ִ� ���·� ���������� ���� 
	{
		MoveToTarget(Target);
	}

	if (CombatTargets.Num() == 0) // no one's in Combatsphere
	{
		if (AgroTarget != Main && AgroTargets.Num() != 0)
		{
			MoveToTarget(AgroTargets[0]);
		}
	}
	else //�������� ���� �ٸ� �������� ������
	{		
		if (AgroTarget != Main) //�÷��̾ �Ѿư��� ���� �ƴϸ�
		{
			CombatTarget = CombatTargets[0];
			Attack();
		}
	}	
}

void AEnemy::WeaponCollisionOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (auto actor = Cast<AEnemy>(OtherActor)) 
	{
		return;
	}

	if (OtherActor)
	{
		AStudent* Target = Cast<AStudent>(OtherActor);
		if (Target)
		{
			UGameplayStatics::ApplyDamage(Target, Damage, AIController, this, UDamageType::StaticClass());
		}
	}
}

void AEnemy::WeaponCollisionOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}

void AEnemy::ActivateWeaponCollisions()
{
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	if(bHasSecondCollision)
	{
		CombatCollision2->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
}

void AEnemy::DeactivateWeaponCollisions()
{
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (bHasSecondCollision)
	{
		CombatCollision2->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AEnemy::MoveToTarget(AStudent* Target)
{
	if (AIController && !IsDead())
	{
		if (Target == Main && Main->IsDead())
		{
			if (AgroTargets.Num() == 0)
			{
				MoveToInitialLocation();
				return;
			}
		}
		EnemyMovementStatus = EEnemyMovementStatus::EMS_MoveToTarget;

		if(GetWorldTimerManager().IsTimerActive(CheckLocationTimer))
		{
			GetWorldTimerManager().ClearTimer(CheckLocationTimer);
		}

		AgroTarget = Target;
		FAIMoveRequest MoveRequest;
		MoveRequest.SetGoalActor(Target);
		MoveRequest.SetAcceptanceRadius(15.0f);
		FNavPathSharedPtr NavPath;
		AIController->MoveTo(MoveRequest, &NavPath);
		CheckChaseState();

		/** ��׷�(�ν�)����(�� �ݸ���)�� �浹���� �� � ��θ� ���� ���Ͱ� ��������� Ȯ�� ����
		auto PathPoints = NavPath->GetPathPoints();
		for (auto Point : PathPoints)
		{
			FVector Location = Point.Location;
			UKismetSystemLibrary::DrawDebugSphere(this, Location, 25.8f, 8, FLinearColor::Green, 10.f, 1.5f);
		}
		*/
	}
}

void AEnemy::Attack()
{
	if (!IsDead())
	{
		if (EnemyMovementStatus != EEnemyMovementStatus::EMS_Attacking && CombatTarget)
		{
			if (CombatTarget == Main && Main->IsDead())
			{
				AttackEnd();
				return;
			}

			if (AIController)
			{
				AIController->StopMovement();
			}

			EnemyMovementStatus = EEnemyMovementStatus::EMS_Attacking;
			bInterpToTarget = false;

			if (AnimInstance)
			{
				AnimInstance->Montage_Play(CombatMontage);

				if (EnemyType == EEnemyType::Grux || EnemyType == EEnemyType::LizardShaman
					|| EnemyType == EEnemyType::Archer || EnemyType == EEnemyType::Spider)
				{
					AnimInstance->Montage_JumpToSection(FName("Attack"), CombatMontage);
				}
				else
				{
					int32 MaxAttackNum = 0;
					if(EnemyType == EEnemyType::LittleDino || EnemyType == EEnemyType::Lizard)
					{
						MaxAttackNum = 2;
					}
					else
					{
						MaxAttackNum = 3;
					}

					int32 AttackNum = FMath::RandRange(1, MaxAttackNum);
					AnimInstance->Montage_JumpToSection(FName("Attack" + FString::FromInt(AttackNum)), CombatMontage);
				}
			}
		}
		else
		{
			if(EnemyMovementStatus == EEnemyMovementStatus::EMS_Attacking) 
			{
				AttackEnd();
			}
		}
	}
}

void AEnemy::AttackEnd()
{
	EnemyMovementStatus = EEnemyMovementStatus::EMS_Idle;
	bInterpToTarget = true;

	if (CombatTargets.Num() != 0 && !AgroTarget)
	{
		if (CombatTarget)
		{
			if (CombatTarget == Main && Main->IsDead())
			{
				AgroTargets.Remove(Main);
				CombatTargets.Remove(Main);

				if (CombatTargets.Num() == 0) // no one's in Combatsphere
				{
					if (AgroTargets.Num() != 0) // �νĹ��� ���� �ٸ� �������� ������
					{
						MoveToTarget(AgroTargets[0]);
					}
				}
				else //�������� ���� �ٸ� �������� ������
				{
					CombatTarget = CombatTargets[0];
				}
			}
		}
		FTimerHandle AttackTimer;
		GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, AttackDelay);
	}
}

float AEnemy::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	MagicAttack = Cast<AMagicSkill>(DamageCauser);
	if(MagicAttack != nullptr && MagicAttack->GetCaster() == ECasterType::Player)
	{
		bAttackFromPlayer = true;
	}
		
	if (Health - DamageAmount <= 0.f) // Decrease Health
	{
		Health = 0.f;
		Die();
	}
	else
	{
		if (AnimInstance && EnemyMovementStatus != EEnemyMovementStatus::EMS_Attacking)
		{
			AnimInstance->Montage_Play(CombatMontage);
			AnimInstance->Montage_JumpToSection(FName("Hit"), CombatMontage);
		}
		Health -= DamageAmount;
	}
	return DamageAmount;
}

void AEnemy::Die()
{
	if (EnemyMovementStatus != EEnemyMovementStatus::EMS_Dead)
	{
		GameManager->UpdateDeadEnemy(EnemyType);

		if (AIController) 
		{
			AIController->StopMovement();
		}

		bInterpToTarget = false;

		if (DeathSound && !GameManager->IsSkipping())
		{
			UGameplayStatics::PlaySound2D(this, DeathSound);
		}

		if (bAttackFromPlayer) 
		{
			Main->GainExp(EnemyExp);
		}

		if (AnimInstance && !GameManager->IsSkipping())
		{
			AnimInstance->Montage_Play(CombatMontage);
			AnimInstance->Montage_JumpToSection(FName("Death"), CombatMontage);
		}
		EnemyMovementStatus = EEnemyMovementStatus::EMS_Dead;
	}
}

void AEnemy::DeathEnd()
{
	GetMesh()->bPauseAnims = true;
	GetMesh()->bNoSkeletonUpdate = true;
	
	if (UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("first")) // ù��° ���� Ŭ�����ߴ���
	{
		if (GameManager->GetDeadEnemies().Find(EEnemyType::Golem) != nullptr)
		{
			const int32* GoblinCount = GameManager->GetDeadEnemies().Find(EEnemyType::Goblin);
			const int32* GruxCount = GameManager->GetDeadEnemies().Find(EEnemyType::Grux);
			if (GoblinCount != nullptr && GruxCount != nullptr)
			{
				if ((*GoblinCount + *GruxCount) == 8)
				{
					GameManager->SaveGame();
					GameManager->GetDialogueManager()->DisplayDialogueUI();
				}
			}
		}
	}
	
	if (UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("second")) // �Ź� óġ �� Ȥ�� ������ ���� óġ ��
	{
		if (GameManager->GetDeadEnemies().Find(EnemyType) != nullptr)
		{
			if ((EnemyType == EEnemyType::Spider && GameManager->GetDeadEnemies()[EnemyType] == 5)
				|| (EnemyType == EEnemyType::LittleMonster && GameManager->GetDeadEnemies()[EnemyType] == 3))
			{
				NPCManager->AllNpcStopFollowPlayer();
				GameManager->SaveGame();
				GameManager->GetDialogueManager()->DisplayDialogueUI();
			}
		}
	}

	if (UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("boss"))
	{
		if (GameManager->GetDeadEnemies().Find(EEnemyType::Boss) != nullptr)
		{
			const int32* ShamanCount = GameManager->GetDeadEnemies().Find(EEnemyType::LizardShaman);
			if (ShamanCount && *ShamanCount == 7)
			{
				GameManager->GetDialogueManager()->DisplayDialogueUI();
			}
		}
	}
	FTimerHandle DeathTimer;
	GetWorldTimerManager().SetTimer(DeathTimer, this, &AEnemy::Disappear, DeathDelay);
}

void AEnemy::Disappear()
{
	DisableWeaponCollisions();
	DisableSphereCollisions();
	Destroy();
}

void AEnemy::DisableWeaponCollisions()
{
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CombatCollision->DestroyComponent();
	CombatCollision = nullptr;
	if (bHasSecondCollision)
	{
		CombatCollision2->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		CombatCollision2->DestroyComponent();
		CombatCollision2 = nullptr;
	}
}

void AEnemy::DisableSphereCollisions()
{
	AgroSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CombatSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	AgroSphere->DestroyComponent();
	CombatSphere->DestroyComponent();

	AgroSphere = nullptr;
	CombatSphere = nullptr;

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemy::HitEnd()
{
	if (!Main) SetMain();

	if (AgroTarget) // �������� �Ѿư��� �־����� �ٽ� �Ѿư���
	{
		MoveToTarget(AgroTarget);
	}

	if (!CombatTarget && AgroSound) 
	{
		UGameplayStatics::PlaySound2D(this, AgroSound);
	}
	
	/* When enemy doesn't have combat target, player attacks enemy
	or when enemy's combat target is not player and player attacks enemy.
	At this time, enemy must sets player as a combat target.
	*/
	if (MagicAttack != nullptr)
	{
		if (MagicAttack->GetCaster() == ECasterType::Player)
		{
			bAttackFromPlayer = true;
			if (CombatTarget == nullptr || CombatTarget != Main)
			{
				if (CombatTarget)
				{
					if (EnemyMovementStatus == EEnemyMovementStatus::EMS_Attacking)
					{
						AnimInstance->Montage_Stop(0.1f, CombatMontage);
					}
				}
				if (!Main->IsDead()) 
				{
					MoveToTarget(Main);
				}
			}
		}

		// When enemy doesn't have any combat target and enemy doesn't follow player,  Ai(npc) attacks enemy
		if (!CombatTarget && AgroTarget != Main && MagicAttack->GetCaster() == ECasterType::NPC)
		{
			if(MagicAttack->GetInstigator() != nullptr)
			{
				AStudent* NPC = Cast<AStudent>(MagicAttack->GetInstigator()->GetPawn());
				if (NPC && CombatTargets.Contains(NPC))
				{
					CombatTarget = NPC;
				}
				else
				{
					MoveToTarget(NPC);
				}
			}
		}
	}
	
	if (CombatTarget)
	{
		AttackEnd();
	}
}

// �̵� �� ���� ���� �ƹ��� ���� �� �ٽ� �ʱ� ��ġ�� �̵�
void AEnemy::MoveToInitialLocation()
{
	if (AIController && !IsDead())
	{
		GetWorldTimerManager().SetTimer(CheckLocationTimer, this, &AEnemy::CheckCurrentLocation, 0.5f);
		AIController->MoveToLocation(InitialLocation);
	}
}

void AEnemy::CheckCurrentLocation()
{
	if (!IsDead())
	{
		if (AgroTarget)
		{
			AIController->StopMovement();
			GetWorldTimerManager().ClearTimer(CheckLocationTimer);
			return;
		}

		ReturnCounter += 1;
		if (ReturnCounter >= MAX_RETURN_COUNT) 
		{
			SetActorLocation(InitialLocation);
		}

		float distance = (GetActorLocation() - InitialLocation).Size();
		if (distance <= 70.f)
		{
			if (AIController)
			{
				AIController->StopMovement();
				EnemyMovementStatus = EEnemyMovementStatus::EMS_Idle;
				ReturnCounter = 0;
				SetActorRotation(InitialRotation);
				GetWorldTimerManager().ClearTimer(CheckLocationTimer);
			}
		}
		else
		{
			GetWorldTimerManager().SetTimer(CheckLocationTimer, this, &AEnemy::CheckCurrentLocation, 0.5f);
		}
	}
	else
	{
		GetWorldTimerManager().ClearTimer(CheckLocationTimer);
	}
}

void AEnemy::CheckChaseState()
{
	if (EnemyMovementStatus == EEnemyMovementStatus::EMS_MoveToTarget)
	{
		MoveFailCounter += 1;
		if (MoveFailCounter > MAX_CHASE_COUNT)
		{
			if (CombatTargets.Num() != 0)
			{
				AgroTarget = nullptr;
				CombatTarget = CombatTargets[0];
				Attack();
			}
			else if(AgroTargets.Num() != 0)
			{
				AgroTarget = nullptr;
				for (int32 i = 0; i < AgroTargets.Num(); i++)
				{
					if (AgroTargets[i] != AgroTarget)
					{
						MoveFailCounter = 0;
						GetWorldTimerManager().ClearTimer(CheckChaseStateTimer);
						MoveToTarget(AgroTargets[i]);
						break;
					}
				}
			}
		}
		GetWorldTimerManager().SetTimer(CheckChaseStateTimer, this, &AEnemy::CheckChaseState, 0.5f);
	}
	else
	{
		MoveFailCounter = 0;
		GetWorldTimerManager().ClearTimer(CheckChaseStateTimer);
	}
}
