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
#include "MainPlayerController.h"

//////////////////////////////////////////////////////////////////////////
// AYaroCharacter

AYaroCharacter::AYaroCharacter()
{
	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 60.f;
	GetCharacterMovement()->AirControl = 0.2f;
	GetCharacterMovement()->MaxStepHeight = 55.f;

	Wand = CreateDefaultSubobject<UBoxComponent>(TEXT("WandMesh"));
	Wand->SetupAttachment(GetMesh(), FName("RightHandSocket"));

	CombatSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatSphere"));
	CombatSphere->SetupAttachment(GetRootComponent());
	CombatSphere->InitSphereRadius(550.f);
	CombatSphere->SetRelativeLocation(FVector(260.f, 0.f, 0.f));
	CombatSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	CombatSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Ignore);
	CombatSphere->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);

    AttackSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AttackSphere"));
    AttackSphere->SetupAttachment(GetRootComponent());
    AttackSphere->InitSphereRadius(460.f);
    AttackSphere->SetRelativeLocation(FVector(250.f, 0.f, 0.f));
    AttackSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
    AttackSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Ignore);
    AttackSphere->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);

	NotAllowSphere = CreateDefaultSubobject<USphereComponent>(TEXT("NotAllowSphere"));
	NotAllowSphere->SetupAttachment(GetRootComponent());
	NotAllowSphere->InitSphereRadius(100.f);
	NotAllowSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	NotAllowSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Ignore);
	NotAllowSphere->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);

	bOverlappingAttackSphere = false;

	AttackArrow = CreateAbstractDefaultSubobject<UArrowComponent>(TEXT("AttackArrow"));
	AttackArrow->SetupAttachment(GetRootComponent());
	AttackArrow->SetRelativeLocation(FVector(160.f, 4.f, 26.f));

	InterpSpeed = 15.f;
	bInterpToEnemy = false;

	// positions in first dungeon
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

	CombatSphere->OnComponentBeginOverlap.AddDynamic(this, &AYaroCharacter::CombatSphereOnOverlapBegin);
	CombatSphere->OnComponentEndOverlap.AddDynamic(this, &AYaroCharacter::CombatSphereOnOverlapEnd);

    AttackSphere->OnComponentBeginOverlap.AddDynamic(this, &AYaroCharacter::AttackSphereOnOverlapBegin);
    AttackSphere->OnComponentEndOverlap.AddDynamic(this, &AYaroCharacter::AttackSphereOnOverlapEnd);

	NotAllowSphere->OnComponentBeginOverlap.AddDynamic(this, &AYaroCharacter::NotAllowSphereOnOverlapBegin);
	NotAllowSphere->OnComponentEndOverlap.AddDynamic(this, &AYaroCharacter::NotAllowSphereOnOverlapEnd);

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

	if (bInterpToCharacter && TargetCharacter)
	{
        FRotator LookAtYaw = GetLookAtRotationYaw(TargetCharacter->GetActorLocation());
        FRotator InterpRotation = FMath::RInterpTo(GetActorRotation(), LookAtYaw, DeltaTime, InterpSpeed); //smooth transition

        SetActorRotation(InterpRotation);
	}

	if (!Player)
	{
		ACharacter* p = UGameplayStatics::GetPlayerCharacter(this, 0);
		Player = Cast<AMain>(p);

		if (Player)
		{
            if (this->GetName().Contains("Momo"))
            {
                Player->Momo = this;
            }
            if (this->GetName().Contains("Luko"))
            {
                Player->Luko = this;
            }
            if (this->GetName().Contains("Vovo"))
            {
                Player->Vovo = this;
            }
            if (this->GetName().Contains("Vivi"))
            {
                Player->Vivi = this;
            }
            if (this->GetName().Contains("Zizi"))
            {
                Player->Zizi = this;
            }
			Player->NPCList.Add(this);
		}
	}


	//if (!canGo && Player && Player->NpcGo)
	//{
	//	canGo = true;
	//	if (this->GetName().Contains("Momo") || this->GetName().Contains("Luko"))
	//	{
 //           GetWorldTimerManager().SetTimer(MoveTimer, this, &AYaroCharacter::MoveToPlayer, 1.f);
	//	}
	//	else // 비비, 지지, 보보
	//	{
	//		MoveToLocation();
	//	}
	//}

    if(!AnimInstance) AnimInstance = GetMesh()->GetAnimInstance();

}


