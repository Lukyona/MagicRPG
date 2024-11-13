// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "Components/SphereComponent.h"
#include "AIController.h"
#include "Yaro/Character/Main.h"
#include "Yaro/Character/YaroCharacter.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/BoxComponent.h"
#include "Animation/AnimInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Yaro/MagicSkill.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Components/CapsuleComponent.h"
#include "Sound/SoundCue.h"
#include "Yaro/System/MainPlayerController.h"
#include "Yaro/System/GameManager.h"
#include "Yaro/System/DialogueManager.h"
#include "Yaro/System/NPCManager.h"
#include "Engine/EngineTypes.h"

// Sets default values
AEnemy::AEnemy()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bOverlappingCombatSphere = false;

	EnemyMovementStatus = EEnemyMovementStatus::EMS_Idle;

	DeathDelay = 3.f;

	InterpSpeed = 10.f; //(캐릭터 바라볼 때)회전 속도
	bInterpToTarget = false;

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	GameManager = Cast<UGameManager>(GetWorld()->GetGameInstance());
	if (GameManager) NPCManager = GameManager->GetNPCManager();

	AIController = Cast<AAIController>(GetController());

	SetMain();

	AnimInstance = GetMesh()->GetAnimInstance();

	AgroSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::AgroSphereOnOverlapBegin);
	AgroSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::AgroSphereOnOverlapEnd);

	CombatSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::CombatSphereOnOverlapBegin);
	CombatSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::CombatSphereOnOverlapEnd);

	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	InitialLocation = GetActorLocation(); 
	InitialRotation = GetActorRotation();

}

void AEnemy::SetAgroSphere(float value)
{
	AgroSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AgroSphere"));
	AgroSphere->SetupAttachment(GetRootComponent());
	AgroSphere->InitSphereRadius(value);
	AgroSphere->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
	AgroSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	AgroSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Ignore);
}

void AEnemy::SetCombatSphere(float value)
{
	CombatSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatSphere"));
	CombatSphere->SetupAttachment(GetRootComponent());
	CombatSphere->InitSphereRadius(value);
	CombatSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	CombatSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Ignore);
	CombatSphere->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
}

void AEnemy::EnableFirstWeaponCollision()
{
	CombatCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::CombatOnOverlapBegin);
	CombatCollision->OnComponentEndOverlap.AddDynamic(this, &AEnemy::CombatOnOverlapEnd);

	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CombatCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CombatCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CombatCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
}

void AEnemy::EnableSecondWeaponCollision()
{
	CombatCollision2->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::CombatOnOverlapBegin);
	CombatCollision2->OnComponentEndOverlap.AddDynamic(this, &AEnemy::CombatOnOverlapEnd);

	CombatCollision2->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CombatCollision2->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CombatCollision2->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CombatCollision2->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
}


// Called every frame
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

void AEnemy::SetInterpToTarget(bool Interp)
{
	bInterpToTarget = Interp;
}

FRotator AEnemy::GetLookAtRotationYaw(FVector Target)
{
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Target);
	FRotator LookAtRotationYaw(0.f, LookAtRotation.Yaw, 0.f);
	return LookAtRotationYaw;
}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AEnemy::AgroSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (auto actor = Cast<AEnemy>(OtherActor)) return; // 오버랩된 게 Enemy라면 코드 실행X

	if (OtherActor && Alive()) //전투타겟이 없을 때 NPC에게도 유효, 전투타겟이 있어도 플레이어에게 반응
	{
		if (AgroSound && AgroTargets.Num() == 0) // 인식 범위에 아무도 없었으면 인식 사운드 재생
		{
			if(!UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("boss"))
				UGameplayStatics::PlaySound2D(this, AgroSound);
		}

		AStudent* target = Cast<AStudent>(OtherActor);

		if (!target) return;
		//UE_LOG(LogTemp, Log, TEXT("AgroSphereOnOverlapBegin %s"), *target->GetName());

		for (int i = 0; i < AgroTargets.Num(); i++)
		{
			if (target == AgroTargets[i]) return;
		}
		AgroTargets.Add(target); // Add to target list

		if (target == Main)
		{
			MoveToTarget(Main);
		}
		else // npc가 인식 범위 내에 들어옴
		{	
			if (!CombatTarget && AgroTarget != Main) //다른 npc와 전투 중이 아니어야하고, 플레이어를 따라가는 중이 아니어야함
			{
				MoveToTarget(target);
			}				
		}
	}
}

