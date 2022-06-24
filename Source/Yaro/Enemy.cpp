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
	CombatCollision->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, FName("EnemySocket_1"));
	
	//여기서 두번째 소켓이 있는지 확인해봤자 블루프린트에선 적용이 안 된다.
	CombatCollision2 = CreateDefaultSubobject<UBoxComponent>(TEXT("CombatCollision2"));
	CombatCollision2->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, FName("EnemySocket_2"));

	bOverlappingCombatSphere = false;

	//This is the default value, each enemy has different health.
	Health = 100.f;
	MaxHealth = 100.f;

	EnemyMovementStatus = EEnemyMovementStatus::EMS_Idle;

	DeathDelay = 3.f;

	bHasValidTarget = false;

	InterpSpeed = 10.f; //(플레이어 바라볼 때)회전 속도
	bInterpToTarget = false;

	AttackDelay = 0.4f; // 공격 텀
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

	Main = Cast<AMain>(UGameplayStatics::GetPlayerCharacter(this, 0));

	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	InitialLocation = GetActorLocation(); // Set initial enemy location
	InitialRotation = GetActorRotation();


	//Main->CurrentEnemyNum += 1;
	//Main->Enemies.Add(this);

}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

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

		if (AIController) AIController->StopMovement();

		if (OtherActor == Main)
		{
			AgroTarget = Main;

			bOverlappingCombatSphere = false;
			if (bAttacking) AttackEnd();
			MoveToTarget(Main);
			CombatTarget = nullptr;

			UE_LOG(LogTemp, Log, TEXT("move to main"));
			for (int i = 0; i < AgroTargets.Num(); i++)
			{
				if (Main == AgroTargets[i]) return;
			}
			AgroTargets.Add(Main); // Add to target list
		}
		else
		{
			ACharacter* target = Cast<ACharacter>(OtherActor); // Npc's in agroshpere
			if (target)
			{
				if (!CombatTarget && AgroTarget != Main)
				{
					AgroTarget = target;
					MoveToTarget(target);
					UE_LOG(LogTemp, Log, TEXT("move to npc"));

				}
				
				for (int i = 0; i < AgroTargets.Num(); i++)
				{
					if (target == AgroTargets[i]) return;
				}
				AgroTargets.Add(target); // Add to target list
			}
		}	

	}
}

void AEnemy::AgroSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (auto actor = Cast<AEnemy>(OtherActor)) return; // 오버랩된 게 Enemy라면 코드 실행X


	if (OtherActor && Alive())
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
			
			if (CombatTarget == target) CombatTarget = nullptr;
			for (int i = 0; i < AgroTargets.Num(); i++) // Remove target in the target list
			{
				if (target == AgroTargets[i])
				{
					AgroTargets.Remove(target);
				}
			}
			if (AgroTarget != Main)
			{
				AgroTarget = nullptr;
				if (CombatTargets.Num() != 0)
				{
					bOverlappingCombatSphere = true;
					bHasValidTarget = true;
					CombatTarget = CombatTargets[0];

					AttackEnd();
	
					UE_LOG(LogTemp, Log, TEXT("set combattarget"));

				}
				else
				{
					if (AgroTargets.Num() != 0)
					{
						AgroTarget = AgroTargets[0];
						MoveToTarget(AgroTargets[0]);
						UE_LOG(LogTemp, Log, TEXT("movet to1"));
					}


				}

			}
			else
			{
				if(target == Main) AgroTarget = nullptr;
			}

			if (AgroTargets.Num() == 0) // no one's in agrosphere
			{
				MoveToLocation();
			}
			else
			{
				

			}
		}
	}
}