void AYaroCharacter::MoveToPlayer()
{
	if (Player == nullptr) return;

	if (CombatTarget == nullptr && !bAttacking && !bOverlappingAttackSphere && AgroTargets.Num() == 0) // basic condition
	{
		if (Player->MainPlayerController->bDialogueUIVisible)
		{
			if (Player->MainPlayerController->DialogueNum != 6)
			{
				GetWorldTimerManager().SetTimer(MoveTimer, this, &AYaroCharacter::MoveToPlayer, 0.5f);
				return;
			}
		}

		if (Player->MainPlayerController->DialogueNum < 3)
		{
			// 대화 넘버 1에서 루코가 이동하는 것 허용
			if (!(Player->MainPlayerController->DialogueNum == 1 && this->GetName().Contains("Luko")))
				return;
		}

		for (int i = 0; i < Player->NPCList.Num(); i++)
		{
			if (Player->NPCList[i]->AgroTargets.Num() != 0 && Player->NPCList[i]->AgroTargets[0]->EnemyMovementStatus != EEnemyMovementStatus::EMS_Dead
				&& !UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("first")) // 다른 npc의 인식 범위에 몬스터가 있으면 도와주러 감, 단 첫번째 던전은 제외
			{
				UE_LOG(LogTemp, Log, TEXT("go help move %s"), *this->GetName());
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
				MoveToTarget(Player->NPCList[i]->AgroTargets[0]);
				GetWorldTimerManager().SetTimer(MoveTimer, this, &AYaroCharacter::MoveToPlayer, 1.f);

				return;
			}
		}

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
			if (UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("first") && !GetWorldTimerManager().IsTimerActive(TeleportTimer))
				Teleport();
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
			TeleportCount = 0;
        }

        FAIMoveRequest MoveRequest;
        MoveRequest.SetGoalActor(Player);
        MoveRequest.SetAcceptanceRadius(80.f);

        FNavPathSharedPtr NavPath;
        AIController->MoveTo(MoveRequest, &NavPath);
	}
	else
	{
		if(UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("first"))
			GetWorldTimerManager().ClearTimer(TeleportTimer); 
	}

    GetWorldTimerManager().SetTimer(MoveTimer, this, &AYaroCharacter::MoveToPlayer, 0.5f);
}

