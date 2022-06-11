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

	bOverlappingCombatSphere = false;
	bHasCombatTarget = false;
	targetIndex = 0;

	AttackArrow = CreateAbstractDefaultSubobject<UArrowComponent>(TEXT("AttackArrow"));
	AttackArrow->SetupAttachment(GetRootComponent());
	AttackArrow->SetRelativeLocation(FVector(160.f, 4.f, 26.f));

	InterpSpeed = 15.f;
	bInterpToEnemy = false;


	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

void AYaroCharacter::BeginPlay()
{
	Super::BeginPlay();

	ACharacter* p = UGameplayStatics::GetPlayerCharacter(this, 0);
	Player = Cast<AMain>(p);

	AIController = Cast<AAIController>(GetController());
	MoveToPlayer();

	CombatSphere->OnComponentBeginOverlap.AddDynamic(this, &AYaroCharacter::CombatSphereOnOverlapBegin);
	CombatSphere->OnComponentEndOverlap.AddDynamic(this, &AYaroCharacter::CombatSphereOnOverlapEnd);

}

void AYaroCharacter::Tick(float DeltaTime)
{
	if (bInterpToEnemy && CombatTarget)
	{
		FRotator LookAtYaw = GetLookAtRotationYaw(CombatTarget->GetActorLocation());
		FRotator InterpRotation = FMath::RInterpTo(GetActorRotation(), LookAtYaw, DeltaTime, InterpSpeed); //smooth transition

		SetActorRotation(InterpRotation);
	}
}


void AYaroCharacter::MoveToPlayer()
{
	FTimerHandle WaitHandle;
	float WaitTime = 1.5f; // ������ Ÿ�� ����
	GetWorld()->GetTimerManager().SetTimer(WaitHandle, FTimerDelegate::CreateLambda([&]()
		{
			if (!CombatTarget && !bAttacking && !bOverlappingCombatSphere || Player->MovementStatus == EMovementStatus::EMS_Dead)
			{
				float distance = GetDistanceTo(Player);

				if (distance >= 500.f) //���� �Ÿ� �̻� �������ִٸ� �ӵ� ���� �޸���
				{
					//UE_LOG(LogTemp, Log, TEXT("%s"), *(this->GetName()));
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
				else //�����ٸ� �ӵ� ���� �ȱ�
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
			
		}), WaitTime, true);	//������ �ð� �����Ͽ� ��� �ݺ�
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
		MoveRequest.SetAcceptanceRadius(400.0f);

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
					Targets.Remove(Enemy); //Ÿ���� ���� �� �迭���� ����
				}
			}
			if (Targets.Num() == 0)
			{
				bOverlappingCombatSphere = false;
			}

			if (CombatTarget == Enemy)
			{
				CombatTarget = nullptr;
				bHasCombatTarget = false;
			}
		}
	}
}

void AYaroCharacter::Attack()
{
	if (!bAttacking && Player->MovementStatus != EMovementStatus::EMS_Dead)
	{
		SkillNum = 0;//FMath::RandRange(1, 3);
		UBlueprintGeneratedClass* LoadedBP = LoadObject<UBlueprintGeneratedClass>(GetWorld(), TEXT("/Game/Blueprint/MagicAttacks/GreenStormAttack.GreenStormAttack_C")); //�ʱ�ȭ �� �ϸ� ToSpawn�� �ʱ�ȭ���� ���� ���� �־��ٰ� ������
		switch (SkillNum)
		{
			case 1:
				LoadedBP = LoadObject<UBlueprintGeneratedClass>(GetWorld(), TEXT("/Game/Blueprint/MagicAttacks/WindAttack.WindAttack_C"));
				break;
			case 2:
				LoadedBP = LoadObject<UBlueprintGeneratedClass>(GetWorld(), TEXT("/Game/Blueprint/MagicAttacks/ShockAttack.ShockAttack_C"));
				break;
			case 3:
				LoadedBP = LoadObject<UBlueprintGeneratedClass>(GetWorld(), TEXT("/Game/Blueprint/MagicAttacks/EarthAttack.EarthAttack_C"));
				break;
			default:
				break;
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
		if (!CombatTarget || CombatTarget->EnemyMovementStatus == EEnemyMovementStatus::EMS_Dead)
		{
			CombatTarget = Targets[targetIndex];
			targetIndex++;
			if (targetIndex >= Targets.Num()) //Ÿ���ε����� �� Ÿ�� ���� �� �� �̻��̸� �ٽ� 0���� �ʱ�ȭ
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
					if (SkillNum != 1 && CombatTarget)
					{
						spawnLocation = CombatTarget->GetActorLocation();
					}

					MagicAttack = world->SpawnActor<AMagicSkill>(ToSpawn, spawnLocation, rotator, spawnParams);
					if (CombatTarget) MagicAttack->Target = CombatTarget;
				}
			}), 0.6f, false); // 0.6�� �� ����, �ݺ�X
	}
}