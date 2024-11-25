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

// Sets default values
AEnemy::AEnemy()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	EnemyMovementStatus = EEnemyMovementStatus::EMS_Idle;

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	GameManager = Cast<UGameManager>(GetWorld()->GetGameInstance());
	if (GameManager) NPCManager = GameManager->GetNPCManager();

	AIController = Cast<AAIController>(GetController());
	AnimInstance = GetMesh()->GetAnimInstance();

	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	InitialLocation = GetActorLocation(); 
	InitialRotation = GetActorRotation();

	Health = MaxHealth;

	BindSphereComponentEvents();
	if(!bIsRangedAttacker)
		BindWeaponCollisionEvents();

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

void AEnemy::SetAgroSphere(float Radius)
{
	AgroSphere = CreateSphereComponent("AgroSphere", Radius);
}

void AEnemy::SetCombatSphere(float Radius)
{
	CombatSphere = CreateSphereComponent("CombatSphere", Radius);
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
		CreateCollision("CombatCollision2", "EnemySocket_2");
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
	SetAgroSphere(AgroSphereRadius);
	SetCombatSphere(CombatSphereRadius);
	if(!bIsRangedAttacker)
		CreateWeaponCollisions();
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    if (!Main) SetMain();

	if (bInterpToTarget && bOverlappingCombatSphere && CombatTarget)
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

void AEnemy::AgroSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (auto actor = Cast<AEnemy>(OtherActor)) return; // 오버랩된 게 Enemy라면 코드 실행X

	if (OtherActor && !IsDead()) //전투타겟이 없을 때 NPC에게도 유효, 전투타겟이 있어도 플레이어에게 반응
	{
		if (AgroSound && AgroTargets.Num() == 0) // 인식 범위에 아무도 없었으면 인식 사운드 재생
		{
			if(!UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("boss"))
				UGameplayStatics::PlaySound2D(this, AgroSound);
		}

		AStudent* Target = Cast<AStudent>(OtherActor);

		if (!Target) return;
		if (AgroTargets.Contains(Target)) return;

		AgroTargets.Add(Target); // Add to target list

		if (Target == Main)
		{
			MoveToTarget(Main);
		}
		else // npc가 인식 범위 내에 들어옴
		{	
			if (!CombatTarget && AgroTarget != Main) //다른 npc와 전투 중이 아니어야하고, 플레이어를 따라가는 중이 아니어야함
			{
				MoveToTarget(Target);
			}				
		}
	}
}

void AEnemy::AgroSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (auto actor = Cast<AEnemy>(OtherActor)) return; // 오버랩된 게 Enemy라면 코드 실행X

	if (OtherActor && !IsDead())
	{
		AStudent* Target = Cast<AStudent>(OtherActor);

		if (Target)
		{
			if (!UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("boss"))
			{
				if(AgroTargets.Contains(Target))
					AgroTargets.Remove(Target);
			}
			
			if (AgroTarget != nullptr && AgroTarget != Main) //npc를 쫓아가던 중이면(인식 범위 나간 것도 npc)
			{
				AgroTarget = nullptr;
				if (CombatTargets.Num() != 0) //전투 범위에 다른 누군가가(npc) 있으면
				{
					bOverlappingCombatSphere = true;
					CombatTarget = CombatTargets[0];
					if (bAttacking) AttackEnd();
				}
				else if (AgroTargets.Num() != 0) //전투범위에는 아무도 없지만 인식범위에 누군가가 있으면
				{
					MoveToTarget(AgroTargets[0]);			
				}
			}
			else //플레이어를 쫓아가던 중
			{
				if (Target == Main) //플레이어가 인식 범위 밖으로 나감
				{
					AgroTarget = nullptr;
				}
			}

			if (AgroTargets.Num() == 0) // no one's in agrosphere
			{
				AIController->StopMovement();
				MoveToLocation();
			}
		}
	}
}

void AEnemy::CombatSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (auto actor = Cast<AEnemy>(OtherActor)) return; // 오버랩된 게 Enemy라면 코드 실행X

	if (OtherActor && !IsDead()) //전투타겟이 없을 때 NPC에게도 유효, 전투타겟이 있어도 플레이어에게 반응
	{
		AStudent* Target = Cast<AStudent>(OtherActor);

		if (Target)
		{			
			if (CombatTargets.Contains(Target)) return;

			CombatTargets.Add(Target); // Add to target list

			if (CombatTarget && Target != Main) return; //전투타겟이 있는데, 전투범위에 다른 npc가 들어오면 리턴

			if ((AgroTarget == Main && Target == Main) || AgroTarget != Main) //npc를 쫓아가던 중(누군가 전투범위에 들어옴) 혹은 플레이어를 쫓던 중에 전투범위에 플레이어가 들어왔을 때
			{
				AgroTarget = nullptr;
				CombatTarget = Target;
				bOverlappingCombatSphere = true;
				Attack();		
			}
		}
	}
}