void AYaroCharacter::Teleport()
{
	if (Player->MovementStatus == EMovementStatus::EMS_Dead)
	{
		TeleportCount = 0;
		return;
	}
	TeleportCount += 1;
	if (TeleportCount >= 25) SetActorLocation(Player->GetActorLocation() + FVector(15.f, 25.f, 0.f));

	float distance = (GetActorLocation() - Player->GetActorLocation()).Size();
	//UE_LOG(LogTemp, Log, TEXT("count %d %s"), TeleportCount, *this->GetName());

	if (distance <= 400.f)
	{
		if (AIController)
		{
			TeleportCount = 0;
			GetWorldTimerManager().ClearTimer(TeleportTimer);
		}
	}
	else
	{
		GetWorldTimerManager().SetTimer(TeleportTimer, this, &AYaroCharacter::Teleport, 0.5f);
	}
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
			//UE_LOG(LogTemp, Log, TEXT("Yaro CombatSphereOnOverlapBegin %s"), *this->GetName());

			for (int i = 0; i < AgroTargets.Num(); i++)
			{
				if (Enemy == AgroTargets[i]) //already exist
				{
					return;
				}
			}
			AgroTargets.Add(Enemy);

			if (!CombatTarget)
			{
				//UE_LOG(LogTemp, Log, TEXT("MoveToTarget %s"), *this->GetName());

				MoveToTarget(Enemy);
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
			if (!UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("boss") || (UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("boss") && Enemy->EnemyMovementStatus == EEnemyMovementStatus::EMS_Dead))
			{
				for (int i = 0; i < AgroTargets.Num(); i++)
				{
					if (Enemy == AgroTargets[i]) //already exist
					{
						AgroTargets.Remove(Enemy); //타겟팅 가능 몹 배열에서 제거
					}
				}
			}

			//UE_LOG(LogTemp, Log, TEXT("CombatSphereOnOverlapEnd %s"), *this->GetName());

			if (!CombatTarget || Enemy == CombatTarget)
			{
				CombatTarget = nullptr;

				if (AgroTargets.Num() == 0)
				{
					for (int i = 0; i < Player->NPCList.Num(); i++)
					{
						if (Player->NPCList[i]->AgroTargets.Num() != 0 && !UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("first")) // 다른 npc의 인식 범위에 몬스터가 있으면 도와주러 감
						{

							if (Player->NPCList[i]->AgroTargets[0]->EnemyMovementStatus == EEnemyMovementStatus::EMS_Dead) return;
							UE_LOG(LogTemp, Log, TEXT("CombatSphereOnOverlapEnd Go Help %s"), *this->GetName());

							MoveToTarget(Player->NPCList[i]->AgroTargets[0]);
								
						}
					}
				}
				else
				{
					//UE_LOG(LogTemp, Log, TEXT("I Have AgroTargets %s"), *this->GetName());

					bOverlappingAttackSphere = true;
					CombatTarget = AgroTargets[0];
					Attack();
				}
			}
			
		}
	}
}

void AYaroCharacter::AttackSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor)
    {
        AEnemy* Enemy = Cast<AEnemy>(OtherActor);

        if (Enemy)
        {
			for (int i = 0; i < CombatTargets.Num(); i++)
			{
				if (Enemy == CombatTargets[i]) //already exist
				{
					return;
				}
			}
			CombatTargets.Add(Enemy);

			if (CombatTarget) return;

            //UE_LOG(LogTemp, Log, TEXT("AttackSphereOnOverlapBegin %s"), *this->GetName());
			bOverlappingAttackSphere = true;

			CombatTarget = Enemy;

			AIController->StopMovement();
			Attack();          
        }
    }
}

void AYaroCharacter::AttackSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor)
    {
        AEnemy* Enemy = Cast<AEnemy>(OtherActor);
        if (Enemy)
        {
            //UE_LOG(LogTemp, Log, TEXT("AttackSphereOnOverlapEnd %s"), *this->GetName());	

			for (int i = 0; i < CombatTargets.Num(); i++)
			{
				if (Enemy == CombatTargets[i]) //already exist
				{
					CombatTargets.Remove(Enemy); //타겟팅 가능 몹 배열에서 제거
				}
			}

			if (Enemy == CombatTarget)
			{
				CombatTarget = nullptr;			

				if (CombatTargets.Num() != 0)
				{
					//UE_LOG(LogTemp, Log, TEXT("I Have CombatTargets %s"), *this->GetName());

					CombatTarget = CombatTargets[0];
					Attack();
				}
				else
				{
					bOverlappingAttackSphere = false;

					if (AgroTargets.Num() != 0 && AgroTargets[0]->EnemyMovementStatus != EEnemyMovementStatus::EMS_Dead)
					{
						//UE_LOG(LogTemp, Log, TEXT("No CombatTargets but AgroTargets Move %s"), *this->GetName());

						MoveToTarget(AgroTargets[0]);

					}
					else
					{
						//UE_LOG(LogTemp, Log, TEXT("Nothing %s"), *this->GetName());

					}

				}
			}
        }
    }
}