void AEnemy::AgroSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (auto actor = Cast<AEnemy>(OtherActor)) return; // 오버랩된 게 Enemy라면 코드 실행X

	if (OtherActor && Alive())
	{
		AStudent* target = Cast<AStudent>(OtherActor);

		if (target)
		{
			if (!UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("boss"))
			{
				for (int i = 0; i < AgroTargets.Num(); i++) // Remove target in the target list
				{
					if (target == AgroTargets[i])
					{
						AgroTargets.Remove(target);
					}
				}
			}
			
            //UE_LOG(LogTemp, Log, TEXT("AgroSphereOnOverlapEnd %s"), *target->GetName());

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
				if (target == Main) //플레이어가 인식 범위 밖으로 나감
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

	if (OtherActor && Alive()) //전투타겟이 없을 때 NPC에게도 유효, 전투타겟이 있어도 플레이어에게 반응
	{
		AStudent* target = Cast<AStudent>(OtherActor);

		if (target)
		{			
			//UE_LOG(LogTemp, Log, TEXT("CombatSphereOnOverlapBegin %s"), *OtherActor->GetName());

			for (int i = 0; i < CombatTargets.Num(); i++)
			{
				if (target == CombatTargets[i]) return;
			}
			CombatTargets.Add(target); // Add to target list

			if (CombatTarget && target != Main) return; //전투타겟이 있는데, 전투범위에 다른 npc가 들어오면 리턴

			if ((AgroTarget == Main && target == Main) || AgroTarget != Main) //npc를 쫓아가던 중(누군가 전투범위에 들어옴) 혹은 플레이어를 쫓던 중에 전투범위에 플레이어가 들어왔을 때
			{
				AgroTarget = nullptr;
				CombatTarget = target;
				bOverlappingCombatSphere = true;
				Attack();		
			}
		}
	}
}

void AEnemy::CombatSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (auto actor = Cast<AEnemy>(OtherActor)) return; // 오버랩된 게 Enemy라면 코드 실행X

	if (OtherActor && Alive())
	{
		AStudent* target = Cast<AStudent>(OtherActor);
		////UE_LOG(LogTemp, Log, TEXT("CombatSphere On Overlap End"));
		//UE_LOG(LogTemp, Log, TEXT("CombatSphereOnOverlapEnd %s"), *OtherActor->GetName());

		if (!target) return;

		AnimInstance->Montage_Stop(0.1f, CombatMontage);

		
		if (target == CombatTarget)
		{
			CombatTarget = nullptr; //전투타겟이 전투범위를 나감
			if (bAttacking) AttackEnd();
		}

		for (int i = 0; i < CombatTargets.Num(); i++) // Remove target in the target list
		{
			if (target == CombatTargets[i])
			{
				CombatTargets.Remove(target);
			}
		}

		if(target == Main && Main->GetMovementStatus() != EMovementStatus::EMS_Dead) // 플레이어가 살아있는 상태로 전투범위를 나감 
		{
			//UE_LOG(LogTemp, Log, TEXT("main end move to main"));
			MoveToTarget(target);
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

void AEnemy::MoveToTarget(AStudent* Target)
{
	if (AIController && Alive())
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
		//UE_LOG(LogTemp, Log, TEXT("move to target %s"), *this->GetName());

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

void AEnemy::CombatOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (auto actor = Cast<AEnemy>(OtherActor)) return; // 오버랩된 게 Enemy라면 코드 실행X

	if (OtherActor)
	{
		AStudent* target;
		if (OtherActor == Main)
		{
			target = Main;
		}
		else
		{
			target = Cast<AStudent>(OtherActor);
		}
		if (target)
		{
			if (DamageTypeClass)
			{
				UGameplayStatics::ApplyDamage(target, Damage, AIController, this, DamageTypeClass);
			}
		}
	}
}

void AEnemy::CombatOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}

void AEnemy::ActivateCollision()
{
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AEnemy::DeactivateCollision()
{
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemy::ActivateCollisions()
{
	ActivateCollision();
	CombatCollision2->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AEnemy::DeactivateCollisions()
{
	DeactivateCollision();
	CombatCollision2->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemy::Attack()
{
	if (Alive())
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

				if (this->GetName().Contains("Grux") || this->GetName().Contains("LizardMan")
					|| this->GetName().Contains("Archer") || this->GetName().Contains("Spider"))
				{
					AnimInstance->Montage_JumpToSection(FName("Attack"), CombatMontage);
				}
				else
				{
					int num = 0;

					if(this->GetName().Contains("Dino") || this->GetName().Contains("Lizard_BP"))
                        num = FMath::RandRange(1, 2);
					else
						num = FMath::RandRange(1, 3);

					switch (num)
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
	//UE_LOG(LogTemp, Log, TEXT("attack end %s"), *this->GetName());
	if (bOverlappingCombatSphere && !AgroTarget)
	{
		if (CombatTarget)
		{
			if (CombatTarget == Main)
			{
				if (Main->GetMovementStatus() == EMovementStatus::EMS_Dead)
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
			else
			{
				AYaroCharacter* npc = Cast<AYaroCharacter>(CombatTarget);
			}
		}

		GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, AttackDelay);
	}
}

float AEnemy::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	MagicAttack = Cast<AMagicSkill>(DamageCauser);

	int index = MagicAttack->index;
	if(index == 0) 
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

	////UE_LOG(LogTemp, Log, TEXT("attck %s"), *MagicAttack->GetName());
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
	
	uint32 DeadEnemiesNum = GameManager->GetDeadEnemies().Num();
	if (UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("first") && DeadEnemiesNum == 9) // 첫번째 던전 클리어했는지, 골렘 이후 대화가 정상
	{
		GameManager->SaveGame();
		if (NPCManager->GetNPC("Momo")->GetTeleportCount() == 0) GetWorldTimerManager().ClearTimer(NPCManager->GetNPC("Momo")->GetMoveTimer());
		if (NPCManager->GetNPC("Luko")->GetTeleportCount() == 0) GetWorldTimerManager().ClearTimer(NPCManager->GetNPC("Luko")->GetMoveTimer());

		GameManager->GetDialogueManager()->DisplayDialogueUI();
	}

	if (UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("second"))
	{
		if ((EnemyType == EEnemyType::Spider && GameManager->GetDeadEnemies()[EnemyType] == 5) || (EnemyType == EEnemyType::LittleMonster && GameManager->GetDeadEnemies()[EnemyType] == 8))
		{
			NPCManager->AllNpcStopFollowPlayer();
			GameManager->SaveGame();
			GameManager->GetDialogueManager()->DisplayDialogueUI();
		}
	}

	if ((UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("boss") && DeadEnemiesNum == 5))
	{
		GameManager->SaveGame();
		GameManager->GetDialogueManager()->DisplayDialogueUI();
	}
	
	GetWorldTimerManager().SetTimer(DeathTimer, this, &AEnemy::Disappear, DeathDelay);
}

bool AEnemy::Alive()
{
	return GetEnemyMovementStatus() != EEnemyMovementStatus::EMS_Dead;
}

void AEnemy::CombatCollisionDisabled()
{
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (hasSecondCollision)
		CombatCollision2->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemy::SphereCollisionDisabled()
{
	AgroSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CombatSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemy::Disappear()
{
	CombatCollisionDisabled();

	SphereCollisionDisabled();
	
	Destroy();
}

void AEnemy::HitEnd()
{
	//UE_LOG(LogTemp, Log, TEXT("hit end %s"), *this->GetName());

	if (!Main) Main = Cast<AMain>(UGameplayStatics::GetPlayerCharacter(this, 0));

	if (AgroTarget) // 누군가를 쫓아가고 있었으면 다시 쫓아가기
	{
		MoveToTarget(AgroTarget);
		//UE_LOG(LogTemp, Log, TEXT("move to target again %s"), *this->GetName());
	}


	int index = MagicAttack->index;
	if (!CombatTarget && AgroSound) UGameplayStatics::PlaySound2D(this, AgroSound);


	/* When enemy doesn't have combat target, player attacks enemy
	or when enemy's combat target is not player and player attacks enemy.
	At this time, enemy must sets player as a combat target.
	*/
	if (index == 0)
	{
		bAttackFromPlayer = true;
		if ((CombatTarget == nullptr || CombatTarget != Main))
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
	if (!CombatTarget && AgroTarget != Main && index != 0)
	{
		AStudent* npc = MagicAttack->Caster;
		if (CombatTargets.Contains(npc)) 
		{
			CombatTarget = npc;
			bOverlappingCombatSphere = true;
		}
		else
		{
			MoveToTarget(npc);
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
	if (AIController && Alive())
	{
		GetWorldTimerManager().SetTimer(CheckHandle, this, &AEnemy::CheckLocation, 0.5f);

		AIController->MoveToLocation(InitialLocation);
	}
}

void AEnemy::CheckLocation()
{
	if (Alive())
	{
		if (AgroTarget)
		{
			AIController->StopMovement();
			GetWorldTimerManager().ClearTimer(CheckHandle);
		}

		Count += 1;
		if (Count >= 20) SetActorLocation(InitialLocation);
		float distance = (GetActorLocation() - InitialLocation).Size();
		////UE_LOG(LogTemp, Log, TEXT("%f"), distance);

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

void AEnemy::CreateFirstWeaponCollision()
{
	CombatCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("CombatCollision"));
	CombatCollision->SetupAttachment(GetMesh(), FName("EnemySocket_1"));
}

void AEnemy::CreateSecondWeaponCollision()
{
	//무기 콜리전 2개인 적만 적용
	CombatCollision2 = CreateDefaultSubobject<UBoxComponent>(TEXT("CombatCollision2"));
	CombatCollision2->SetupAttachment(GetMesh(), FName("EnemySocket_2"));

	hasSecondCollision = true;
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
				//UE_LOG(LogTemp, Log, TEXT("change agro"));

				AgroTarget = nullptr;
				for (int i = 0; i < AgroTargets.Num(); i++)
				{
					if (AgroTargets[i] != Main)
					{
						MoveToTarget(AgroTargets[i]);
						//UE_LOG(LogTemp, Log, TEXT("cachangehnge agro and move"));

						break;
					}
				}
			}
		}
		//UE_LOG(LogTemp, Log, TEXT("movingcount %d"), MovingCount);

		GetWorldTimerManager().SetTimer(MovingTimer, this, &AEnemy::MovingNow, 0.5f);
	}
	else
	{
		//UE_LOG(LogTemp, Log, TEXT("movingcount reset "));

		MovingCount = 0;
		GetWorldTimerManager().ClearTimer(MovingTimer);
	}
}