void AEnemy::CombatSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (auto actor = Cast<AEnemy>(OtherActor)) return; // 오버랩된 게 Enemy라면 코드 실행X

	if (OtherActor&& Alive()) //전투타겟이 없을 때 NPC에게도 유효, 전투타겟이 있어도 플레이어에게 반응
	{
		ACharacter* target;
		if (OtherActor == Main)
		{
			target = Main;

		}
		else
		{
			target = Cast<ACharacter>(OtherActor);
			//UE_LOG(LogTemp, Log, TEXT("%s, in combatsphere"), *(OtherActor->GetName()));

		}

		if (target)
		{		
			for (int i = 0; i < CombatTargets.Num(); i++)
			{
				if (target == CombatTargets[i]) return;
			}
			CombatTargets.Add(target); // Add to target list
			
			if (CombatTarget && target != Main)
			{
				UE_LOG(LogTemp, Log, TEXT("aaaaaaaaaaaaaaaaa??"));

				return;

			}

			UE_LOG(LogTemp, Log, TEXT("set combattarget"));

			if (AgroTarget == Main)
			{
				if (target == Main)
				{
					AgroTarget = nullptr;
					bHasValidTarget = true;
					CombatTarget = target;
					bOverlappingCombatSphere = true;
					Attack();
				}

			}
			else
			{
				AgroTarget = nullptr;
				bHasValidTarget = true;
				CombatTarget = target;
				bOverlappingCombatSphere = true;
				Attack();
			}


			
			//UE_LOG(LogTemp, Log, TEXT("set combattarget, %s"), *(CombatTarget->GetName()));

		}
	}
	
}

void AEnemy::CombatSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (auto actor = Cast<AEnemy>(OtherActor)) return; // 오버랩된 게 Enemy라면 코드 실행X

	if (OtherActor && Alive())
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
		 
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		AnimInstance->Montage_Stop(0.1f, CombatMontage);

		SetInterpToTarget(false);
		GetWorldTimerManager().ClearTimer(AttackTimer);

		if (target)
		{
			//UE_LOG(LogTemp, Log, TEXT("end combatsphere, %s"), *(CombatTarget->GetName()));

			if (target)
			{
				for (int i = 0; i < CombatTargets.Num(); i++) // Remove target in the target list
				{
					if (target == CombatTargets[i])
					{
						CombatTargets.Remove(target);
					}
				}

				if (!CombatTarget && CombatTargets.Num() == 0) // no one's in Combatsphere
				{
					bOverlappingCombatSphere = false;
					UE_LOG(LogTemp, Log, TEXT("overlap false"));

					if (target == Main)
					{
						//CombatTarget = nullptr;
						UE_LOG(LogTemp, Log, TEXT("main out combatsphere"));

						AgroTarget = Main;
						MoveToTarget(Main);
					}

					if(AgroTarget != Main)
					{
						CombatTarget = nullptr;
						bHasValidTarget = false;
						UE_LOG(LogTemp, Log, TEXT("this"));

						if (AgroTargets.Num() != 0)
						{
							AgroTarget = AgroTargets[0];
							MoveToTarget(AgroTargets[0]);
							UE_LOG(LogTemp, Log, TEXT("this2"));

						}

					}
					else
					{
						UE_LOG(LogTemp, Log, TEXT("this4"));
						MoveToTarget(Main);
					}
			
				}
				else
				{
					if (target == Main) AgroTarget = Main;

					if (AgroTarget != Main && CombatTargets.Num() != 0)
					{

						CombatTarget = CombatTargets[0];
						bOverlappingCombatSphere = true;

						UE_LOG(LogTemp, Log, TEXT("agrotarget no main, %s"), *(CombatTarget->GetName()));

					}
					else if(AgroTarget == Main)
					{
						
						CombatTarget = Main;
						bOverlappingCombatSphere = false;
						MoveToTarget(Main);
						UE_LOG(LogTemp, Log, TEXT("overlap false2"));

						//bHasValidTarget = false;
						UE_LOG(LogTemp, Log, TEXT("this3"));

					}
					
				}
			}


			

			AttackEnd();

		}
	}
}