void AYaroCharacter::Attack()
{
	if ((!bAttacking) && (CombatTarget) && (CombatTarget->EnemyMovementStatus != EEnemyMovementStatus::EMS_Dead))
	{
        //UE_LOG(LogTemp, Log, TEXT("Attack,  %s"), *this->GetName());

		if (bCanCastStrom) //npc can cast strom magic
		{
			if (Player->MainPlayerController->DialogueNum <= 5)
			{
				SkillNum = FMath::RandRange(1, 3);
			}
			else if (Player->MainPlayerController->DialogueNum <= 14)
			{
				SkillNum = FMath::RandRange(1, 4);
			}
			else
			{
				SkillNum = FMath::RandRange(1, 5);
			}          
		}
		else //npc can't cast strom magic
		{
			if (Player->MainPlayerController->DialogueNum <= 5)
			{
				SkillNum = FMath::RandRange(2, 3);
			}
			else if (Player->MainPlayerController->DialogueNum <= 14)
			{
				SkillNum = FMath::RandRange(2, 4);
			}
			else
			{
				SkillNum = FMath::RandRange(2, 5);
			}
		}
		
        if (SkillNum == 1)
        {
            bCanCastStrom = false;
            FTimerHandle StormTimer;
            GetWorldTimerManager().SetTimer(StormTimer, this, &AYaroCharacter::CanCastStormMagic, 8.f, false);  // 8초 뒤 다시 스톰 사용 가능
        }

		UBlueprintGeneratedClass* LoadedBP = LoadObject<UBlueprintGeneratedClass>(GetWorld(), TEXT("/Game/Blueprints/MagicAttacks/Luko/1_GreenStormAttack.1_GreenStormAttack_C")); //초기화 안 하면 ToSpawn에 초기화되지 않은 변수 넣었다고 오류남
		if (this->GetName().Contains("Luko"))
		{
			switch (SkillNum)
			{
				case 1:
					LoadedBP = LoadObject<UBlueprintGeneratedClass>(GetWorld(), TEXT("/Game/Blueprints/MagicAttacks/Luko/1_GreenStormAttack.1_GreenStormAttack_C"));
					break;
                case 2:
                    LoadedBP = LoadObject<UBlueprintGeneratedClass>(GetWorld(), TEXT("/Game/Blueprints/MagicAttacks/Luko/2_Greenball_Hit_Attack.2_Greenball_Hit_Attack_C"));
                    break;
				case 3:
					LoadedBP = LoadObject<UBlueprintGeneratedClass>(GetWorld(), TEXT("/Game/Blueprints/MagicAttacks/Luko/3_LightAttack.3_LightAttack_C"));
					break;
                case 4:
                    LoadedBP = LoadObject<UBlueprintGeneratedClass>(GetWorld(), TEXT("/Game/Blueprints/MagicAttacks/Luko/4_DarkAttack.4_DarkAttack_C"));
                    break;
                case 5:
                    LoadedBP = LoadObject<UBlueprintGeneratedClass>(GetWorld(), TEXT("/Game/Blueprints/MagicAttacks/Luko/5_GreenLaserAttack.5_GreenLaserAttack_C"));
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
					LoadedBP = LoadObject<UBlueprintGeneratedClass>(GetWorld(), TEXT("/Game/Blueprints/MagicAttacks/Momo/1_RedStormAttack.1_RedStormAttack_C"));
					break;
				case 2:
					LoadedBP = LoadObject<UBlueprintGeneratedClass>(GetWorld(), TEXT("/Game/Blueprints/MagicAttacks/Momo/2_Fireball_Hit_Attack.2_Fireball_Hit_Attack_C"));
					break;
				case 3:
					LoadedBP = LoadObject<UBlueprintGeneratedClass>(GetWorld(), TEXT("/Game/Blueprints/MagicAttacks/Momo/3_FireAttack.3_FireAttack_C"));
					break;
                case 4:
                    LoadedBP = LoadObject<UBlueprintGeneratedClass>(GetWorld(), TEXT("/Game/Blueprints/MagicAttacks/Momo/4_Fireball_Hit_Attack.4_Fireball_Hit_Attack_C"));
                    break;
                case 5:
                    LoadedBP = LoadObject<UBlueprintGeneratedClass>(GetWorld(), TEXT("/Game/Blueprints/MagicAttacks/Momo/5_RedLaserAttack.5_RedLaserAttack_C"));
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
				LoadedBP = LoadObject<UBlueprintGeneratedClass>(GetWorld(), TEXT("/Game/Blueprints/MagicAttacks/Vovo/1_YellowStormAttack.1_YellowStormAttack_C"));
				break;
			case 2:
				LoadedBP = LoadObject<UBlueprintGeneratedClass>(GetWorld(), TEXT("/Game/Blueprints/MagicAttacks/Vovo/2_Waterball_Hit_Attack.2_Waterball_Hit_Attack_C"));
				break;
			case 3:
				LoadedBP = LoadObject<UBlueprintGeneratedClass>(GetWorld(), TEXT("/Game/Blueprints/MagicAttacks/Vovo/3_AquaAttack.3_AquaAttack_C"));
				break;
            case 4:
                LoadedBP = LoadObject<UBlueprintGeneratedClass>(GetWorld(), TEXT("/Game/Blueprints/MagicAttacks/Vovo/4_AuraAttack.4_AuraAttack_C"));
                break;
            case 5:
                LoadedBP = LoadObject<UBlueprintGeneratedClass>(GetWorld(), TEXT("/Game/Blueprints/MagicAttacks/Vovo/5_YellowLaserAttack.5_YellowLaserAttack_C"));
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
				LoadedBP = LoadObject<UBlueprintGeneratedClass>(GetWorld(), TEXT("/Game/Blueprints/MagicAttacks/Vivi/1_BlueStormAttack.1_BlueStormAttack_C"));
				break;
			case 2:
				LoadedBP = LoadObject<UBlueprintGeneratedClass>(GetWorld(), TEXT("/Game/Blueprints/MagicAttacks/Vivi/2_Ice_Hit_Attack.2_Ice_Hit_Attack_C"));
				break;
            case 3:
                LoadedBP = LoadObject<UBlueprintGeneratedClass>(GetWorld(), TEXT("/Game/Blueprints/MagicAttacks/Vivi/3_IceBolt_Hit_Attack.3_IceBolt_Hit_Attack_C"));
                break;
			case 4:
				LoadedBP = LoadObject<UBlueprintGeneratedClass>(GetWorld(), TEXT("/Game/Blueprints/MagicAttacks/Vivi/4_IceAttack.4_IceAttack_C"));
				break;
            case 5:
                LoadedBP = LoadObject<UBlueprintGeneratedClass>(GetWorld(), TEXT("/Game/Blueprints/MagicAttacks/Vivi/5_BlueLaserAttack.5_BlueLaserAttack_C"));
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
				LoadedBP = LoadObject<UBlueprintGeneratedClass>(GetWorld(), TEXT("/Game/Blueprints/MagicAttacks/Zizi/1_PurpleStormAttack.1_PurpleStormAttack_C"));
				break;
			case 2:
				LoadedBP = LoadObject<UBlueprintGeneratedClass>(GetWorld(), TEXT("/Game/Blueprints/MagicAttacks/Zizi/2_Thunderball_Hit_Attack.2_Thunderball_Hit_Attack_C"));
				break;
            case 3:
                LoadedBP = LoadObject<UBlueprintGeneratedClass>(GetWorld(), TEXT("/Game/Blueprints/MagicAttacks/Zizi/3_EnergyBolt_Hit_Attack.3_EnergyBolt_Hit_Attack_C"));
				break;
			case 4:
				LoadedBP = LoadObject<UBlueprintGeneratedClass>(GetWorld(), TEXT("/Game/Blueprints/MagicAttacks/Zizi/4_LightningAttack.4_LightningAttack_C"));
				break;
            case 5:
                LoadedBP = LoadObject<UBlueprintGeneratedClass>(GetWorld(), TEXT("/Game/Blueprints/MagicAttacks/Zizi/5_TornadoAttack.5_TornadoAttack_C"));
                break;
			default:
				break;
			}
		}
		ToSpawn = Cast<UClass>(LoadedBP);

		bAttacking = true;
		SetInterpToEnemy(true);

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
        //UE_LOG(LogTemp, Log, TEXT("AttackEnd... %s"), * this->GetName());
	}
}