void AEnemy::CombatSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (auto actor = Cast<AEnemy>(OtherActor)) return; // 오버랩된 게 Enemy라면 코드 실행X

	if (OtherActor && !IsDead())
	{
		AStudent* Target = Cast<AStudent>(OtherActor);

		if (!Target) return;

		AnimInstance->Montage_Stop(0.1f, CombatMontage);
		
		if (Target == CombatTarget)
		{
			CombatTarget = nullptr; //전투타겟이 전투범위를 나감
			if (bAttacking) AttackEnd();
		}

		if(CombatTargets.Contains(Target))
			CombatTargets.Remove(Target);

		if(Target == Main && Main->GetMovementStatus() != EMovementStatus::EMS_Dead) // 플레이어가 살아있는 상태로 전투범위를 나감 
		{
			MoveToTarget(Target);
		}

		if (CombatTargets.Num() == 0) // no one's in Combatsphere
		{
			bOverlappingCombatSphere = false;

			if (AgroTarget != Main) // 플레이어를 쫓아가는 중이 아니라면
			{
				if (AgroTargets.Num() != 0) // 인식범위 내에 다른 누군가가 있으면
				{
					MoveToTarget(AgroTargets[0]);
				}
			}
		}
		else //전투범위 내에 다른 누군가가 있으면
		{		
			if (AgroTarget != Main) //플레이어를 쫓아가는 중이 아니면
			{
				CombatTarget = CombatTargets[0];
				bOverlappingCombatSphere = true;
				Attack();
			}
		}	
	}
}