void AEnemy::MoveToTarget(ACharacter* Target)
{
	SetEnemyMovementStatus(EEnemyMovementStatus::EMS_MoveToTarget);

	if (AIController && Alive())
	{
		GetWorldTimerManager().ClearTimer(CheckHandle);
		UE_LOG(LogTemp, Log, TEXT("move to target, %s"), *(Target->GetName()));

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
	if (Alive() && bHasValidTarget)
	{
		
		if (!bAttacking)
		{
			if (AIController)
			{
				AIController->StopMovement();
				SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Attacking);
			}

			bAttacking = true;
			SetInterpToTarget(false);

			UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
			if (AnimInstance)
			{
				AnimInstance->Montage_Play(CombatMontage);

				if (this->GetName().Contains("Grux"))
				{
					AnimInstance->Montage_JumpToSection(FName("Attack"), CombatMontage);
				}
				else
				{
					int num = FMath::RandRange(1, 3);
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
	//UE_LOG(LogTemp, Log, TEXT("attackend"));
	if (bOverlappingCombatSphere)
	{
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
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			AnimInstance->Montage_Play(CombatMontage);
			AnimInstance->Montage_JumpToSection(FName("Hit"), CombatMontage);
		}
		Health -= DamageAmount;
	}

	MagicAttack = Cast<AMagicSkill>(DamageCauser);


	return DamageAmount;
}


void AEnemy::Die()
{
	if (EnemyMovementStatus != EEnemyMovementStatus::EMS_Dead)
	{
		//Main->CurrentEnemyNum -= 1;
		//Main->Enemies.Remove(this);

		if (AIController) AIController->StopMovement();
		SetInterpToTarget(false);

		if (DeathSound) UGameplayStatics::PlaySound2D(this, DeathSound);

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance)
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
	if(SkillSound) UGameplayStatics::PlaySound2D(this, SkillSound);

	if (CombatTarget)
	{
		UGameplayStatics::ApplyDamage(CombatTarget, Damage, AIController, this, DamageTypeClass);
	}

}

void AEnemy::HitEnd()
{
	if(!Main) Main = Cast<AMain>(UGameplayStatics::GetPlayerCharacter(this, 0));
	int index = MagicAttack->index;
	// When enemy doesn't have any combat target, Ai attacks enemy
	if (!CombatTarget && AgroTarget != Main && index != 0 && Alive())
	{
		if (AgroSound) UGameplayStatics::PlaySound2D(this, AgroSound);

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
		UE_LOG(LogTemp, Log, TEXT(" npc attack, no combattarget"));
	}

	/* When enemy doesn't have combat target, player attacks enemy
	or when enemy's combat target is not player and player attacks enemy.
	At this time, enemy must sets player as a combat target.
	*/
	if ((!CombatTarget || CombatTarget != Main) && index == 0 && Alive())
	{
		if (AgroSound) UGameplayStatics::PlaySound2D(this, AgroSound);

		if (CombatTarget)
		{
			if (EnemyMovementStatus == EEnemyMovementStatus::EMS_Attacking)
			{
				UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
				AnimInstance->Montage_Stop(0.1f, CombatMontage);
				AttackEnd();

				SetInterpToTarget(false);
				GetWorldTimerManager().ClearTimer(AttackTimer);
			}
		}
		MoveToTarget(Main);
	}

	if (Alive() && bOverlappingCombatSphere)
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
				bHasValidTarget = false;
				//UE_LOG(LogTemp, Log, TEXT("clear"));
				SetActorRotation(InitialRotation);
				GetWorldTimerManager().ClearTimer(CheckHandle);

			}
		}
		else
		{
			GetWorldTimerManager().SetTimer(CheckHandle, this, &AEnemy::CheckLocation, 0.5f);
			if (AgroTargets.Num() == 0)
			{
				AIController->StopMovement();
				GetWorldTimerManager().ClearTimer(CheckHandle);
			}
		}
	}
	else
	{
		
			AIController->StopMovement();
			GetWorldTimerManager().ClearTimer(CheckHandle);
		

	}
}