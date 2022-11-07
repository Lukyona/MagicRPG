// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "Components/SphereComponent.h"
#include "AIController.h"
#include "Main.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/BoxComponent.h"
#include "Animation/AnimInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "MagicSkill.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Components/CapsuleComponent.h"
#include "Sound/SoundCue.h"
#include "MainPlayerController.h"
#include "Engine/EngineTypes.h"
#include "YaroCharacter.h"

// Sets default values
AEnemy::AEnemy()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AgroSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AgroSphere"));
	AgroSphere->SetupAttachment(GetRootComponent());
	AgroSphere->InitSphereRadius(500.f);
	AgroSphere->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
	AgroSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	AgroSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Ignore);


	CombatSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatSphere"));
	CombatSphere->SetupAttachment(GetRootComponent());
	CombatSphere->InitSphereRadius(170.f);
	CombatSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	CombatSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Ignore);
	CombatSphere->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);


	CombatCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("CombatCollision"));
	CombatCollision->SetupAttachment(GetMesh(), FName("EnemySocket_1"));

	//여기서 두번째 소켓이 있는지 확인해봤자 블루프린트에선 적용이 안 된다.
	CombatCollision2 = CreateDefaultSubobject<UBoxComponent>(TEXT("CombatCollision2"));
	CombatCollision2->SetupAttachment(GetMesh(), FName("EnemySocket_2"));

	bOverlappingCombatSphere = false;

	//This is the default value, each enemy has different health.
	Health = 100.f;
	MaxHealth = 100.f;

	EnemyMovementStatus = EEnemyMovementStatus::EMS_Idle;

	DeathDelay = 3.f;

	InterpSpeed = 10.f; //(플레이어 바라볼 때)회전 속도
	bInterpToTarget = false;

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	AIController = Cast<AAIController>(GetController());

	AgroSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::AgroSphereOnOverlapBegin);
	AgroSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::AgroSphereOnOverlapEnd);

	CombatSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::CombatSphereOnOverlapBegin);
	CombatSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::CombatSphereOnOverlapEnd);

	CombatCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::CombatOnOverlapBegin);
	CombatCollision->OnComponentEndOverlap.AddDynamic(this, &AEnemy::CombatOnOverlapEnd);

	CombatCollision2->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::CombatOnOverlapBegin);
	CombatCollision2->OnComponentEndOverlap.AddDynamic(this, &AEnemy::CombatOnOverlapEnd);


	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CombatCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CombatCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CombatCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	CombatCollision2->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CombatCollision2->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CombatCollision2->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CombatCollision2->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);


	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	InitialLocation = GetActorLocation(); // Set initial enemy location
	InitialRotation = GetActorRotation();

	Main = Cast<AMain>(UGameplayStatics::GetPlayerCharacter(this, 0));

    AnimInstance = GetMesh()->GetAnimInstance();


}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    if (!Main) Main = Cast<AMain>(UGameplayStatics::GetPlayerCharacter(this, 0));

	if (bInterpToTarget && bOverlappingCombatSphere && CombatTarget)
	{
		FRotator LookAtYaw = GetLookAtRotationYaw(CombatTarget->GetActorLocation());
		FRotator InterpRotation = FMath::RInterpTo(GetActorRotation(), LookAtYaw, DeltaTime, InterpSpeed); //smooth transition
		SetActorRotation(InterpRotation);
	}
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
		if (AgroSound && AgroTargets.Num() == 0) UGameplayStatics::PlaySound2D(this, AgroSound);

		ACharacter* target = Cast<ACharacter>(OtherActor);

		if (!target) return;
		UE_LOG(LogTemp, Log, TEXT("AgroSphereOnOverlapBegin %s"), *target->GetName());

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

		for (int i = 0; i < AgroTargets.Num(); i++)
		{
			if (target == AgroTargets[i]) return;
		}
		AgroTargets.Add(target); // Add to target list
	}
}