void AYaroCharacter::AttackEnd()
{
	bAttacking = false;
	SetInterpToEnemy(false);
    //UE_LOG(LogTemp, Log, TEXT("AttackEnd %s"), *this->GetName());

	if (bOverlappingAttackSphere)
	{
		if (CombatTarget && CombatTarget->EnemyMovementStatus == EEnemyMovementStatus::EMS_Dead)
		{
			//UE_LOG(LogTemp, Log, TEXT("CombatTarget died %s"), *this->GetName());


			for (int i = 0; i < CombatTargets.Num(); i++)
			{
				if (CombatTarget == CombatTargets[i]) //already exist
				{
					CombatTargets.Remove(CombatTarget); //타겟팅 가능 몹 배열에서 제거
				}
			}			

			for (int i = 0; i < AgroTargets.Num(); i++)
			{
				if (CombatTarget == AgroTargets[i]) //already exist
				{
					AgroTargets.Remove(CombatTarget); //타겟팅 가능 몹 배열에서 제거

				}
			}

			CombatTarget = nullptr;

			if (CombatTargets.Num() == 0)
			{
				bOverlappingAttackSphere = false;
                //UE_LOG(LogTemp, Log, TEXT("AttackEnd - No CombatTargets %s"), *this->GetName());
				
				if (AgroTargets.Num() != 0)
				{
					MoveToTarget(AgroTargets[0]);

					UE_LOG(LogTemp, Log, TEXT("AttackEnd - but havve AgroTargets %s"), *this->GetName());

				}
				else
				{
					for (int i = 0; i < Player->NPCList.Num(); i++)
					{
						if (Player->NPCList[i]->AgroTargets.Num() != 0 && !UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("first")) // 다른 npc의 인식 범위에 몬스터가 있으면 도와주러 감
						{
							UE_LOG(LogTemp, Log, TEXT("AttackEnd - Go Help %s"), *this->GetName());

							MoveToTarget(Player->NPCList[i]->AgroTargets[0]);
						}
					}
				}
			}
			else
			{
				CombatTarget = CombatTargets[0];
                //UE_LOG(LogTemp, Log, TEXT("AttackEnd - i have CombatTarget %s"), * this->GetName());
			}
		}

		if (CombatTarget)
			GetWorldTimerManager().SetTimer(AttackTimer, this, &AYaroCharacter::Attack, AttackDelay);
	}
	else
	{
		if (AgroTargets.Num() != 0)
		{
			//UE_LOG(LogTemp, Log, TEXT("AttackEnd2 - No Combat Yes Agro %s"), *this->GetName());
			for (int i = 0; i < AgroTargets.Num(); i++)
			{
				if (AgroTargets[i]->EnemyMovementStatus != EEnemyMovementStatus::EMS_Dead)
				{
					if (GetDistanceTo(AgroTargets[i]) >= 450.f)
					{
						MoveToTarget(AgroTargets[i]);
						UE_LOG(LogTemp, Log, TEXT("AttackEnd2 - Far Move %s"), *this->GetName());
						return;

					}
					else
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

				FVector spawnLocation = AttackArrow->GetComponentTransform().GetLocation();
				if (CombatTarget)
				{
						if (SkillNum == 3 && (this->GetName().Contains("Momo") || this->GetName().Contains("Luko") || this->GetName().Contains("Vovo"))) //모모,루코,보보의 경우 3번 스킬은 적 위치에서 스폰
						{
							spawnLocation = CombatTarget->GetActorLocation();
						}
						
						if (SkillNum == 4 && !this->GetName().Contains("Momo")) //모모 제외 4번 스킬 적 위치에서 스폰
						{
							spawnLocation = CombatTarget->GetActorLocation();
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


void AYaroCharacter::CanCastStormMagic()
{
	bCanCastStrom = true;
}

void AYaroCharacter::MoveToLocation() // Vivi, Vovo, Zizi
{	
	if (AIController)
	{
		AIController->MoveToLocation(Pos[index]);
	}

	GetWorld()->GetTimerManager().SetTimer(TeamMoveTimer, FTimerDelegate::CreateLambda([&]() {

		if (!CombatTarget && !bOverlappingAttackSphere)
		{
            float distance = (GetActorLocation() - Pos[index]).Size();

			if (index <= 3)
			{
				//UE_LOG(LogTemp, Log, TEXT("%f"), distance);
				
				if (AIController && distance <= 70.f)
				{
					index++;
					AIController->MoveToLocation(Pos[index]);
				}
				else
				{
					AIController->MoveToLocation(Pos[index]);
				}
			}
			if (index == 4 && distance <= 70.f) // 골렘쪽으로 이동 중
			{
				if (Player->Momo->CombatTarget && Player->Momo->CombatTarget->GetName().Contains("Golem"))
				{
					AIController->MoveToActor(Player->Momo->CombatTarget);
				}
                if (Player->CombatTarget && Player->CombatTarget->GetName().Contains("Golem"))
                {
                    AIController->MoveToActor(Player->CombatTarget);
                }
			}
		}
	}), 0.5f, true);
}

float AYaroCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	//if (Health - DamageAmount <= 0.f) // Decrease Health
	//{
	//	Health = 0.f;
	//	Die();
	//}
	//else
	//{
	//	if (AnimInstance && !bAttacking)
	//	{
	//		AnimInstance->Montage_Play(CombatMontage);
	//		AnimInstance->Montage_JumpToSection(FName("Hit"), CombatMontage);
	//	}
	//	Health -= DamageAmount;
	//}

	if (!UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("boss")) return DamageAmount;

	MagicAttack = Cast<AMagicSkill>(DamageCauser);

	int TargetIndex = MagicAttack->index;
	
	if (TargetIndex == 11 && AgroTargets.Num() == 0)
	{
		AEnemy* BossEnemy = Cast<AEnemy>(UGameplayStatics::GetActorOfClass(GetWorld(), Boss));
		GetWorldTimerManager().ClearTimer(MoveTimer);
		AIController->StopMovement();
		MoveToTarget(BossEnemy);
		//UE_LOG(LogTemp, Log, TEXT("yesyesyes %s"), *this->GetName());

	}
	////UE_LOG(LogTemp, Log, TEXT("attck %s"), *MagicAttack->GetName());

	return DamageAmount;
}

void AYaroCharacter::Smile()
{
	this->bSmile = true;
}

void AYaroCharacter::UsualFace()
{
	this->bSmile = false;
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
				{
					return;
				}
			}
			DangerousTargets.Add(Enemy);

			if (GetWorldTimerManager().IsTimerActive(SafeDistanceTimer)) return;

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

void AYaroCharacter::MoveToSafeLocation()
{
	if (DangerousTargets.Num() != 0)
	{
		GetWorldTimerManager().SetTimer(SafeDistanceTimer, this, &AYaroCharacter::MoveToSafeLocation, 3.f);
	}
	else
	{
		GetWorldTimerManager().ClearTimer(SafeDistanceTimer);
		return;
	}
	float value = FMath::RandRange(-200.f, 200.f);

	FVector SafeLocation = GetActorLocation() + FVector(value, value, 0.f);
	AIController->MoveToLocation(SafeLocation);
	UE_LOG(LogTemp, Log, TEXT("movwsafeg %s"), *this->GetName());

}
