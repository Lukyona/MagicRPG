// Copyright Epic Games, Inc. All Rights Reserved.

#include "YaroCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h" //GetPlayerCharacter
#include "AIController.h"
#include "Main.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Enemy.h"
#include "Components/ArrowComponent.h"
#include "MagicSkill.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Components/CapsuleComponent.h"


//////////////////////////////////////////////////////////////////////////
// AYaroCharacter

AYaroCharacter::AYaroCharacter()
{
	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 60.f;
	GetCharacterMovement()->AirControl = 0.2f;

	Wand = CreateDefaultSubobject<UBoxComponent>(TEXT("WandMesh"));
	Wand->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, FName("RightHandSocket"));

	CombatSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatSphere"));
	CombatSphere->SetupAttachment(GetRootComponent());
	CombatSphere->InitSphereRadius(500.f);
	CombatSphere->SetRelativeLocation(FVector(260.f, 0.f, 0.f));
	CombatSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	CombatSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Ignore);
	CombatSphere->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);

	bOverlappingCombatSphere = false;
	bHasCombatTarget = false;
	targetIndex = 0;

	AttackArrow = CreateAbstractDefaultSubobject<UArrowComponent>(TEXT("AttackArrow"));
	AttackArrow->SetupAttachment(GetRootComponent());
	AttackArrow->SetRelativeLocation(FVector(160.f, 4.f, 26.f));

	InterpSpeed = 15.f;
	bInterpToEnemy = false;

	Pos.Add(FVector(2517.f, 5585.f, 3351.f));
	Pos.Add(FVector(2345.f, 4223.f, 2833.f));
	Pos.Add(FVector(2080.f, 283.f, 2838.f));
	Pos.Add(FVector(1550.f, -1761.f, 2843.f));
	Pos.Add(FVector(1026.f, -1791.f, 2576.f));
	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

void AYaroCharacter::BeginPlay()
{
	Super::BeginPlay();


	AIController = Cast<AAIController>(GetController());
	if (this->GetName().Contains("Momo") || this->GetName().Contains("Luko")) MoveToPlayer();

	CombatSphere->OnComponentBeginOverlap.AddDynamic(this, &AYaroCharacter::CombatSphereOnOverlapBegin);
	CombatSphere->OnComponentEndOverlap.AddDynamic(this, &AYaroCharacter::CombatSphereOnOverlapEnd);

	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

}

void AYaroCharacter::Tick(float DeltaTime)
{
	if (bInterpToEnemy && CombatTarget)
	{
		FRotator LookAtYaw = GetLookAtRotationYaw(CombatTarget->GetActorLocation());
		FRotator InterpRotation = FMath::RInterpTo(GetActorRotation(), LookAtYaw, DeltaTime, InterpSpeed); //smooth transition

		SetActorRotation(InterpRotation);
	}

	if (!Player)
	{
		ACharacter* p = UGameplayStatics::GetPlayerCharacter(this, 0);
		Player = Cast<AMain>(p);
	}

	if (this->GetName().Contains("Vovo") || this->GetName().Contains("Vivi") || this->GetName().Contains("Zizi"))
	{

		if (!canGo && Player && Player->NpcGo)
		{
			canGo = true;
			MoveToLocation();
		}
	}
}