void AEnemy::AgroSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (auto actor = Cast<AEnemy>(OtherActor)) return; // 오버랩된 게 Enemy라면 코드 실행X

	if (OtherActor && Alive())
	{
		ACharacter* target = Cast<ACharacter>(OtherActor);

		if (target)
		{
			for (int i = 0; i < AgroTargets.Num(); i++) // Remove target in the target list
			{
				if (target == AgroTargets[i])
				{
					AgroTargets.Remove(target);
				}
			}
            //UE_LOG(LogTemp, Log, TEXT("AgroSphereOnOverlapEnd %s"), *target->GetName());

			if (AgroTarget != Main) //npc를 쫓아가던 중이면(인식 범위 나간 것도 npc)
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
		ACharacter* target = Cast<ACharacter>(OtherActor);

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
		ACharacter* target = Cast<ACharacter>(OtherActor);
		//UE_LOG(LogTemp, Log, TEXT("CombatSphere On Overlap End"));
		UE_LOG(LogTemp, Log, TEXT("CombatSphereOnOverlapEnd %s"), *OtherActor->GetName());

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

		if(target == Main && Main->MovementStatus != EMovementStatus::EMS_Dead) // 플레이어가 살아있는 상태로 전투범위를 나감 
		{
			//UE_LOG(LogTemp, Log, TEXT("main"));
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

void AEnemy::MoveToTarget(ACharacter* Target)
{
	if (AIController && Alive())
	{
		if (Target == Main && Main->MovementStatus == EMovementStatus::EMS_Dead)
		{
			MoveToLocation();
			return;
		}
		SetEnemyMovementStatus(EEnemyMovementStatus::EMS_MoveToTarget);

		GetWorldTimerManager().ClearTimer(CheckHandle);
		//UE_LOG(LogTemp, Log, TEXT("move to target"));

		AgroTarget = Target;
		FAIMoveRequest MoveRequest;
		MoveRequest.SetGoalActor(Target);
		MoveRequest.SetAcceptanceRadius(15.0f);

		FNavPathSharedPtr NavPath;

		AIController->MoveTo(MoveRequest, &NavPath);

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
		ACharacter* target;
		if (OtherActor == Main)
		{
			target = Main;
		}
		else
		{
			target = Cast<ACharacter>(OtherActor);
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
	CombatCollision2->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AEnemy::DeactivateCollision()
{
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CombatCollision2->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemy::Attack()
{
	if (Alive())
	{
		if (!bAttacking && CombatTarget)
		{
			if (CombatTarget == Main && Main->MovementStatus == EMovementStatus::EMS_Dead)
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
			//UE_LOG(LogTemp, Log, TEXT("attack"));

			if (AnimInstance)
			{
				AnimInstance->Montage_Play(CombatMontage);

				if (this->GetName().Contains("Grux") || this->GetName().Contains("LizardMan") || this->GetName().Contains("LizardMan")
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
	}
}

void AEnemy::AttackEnd()
{
	bAttacking = false;
	SetInterpToTarget(true);
	//UE_LOG(LogTemp, Log, TEXT("attack end"));
	if (bOverlappingCombatSphere && !AgroTarget)
	{
		if (CombatTarget)
		{
			if (CombatTarget == Main)
			{
				if (Main->MovementStatus == EMovementStatus::EMS_Dead)
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
				/*if (npc->MovementStatus == EMovementStatus::EMS_Dead)
				{

				}*/
			}
		}

		GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, AttackDelay);
	}
}

float AEnemy::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
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

	MagicAttack = Cast<AMagicSkill>(DamageCauser);
	//UE_LOG(LogTemp, Log, TEXT("attck %s"), *MagicAttack->GetName());

	return DamageAmount;
}

void AEnemy::Die()
{
	if (EnemyMovementStatus != EEnemyMovementStatus::EMS_Dead)
	{
		Main->Enemies.Add(Name);
		if (AIController) AIController->StopMovement();
		SetInterpToTarget(false);

		if (DeathSound) UGameplayStatics::PlaySound2D(this, DeathSound);

		if (AnimInstance)
		{
			AnimInstance->Montage_Play(CombatMontage);
			AnimInstance->Montage_JumpToSection(FName("Death"), CombatMontage);
		}
		SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Dead);

		if (bAttackFromPlayer) Main->GetExp(EnemyExp);
	}
}

void AEnemy::DeathEnd()
{
	GetMesh()->bPauseAnims = true;
	GetMesh()->bNoSkeletonUpdate = true;

	if (UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("first") && Main->Enemies.Num() == 9) // 첫번째 던전 클리어했는지
	{
		Main->SaveGame();
		if (Main->Momo->TeleportCount == 0) GetWorldTimerManager().ClearTimer(Main->Momo->MoveTimer);
		if (Main->Luko->TeleportCount == 0) GetWorldTimerManager().ClearTimer(Main->Luko->MoveTimer);

		Main->MainPlayerController->DisplayDialogueUI();
	}

	GetWorldTimerManager().SetTimer(DeathTimer, this, &AEnemy::Disappear, DeathDelay);
}

bool AEnemy::Alive()
{
	return GetEnemyMovementStatus() != EEnemyMovementStatus::EMS_Dead;
}

void AEnemy::Disappear()
{
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CombatCollision2->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AgroSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CombatSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Destroy();
}

void AEnemy::HitGround() //Golem's third skill
{
	if (SkillSound) UGameplayStatics::PlaySound2D(this, SkillSound);

	if (CombatTarget)
	{
		UGameplayStatics::ApplyDamage(CombatTarget, Damage, AIController, this, DamageTypeClass);
	}
}

void AEnemy::HitEnd()
{
	//UE_LOG(LogTemp, Log, TEXT("hit end"));

	if (!Main) Main = Cast<AMain>(UGameplayStatics::GetPlayerCharacter(this, 0));

	if (AgroTarget) // 누군가를 쫓아가고 있었으면 다시 쫓아가기
	{
		MoveToTarget(AgroTarget);
		//UE_LOG(LogTemp, Log, TEXT("move to target again"));
	}
	

	int index = MagicAttack->index;
	if (!CombatTarget && AgroSound) UGameplayStatics::PlaySound2D(this, AgroSound);


	/* When enemy doesn't have combat target, player attacks enemy
	or when enemy's combat target is not player and player attacks enemy.
	At this time, enemy must sets player as a combat target.
	*/
	if (index == 0)
	{
		if ((CombatTarget == nullptr || CombatTarget != Main))
		{
			if (CombatTarget)
			{
				if (EnemyMovementStatus == EEnemyMovementStatus::EMS_Attacking)
				{
					AnimInstance->Montage_Stop(0.1f, CombatMontage);
				}
			}
			if (Main->MovementStatus != EMovementStatus::EMS_Dead) MoveToTarget(Main);
		}
	}

	// When enemy doesn't have any combat target and enemy doesn't follow player,  Ai(npc) attacks enemy
	if (!CombatTarget && AgroTarget != Main && index != 0)
	{
		ACharacter* npc = MagicAttack->Caster;
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

		SetEnemyMovementStatus(EEnemyMovementStatus::EMS_MoveToTarget);
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
		//UE_LOG(LogTemp, Log, TEXT("%f"), distance);

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
			/*if (AgroTargets.Num() == 0)
			{
				AIController->StopMovement();
				GetWorldTimerManager().ClearTimer(CheckHandle);
			}*/
		}
	}
	else
	{
		GetWorldTimerManager().ClearTimer(CheckHandle);
	}
}