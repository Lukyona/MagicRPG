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
#include "Kismet/KismetMathLibrary.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Components/CapsuleComponent.h"
#include "Yaro/System/MainPlayerController.h"

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

		if (Player) 
		{// NPC ���� ����
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
			Player->AddNpcToList(this);
		}
	}
}

void AYaroCharacter::MoveToPlayer()
{
	if (Player == nullptr) return;

	if (CombatTarget == nullptr && !bAttacking && !bOverlappingCombatSphere && AgroTargets.Num() == 0) // ���� ���� �ƴ� ��
	{
		if (Player->GetMainPlayerController()->bDialogueUIVisible) // ��ȭ �߿��� ��κ� �̵�X
		{
			if (Player->GetMainPlayerController()->DialogueNum != 6) // Ư�� ���� �����ϰ� �÷��̾ ���󰡱�
			{
				GetWorldTimerManager().SetTimer(MoveTimer, this, &AYaroCharacter::MoveToPlayer, 0.5f);
				return;
			}
		}

		if (Player->GetMainPlayerController()->DialogueNum < 3)
		{
			// ��ȭ �ѹ� 1���� ���� ȥ�� �÷��̾�Է� �̵�, ���� ���� ���� ����
			if (!(Player->GetMainPlayerController()->DialogueNum == 1 && this->GetName().Contains("Luko")))
				return;
		}

		for (int i = 0; i < Player->GetNPCList().Num(); i++)
		{
			// �ٸ� npc�� �ν� ������ ���Ͱ� ������ �����ַ� ��, �� ù��° ������ ����
			if (Player->GetNPCList()[i]->AgroTargets.Num() != 0
				&& Player->GetNPCList()[i]->AgroTargets[0]->GetEnemyMovementStatus() != EEnemyMovementStatus::EMS_Dead
				&& !UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("first")) 
			{
				// �޸��� ���ǵ�� ����
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
				MoveToTarget(Player->GetNPCList()[i]->AgroTargets[0]);

				//if(!GetWorldTimerManager().IsTimerActive(MoveTimer))
					GetWorldTimerManager().SetTimer(MoveTimer, this, &AYaroCharacter::MoveToPlayer, 1.f);

				return;
			}
		}

		// �÷��̾���� �Ÿ�
        float distance = GetDistanceTo(Player);

        if (distance >= 500.f) //���� �Ÿ� �̻� �������ִٸ� �ӵ� ���� �޸���
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

			// ù��° ���������� npc���� ������ ���ų� ���缭 ���࿡ ���ص��� �ʵ��� �ڷ���Ʈ ��� ����
			if (UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("first")
				&& !GetWorldTimerManager().IsTimerActive(TeleportTimer))
				Teleport();
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

					for (int i = 0; i < Player->GetNPCList().Num(); i++)
					{
						// �ٸ� npc�� �ν� ������ ���Ͱ� ������ �����ַ� ��, ù��° ���� ����
						if (Player->GetNPCList()[i]->AgroTargets.Num() != 0
							&& !UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("first")) 
						{
							// �� ���Ͱ� ���� ���¸� ����
							if (Player->GetNPCList()[i]->AgroTargets[0]->GetEnemyMovementStatus() == EEnemyMovementStatus::EMS_Dead) return;
							//UE_LOG(LogTemp, Log, TEXT("AgroSphereOnOverlapEnd Go Help %s"), *this->GetName());

							MoveToTarget(Player->GetNPCList()[i]->AgroTargets[0]);
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
		if (this->GetName().Contains("Vovo") && bIsHealTime)
		{
			GetWorldTimerManager().SetTimer(AttackTimer, this, &AYaroCharacter::Attack, 2.f);
			return;
		}

		if (bCanCastStrom) // npc can cast strom magic
		{
			if (Player->GetMainPlayerController()->DialogueNum <= 5)
				SetSkillNum(FMath::RandRange(1, 3));
			else if (Player->GetMainPlayerController()->DialogueNum <= 14)
				SetSkillNum(FMath::RandRange(1, 4));
			else
				SetSkillNum(FMath::RandRange(1, 5));
		}
		else // npc can't cast strom magic
		{
			if (Player->GetMainPlayerController()->DialogueNum <= 5)
				SetSkillNum(FMath::RandRange(2, 3));
			else if (Player->GetMainPlayerController()->DialogueNum <= 14)
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

		UBlueprintGeneratedClass* LoadedBP = LoadObject<UBlueprintGeneratedClass>(GetWorld(), TEXT("/Game/Blueprints/MagicAttacks/Luko/1_GreenStormAttack.1_GreenStormAttack_C")); //�ʱ�ȭ �� �ϸ� ToSpawn�� �ʱ�ȭ���� ���� ���� �־��ٰ� ������
		if (this->GetName().Contains("Luko"))
		{
			switch (GetSkillNum())
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
			switch (GetSkillNum())
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
			switch (GetSkillNum())
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
			switch (GetSkillNum())
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
			switch (GetSkillNum())
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
					for (int i = 0; i < Player->GetNPCList().Num(); i++)
					{
						// �ٸ� npc�� �ν� ������ ���Ͱ� ������ �����ַ� ��, ù��° ���� ����
						if (Player->GetNPCList()[i]->AgroTargets.Num() != 0
							&& !UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("first")) 
						{
							//UE_LOG(LogTemp, Log, TEXT("AttackEnd - Go Help %s"), *this->GetName());
							MoveToTarget(Player->GetNPCList()[i]->AgroTargets[0]);
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
						(this->GetName().Contains("Momo")
						|| this->GetName().Contains("Luko")
						|| this->GetName().Contains("Vovo"))) 
					{
						spawnLocation = CombatTarget->GetActorLocation();
					}
						
					//��� ���� 4�� ��ų �� ��ġ���� ����
					if (GetSkillNum() == 4 && !this->GetName().Contains("Momo"))
					{
						spawnLocation = CombatTarget->GetActorLocation();
					}			
				}		

				MagicAttack = world->SpawnActor<AMagicSkill>(ToSpawn, spawnLocation, rotator, spawnParams);
		
				if (MagicAttack && CombatTarget)
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
				//UE_LOG(LogTemp, Log, TEXT("%f"), distance);
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
				if (Player->Momo->CombatTarget && Player->Momo->CombatTarget->GetName().Contains("Golem"))
				{
					AIController->MoveToActor(Player->Momo->CombatTarget);
				}
                if (Player->GetCombatTarget() && Player->GetCombatTarget()->GetName().Contains("Golem"))
                {
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

void AYaroCharacter::Smile() // ���� ǥ��
{
	this->bSmile = true;
}

void AYaroCharacter::UsualFace() // ��� ǥ��
{
	this->bSmile = false;
}