void AYaroCharacter::MoveToPlayer()
{
	FTimerHandle WaitHandle;
	float WaitTime = 1.5f; // 딜레이 타임 설정
	GetWorld()->GetTimerManager().SetTimer(WaitHandle, FTimerDelegate::CreateLambda([&]()
		{
			if (Player && Player->NpcGo && !CombatTarget && !bAttacking && !bOverlappingCombatSphere)
			{
				float distance = GetDistanceTo(Player);

				if (distance >= 500.f) //일정 거리 이상 떨어져있다면 속도 높여 달리기
				{
					if ((this->GetName()).Contains("Momo"))
					{
						GetCharacterMovement()->MaxWalkSpeed = 600.f;
					}
					else if ((this->GetName()).Contains("Zizi") || (this->GetName()).Contains("Vivi"))
					{
						GetCharacterMovement()->MaxWalkSpeed = 500.f;
					}
					else
					{
						GetCharacterMovement()->MaxWalkSpeed = 450.f;
					}
				}
				else //가깝다면 속도 낮춰 걷기
				{
					if ((this->GetName()).Contains("Momo"))
					{
						GetCharacterMovement()->MaxWalkSpeed = 300.f;
					}
					else if ((this->GetName()).Contains("Zizi") || (this->GetName()).Contains("Vivi"))
					{
						GetCharacterMovement()->MaxWalkSpeed = 250.f;
					}
					else
					{
						GetCharacterMovement()->MaxWalkSpeed = 225.f;
					}
				}

				FAIMoveRequest MoveRequest;
				MoveRequest.SetGoalActor(Player);
				MoveRequest.SetAcceptanceRadius(80.f);

				FNavPathSharedPtr NavPath;
				AIController->MoveTo(MoveRequest, &NavPath);

			}
			
		}), WaitTime, true);	//딜레이 시간 적용하여 계속 반복
}

void AYaroCharacter::SetInterpToEnemy(bool Interp)
{
	bInterpToEnemy = Interp;
}

FRotator AYaroCharacter::GetLookAtRotationYaw(FVector Target)
{
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Target);
	FRotator LookAtRotationYaw(0.f, LookAtRotation.Yaw, 0.f);
	return LookAtRotationYaw;
}

void AYaroCharacter::MoveToTarget(AEnemy* Target)
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

void AYaroCharacter::CombatSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		AEnemy* Enemy = Cast<AEnemy>(OtherActor);
		if (Enemy)
		{
			bOverlappingCombatSphere = true;
			for (int i = 0; i < Targets.Num(); i++)
			{
				if (Enemy == Targets[i]) //already exist
				{
					return;
				}
			}
			Targets.Add(Enemy);

			if (!CombatTarget)
			{
				CombatTarget = Targets[targetIndex];
				targetIndex++;
				MoveToTarget(CombatTarget);
				bHasCombatTarget = true;
			}
			if (!bAttacking)
			{
				Attack();
			}
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
			for (int i = 0; i < Targets.Num(); i++)
			{
				if (Enemy == Targets[i]) //already exist
				{
					Targets.Remove(Enemy); //타겟팅 가능 몹 배열에서 제거
					targetIndex--;
					if (targetIndex < 0) targetIndex = 0;
				}
			}

			if (Enemy == CombatTarget)
			{
				//UE_LOG(LogTemp, Log, TEXT("%s, overlap out"), *(this->GetName()));

				CombatTarget = nullptr;
				bHasCombatTarget = false;
			}

			if (Targets.Num() == 0)
			{
				bOverlappingCombatSphere = false;
			}
			else
			{
				AttackEnd();
			}
		}
	}
}