void AEnemy::WeaponCollisionOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (auto actor = Cast<AEnemy>(OtherActor)) return; // 오버랩된 게 Enemy라면 코드 실행X

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
		CombatCollision2->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AEnemy::DeactivateWeaponCollisions()
{
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (bHasSecondCollision)
		CombatCollision2->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemy::MoveToTarget(AStudent* Target)
{
	if (AIController && !IsDead())
	{
		if (Target == Main && Main->GetMovementStatus() == EMovementStatus::EMS_Dead)
		{
			if (AgroTargets.Num() == 0)
			{
				MoveToLocation();
				return;
			}
		}
		SetEnemyMovementStatus(EEnemyMovementStatus::EMS_MoveToTarget);

		GetWorldTimerManager().ClearTimer(CheckHandle);

		AgroTarget = Target;
		FAIMoveRequest MoveRequest;
		MoveRequest.SetGoalActor(Target);
		MoveRequest.SetAcceptanceRadius(15.0f);
		FNavPathSharedPtr NavPath;
		AIController->MoveTo(MoveRequest, &NavPath);
		MovingNow();
		/** 어그로(인식)범위(구 콜리전)에 충돌했을 때 어떤 경로를 따라서 몬스터가 따라오는지 확인 가능
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
		if (!bAttacking && CombatTarget)
		{
			if (CombatTarget == Main && Main->GetMovementStatus() == EMovementStatus::EMS_Dead)
			{
				AttackEnd();
				return;
			}

			if (AIController)
			{
				AIController->StopMovement();
				SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Attacking);
			}

			bAttacking = true;
			SetInterpToTarget(false);

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
					int AttackNum = 0;

					if(EnemyType == EEnemyType::LittleDino || EnemyType == EEnemyType::Lizard)
						AttackNum = FMath::RandRange(1, 2);
					else
						AttackNum = FMath::RandRange(1, 3);

					switch (AttackNum)
					{
						case 1:
							AnimInstance->Montage_JumpToSection(FName("Attack1"), CombatMontage);
							break;
						case 2:
							AnimInstance->Montage_JumpToSection(FName("Attack2"), CombatMontage);
							break;
						case 3:
							AnimInstance->Montage_JumpToSection(FName("Attack3"), CombatMontage);
							break;
					}
				}
			}
		}
		else
		{
			if(bAttacking) AttackEnd();
		}
	}
}

void AEnemy::AttackEnd()
{
	bAttacking = false;
	SetInterpToTarget(true);

	if (bOverlappingCombatSphere && !AgroTarget)
	{
		if (CombatTarget)
		{
			if (CombatTarget == Main)
			{
				if (Main->IsDead())
				{
					AgroTargets.Remove(Main);
					CombatTargets.Remove(Main);

					if (CombatTargets.Num() == 0) // no one's in Combatsphere
					{
						bOverlappingCombatSphere = false;

						if (AgroTargets.Num() != 0) // 인식범위 내에 다른 누군가가 있으면
						{
							MoveToTarget(AgroTargets[0]);
						}
						
					}
					else //전투범위 내에 다른 누군가가 있으면
					{
						CombatTarget = CombatTargets[0];
						bOverlappingCombatSphere = true;
					}
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
		bAttackFromPlayer = true;
		
	if (Health - DamageAmount <= 0.f) // Decrease Health
	{
		Health = 0.f;
		Die();
	}
	else
	{
		if (AnimInstance && !bAttacking)
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

		if (AIController) AIController->StopMovement();

		SetInterpToTarget(false);

		if (DeathSound && !GameManager->IsSkipping()) UGameplayStatics::PlaySound2D(this, DeathSound);

		if (bAttackFromPlayer) Main->GainExp(EnemyExp);

		if (AnimInstance && !GameManager->IsSkipping())
		{
			AnimInstance->Montage_Play(CombatMontage);
			AnimInstance->Montage_JumpToSection(FName("Death"), CombatMontage);
		}
		SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Dead);
	}
}

void AEnemy::DeathEnd()
{
	GetMesh()->bPauseAnims = true;
	GetMesh()->bNoSkeletonUpdate = true;
	
	if (UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("first")) // 첫번째 던전 클리어했는지
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
	
	if (UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("second")) // 거미 처치 후 혹은 마지막 몬스터 처치 후
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
	if (bHasSecondCollision)
		CombatCollision2->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemy::DisableSphereCollisions()
{
	AgroSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CombatSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemy::HitEnd()
{
	if (!Main) Main = Cast<AMain>(UGameplayStatics::GetPlayerCharacter(this, 0));

	if (AgroTarget) // 누군가를 쫓아가고 있었으면 다시 쫓아가기
	{
		MoveToTarget(AgroTarget);
	}

	if (!CombatTarget && AgroSound) UGameplayStatics::PlaySound2D(this, AgroSound);
	
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
				if (Main->GetMovementStatus() != EMovementStatus::EMS_Dead) MoveToTarget(Main);
			}
		}

		// When enemy doesn't have any combat target and enemy doesn't follow player,  Ai(npc) attacks enemy
		if (!CombatTarget && AgroTarget != Main && MagicAttack->GetCaster() == ECasterType::NPC)
		{
			if(MagicAttack->GetInstigator() != nullptr)
			{
				AStudent* NPC = Cast<AStudent>(MagicAttack->GetInstigator()->GetPawn());
				if (CombatTargets.Contains(NPC))
				{
					CombatTarget = NPC;
					bOverlappingCombatSphere = true;
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

// 이동 후 범위 내에 아무도 없을 때 다시 초기 위치로 이동
void AEnemy::MoveToLocation()
{
	if (AIController && !IsDead())
	{
		GetWorldTimerManager().SetTimer(CheckHandle, this, &AEnemy::CheckLocation, 0.5f);
		AIController->MoveToLocation(InitialLocation);
	}
}

void AEnemy::CheckLocation()
{
	if (!IsDead())
	{
		if (AgroTarget)
		{
			AIController->StopMovement();
			GetWorldTimerManager().ClearTimer(CheckHandle);
		}

		Count += 1;
		if (Count >= 20) SetActorLocation(InitialLocation);
		float distance = (GetActorLocation() - InitialLocation).Size();

		if (distance <= 70.f)
		{
			if (AIController)
			{
				AIController->StopMovement();
				SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Idle);
				Count = 0;
				SetActorRotation(InitialRotation);
				GetWorldTimerManager().ClearTimer(CheckHandle);
			}
		}
		else
		{
			GetWorldTimerManager().SetTimer(CheckHandle, this, &AEnemy::CheckLocation, 0.5f);
		}
	}
	else
	{
		GetWorldTimerManager().ClearTimer(CheckHandle);
	}
}

void AEnemy::MovingNow()
{
	if (AgroTarget == Main && EnemyMovementStatus == EEnemyMovementStatus::EMS_MoveToTarget)
	{
		MovingCount += 1;
		if (MovingCount > 6)
		{
			if (CombatTargets.Num() != 0)
			{
				//UE_LOG(LogTemp, Log, TEXT("change target and attack"));
				AgroTarget = nullptr;
				CombatTarget = CombatTargets[0];
				bOverlappingCombatSphere = true;
				Attack();
			}
			else if(AgroTargets.Num() != 0)
			{
				AgroTarget = nullptr;
				for (int i = 0; i < AgroTargets.Num(); i++)
				{
					if (AgroTargets[i] != Main)
					{
						MoveToTarget(AgroTargets[i]);
						break;
					}
				}
			}
		}
		GetWorldTimerManager().SetTimer(MovingTimer, this, &AEnemy::MovingNow, 0.5f);
	}
	else
	{
		MovingCount = 0;
		GetWorldTimerManager().ClearTimer(MovingTimer);
	}
}