void AYaroCharacter::Attack()
{
	if ((!bAttacking) && (CombatTarget) && (CombatTarget->EnemyMovementStatus != EEnemyMovementStatus::EMS_Dead))
	{

		SkillNum = FMath::RandRange(1, 3);
		UBlueprintGeneratedClass* LoadedBP = LoadObject<UBlueprintGeneratedClass>(GetWorld(), TEXT("/Game/Blueprint/MagicAttacks/GreenStormAttack.GreenStormAttack_C")); //초기화 안 하면 ToSpawn에 초기화되지 않은 변수 넣었다고 오류남
		if (this->GetName().Contains("Luko"))
		{
			switch (SkillNum)
			{
				case 1:
					LoadedBP = LoadObject<UBlueprintGeneratedClass>(GetWorld(), TEXT("/Game/Blueprint/MagicAttacks/GreenStormAttack.GreenStormAttack_C"));
					break;
				case 2:
					LoadedBP = LoadObject<UBlueprintGeneratedClass>(GetWorld(), TEXT("/Game/Blueprint/MagicAttacks/DarkAttack.DarkAttack_C"));
					break;
				case 3:
					LoadedBP = LoadObject<UBlueprintGeneratedClass>(GetWorld(), TEXT("/Game/Blueprint/MagicAttacks/LightAttack.LightAttack_C"));
					break;
				default:
					break;
			}
		}
		if (this->GetName().Contains("Momo"))
		{
			switch (SkillNum)
			{
				case 1:
					LoadedBP = LoadObject<UBlueprintGeneratedClass>(GetWorld(), TEXT("/Game/Blueprint/MagicAttacks/RedStormAttack.RedStormAttack_C"));
					break;
				case 2:
					LoadedBP = LoadObject<UBlueprintGeneratedClass>(GetWorld(), TEXT("/Game/Blueprint/MagicAttacks/Fireball_Hit_Attack.Fireball_Hit_Attack_C"));
					break;
				case 3:
					LoadedBP = LoadObject<UBlueprintGeneratedClass>(GetWorld(), TEXT("/Game/Blueprint/MagicAttacks/FireAttack.FireAttack_C"));
					break;
				default:
					break;
			}
		}
		if (this->GetName().Contains("Vovo"))
		{
			switch (SkillNum)
			{
			case 1:
				LoadedBP = LoadObject<UBlueprintGeneratedClass>(GetWorld(), TEXT("/Game/Blueprint/MagicAttacks/YellowStormAttack.YellowStormAttack_C"));
				break;
			case 2:
				LoadedBP = LoadObject<UBlueprintGeneratedClass>(GetWorld(), TEXT("/Game/Blueprint/MagicAttacks/Waterball_Hit_Attack.Waterball_Hit_Attack_C"));
				break;
			case 3:
				LoadedBP = LoadObject<UBlueprintGeneratedClass>(GetWorld(), TEXT("/Game/Blueprint/MagicAttacks/AquaAttack.AquaAttack_C"));
				break;
			default:
				break;
			}
		}
		if (this->GetName().Contains("Vivi"))
		{
			switch (SkillNum)
			{
			case 1:
				LoadedBP = LoadObject<UBlueprintGeneratedClass>(GetWorld(), TEXT("/Game/Blueprint/MagicAttacks/BlueStormAttack.BlueStormAttack_C"));
				break;
			case 2:
				LoadedBP = LoadObject<UBlueprintGeneratedClass>(GetWorld(), TEXT("/Game/Blueprint/MagicAttacks/Ice_Hit_Attack.Ice_Hit_Attack_C"));
				break;
			case 3:
				LoadedBP = LoadObject<UBlueprintGeneratedClass>(GetWorld(), TEXT("/Game/Blueprint/MagicAttacks/IceAttack.IceAttack_C"));
				break;
			default:
				break;
			}
		}
		if (this->GetName().Contains("Zizi"))
		{
			switch (SkillNum)
			{
			case 1:
				LoadedBP = LoadObject<UBlueprintGeneratedClass>(GetWorld(), TEXT("/Game/Blueprint/MagicAttacks/PurpleStormAttack.PurpleStormAttack_C"));
				break;
			case 2:
				LoadedBP = LoadObject<UBlueprintGeneratedClass>(GetWorld(), TEXT("/Game/Blueprint/MagicAttacks/Thunderball_Hit_Attack.Thunderball_Hit_Attack_C"));
				break;
			case 3:
				LoadedBP = LoadObject<UBlueprintGeneratedClass>(GetWorld(), TEXT("/Game/Blueprint/MagicAttacks/LightningAttack.LightningAttack_C"));
				break;
			default:
				break;
			}
		}
		ToSpawn = Cast<UClass>(LoadedBP);

		bAttacking = true;
		SetInterpToEnemy(true);

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && CombatMontage)
		{
			AnimInstance->Montage_Play(CombatMontage);
			AnimInstance->Montage_JumpToSection(FName("Attack"), CombatMontage);

			Spawn();
		}
	}
}

void AYaroCharacter::AttackEnd()
{
	bAttacking = false;
	SetInterpToEnemy(false);

	if (bOverlappingCombatSphere)
	{
		if (CombatTarget && CombatTarget->EnemyMovementStatus == EEnemyMovementStatus::EMS_Dead)
		{
			for (int i = 0; i < Targets.Num(); i++)
			{
				if (CombatTarget == Targets[i]) //already exist
				{
					Targets.Remove(CombatTarget); //타겟팅 가능 몹 배열에서 제거
					targetIndex--;
					if (targetIndex < 0) targetIndex = 0;

				}
			}			

			CombatTarget = nullptr;
			bHasCombatTarget = false;

			if (Targets.Num() == 0)
			{
				bOverlappingCombatSphere = false;
			}
			else
			{
				CombatTarget = Targets[targetIndex];

				targetIndex++;
				if (targetIndex >= Targets.Num()) //타겟인덱스가 총 타겟 가능 몹 수 이상이면 다시 0으로 초기화
				{
					targetIndex = 0;
				}
				bHasCombatTarget = true;
			}
		}

		if (Targets.Num() != 0 && !CombatTarget)
		{
			CombatTarget = Targets[targetIndex];

			targetIndex++;
			if (targetIndex >= Targets.Num()) //타겟인덱스가 총 타겟 가능 몹 수 이상이면 다시 0으로 초기화
			{
				targetIndex = 0;
			}
			bHasCombatTarget = true;
		}
		if (CombatTarget)
			GetWorldTimerManager().SetTimer(AttackTimer, this, &AYaroCharacter::Attack, AttackDelay);
	}
}


void AYaroCharacter::Spawn()
{
	if (ToSpawn)
	{
		FTimerHandle WaitHandle;
		GetWorld()->GetTimerManager().SetTimer(WaitHandle, FTimerDelegate::CreateLambda([&]()
			{
				UWorld* world = GetWorld();
				if (world)
				{
					FActorSpawnParameters spawnParams;
					spawnParams.Owner = this;

					FRotator rotator = this->GetActorRotation();

					FVector spawnLocation = AttackArrow->GetComponentTransform().GetLocation();
					if (CombatTarget)
					{
						if (this->GetName().Contains("Luko"))
						{
							if (SkillNum != 1) //루코의 경우 2,3번 스킬은 적 위치에서 스폰
							{
								spawnLocation = CombatTarget->GetActorLocation();
							}
						}
						else
						{
							if (SkillNum == 3) //루코 제외 3번 스킬만 적 위치에서 스폰
							{
								spawnLocation = CombatTarget->GetActorLocation();
							}
						}
					}		

					MagicAttack = world->SpawnActor<AMagicSkill>(ToSpawn, spawnLocation, rotator, spawnParams);
					if (MagicAttack && CombatTarget)
					{
						MagicAttack->Target = CombatTarget;
						MagicAttack->Caster = this;
					}
				}
			}), 0.6f, false); // 0.6초 뒤 실행, 반복X
	}
}

void AYaroCharacter::MoveToLocation()
{	
	if (AIController)
	{
		AIController->MoveToLocation(Pos[index]);
		//index++;
	}

	//bool vovo = false;
	//bool vivi = false;
	//bool zizi = false;

	GetWorld()->GetTimerManager().SetTimer(TeamMoveTimer, FTimerDelegate::CreateLambda([&]() {
		if (!CombatTarget && !bOverlappingCombatSphere)
		{
			if (index <= 3)
			{
				float distance = (GetActorLocation() - Pos[index]).Size();
				//UE_LOG(LogTemp, Log, TEXT("%f"), distance);
				
				if (AIController && distance <= 70.f)
				{
					//if (this->GetName().Contains("Vovo")) vovo = true;
					//if (this->GetName().Contains("Vivi")) vivi = true;
					//if (this->GetName().Contains("Zizi")) zizi = true;

					index++;
					AIController->MoveToLocation(Pos[index]);
				}
				else
				{
					//if (this->GetName().Contains("Vovo")) vovo = false;
					//if (this->GetName().Contains("Vivi")) vivi = false;
					//if (this->GetName().Contains("Zizi")) zizi = false;
					AIController->MoveToLocation(Pos[index]);
				}
			}
		}
	}), 1.f, true);
}
