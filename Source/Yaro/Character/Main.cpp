// Fill out your copyright notice in the Description page of Project Settings.

#include "Main.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerStart.h"
#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Components/ArrowComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/world.h"

#include "Yaro/System/MainPlayerController.h"
#include "Yaro/System/GameManager.h"
#include "Yaro/System/DialogueManager.h"
#include "Yaro/System/NPCManager.h"
#include "Yaro/System/UIManager.h"
#include "Yaro/Character/MainAnimInstance.h"
#include "Yaro/Character/YaroCharacter.h"
#include "Yaro/Character/Enemies/Enemy.h"
#include "Yaro/Character/Enemies/Boss.h"
#include "Yaro/Structs/AttackSkillData.h"
#include "Yaro/MagicSkill.h"
#include "Yaro/ItemStorage.h"
#include "Yaro/DialogueUI.h"
#include "Yaro/Weapon.h"

const float MIN_STAMINA_TO_RUN = 30.f;
const float MIN_MANA_TO_CAST = 15.f;
const float HP_RECOVERY_AMOUNT = 5.f;
const float MP_RECOVERY_AMOUNT = 5.f;
const float SP_RECOVERY_AMOUNT = 1.f;
const float REVIVE_HP_AMOUNT = 50.f;

AMain::AMain()
{
	InitializeCamera();
	InitializeStats();

	// Configure character movement
	WalkSpeed = 350.f;
	RunSpeed = 600.f;
	GetCharacterMovement()->JumpZVelocity = 400.f;
	GetCharacterMovement()->SetWalkableFloorAngle(50.f);
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	CombatSphere = CreateSphereComponent(TEXT("CombatSphere"), 600.f, FVector(100.f, 0.f, 0.f));
	ItemSphere = CreateSphereComponent(TEXT("ItemSphere"), 80.f, FVector::ZeroVector);

	InitializeLevelData();
}

void AMain::InitializeCamera()
{
	//Create CameraBoom (pulls towards the player if there's a collision), �ݸ����� ������ ī�޶� �÷��̾������� ��� 
	CameraBoom = CreateAbstractDefaultSubobject<USpringArmComponent>(TEXT("Camera"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 500.f; //Camera follows at this distance
	CameraBoom->bUsePawnControlRotation = true; // Rotate arm based on controller

	// but npc, enemy�鵵 ���⿡ �ݸ������� �ش�Ǿ� ���� �÷��̰� �����ϹǷ� �ݸ��� �׽�Ʈ ��
	//CameraBoom->bDoCollisionTest = false;

	CameraBoom->SetWorldRotation(FRotator(-30.0f, 0.f, 0.0f));
	CameraBoom->SocketOffset.Z = 70.f;

	// Create FollowCamera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	// Attach the camera to the end of the boom and let the boom adjust to match
	// the controller orientation
	FollowCamera->bUsePawnControlRotation = false;

	// Set out turn rates for input
	BaseTurnRate = 20.f;
	BaseLookUpRate = 40.f;

	// Don't rotate when the controller rotates.
	// Let that just affect the camera.
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
}

void AMain::InitializeStats()
{
	//���� ���� �ʱ�ȭ
	PlayerStats.Add(EPlayerStat::Gender, 0.f);
	PlayerStats.Add(EPlayerStat::MaxHP, 300.f);
	PlayerStats.Add(EPlayerStat::HP, 300.f);
	PlayerStats.Add(EPlayerStat::MaxMP, 150.f);
	PlayerStats.Add(EPlayerStat::MP, 150.f);
	PlayerStats.Add(EPlayerStat::MaxSP, 300.f);
	PlayerStats.Add(EPlayerStat::SP, 300.f);
	PlayerStats.Add(EPlayerStat::Level, 1.f);
	PlayerStats.Add(EPlayerStat::Exp, 0.f);
	PlayerStats.Add(EPlayerStat::MaxExp, 60.f);
	PlayerStats.Add(EPlayerStat::PotionNum, 0.f);
}

USphereComponent* AMain::CreateSphereComponent(FName Name, float Radius, FVector RelativeLocation)
{
	USphereComponent* Sphere = CreateDefaultSubobject<USphereComponent>(Name);
	Sphere->SetupAttachment(GetRootComponent());
	Sphere->InitSphereRadius(Radius);
	Sphere->SetRelativeLocation(RelativeLocation);
	Sphere->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
	return Sphere;
}

void AMain::InitializeLevelData()
{
	LevelData.Add(1, FLevelStats(60.f, 300.f, 150.f, 300.f));
	LevelData.Add(2, FLevelStats(150.f, 350.f, 175.f, 325.f));
	LevelData.Add(3, FLevelStats(250.f, 450.f, 200.f, 350.f));
	LevelData.Add(4, FLevelStats(360.f, 600.f, 230.f, 375.f));
	LevelData.Add(5, FLevelStats(0.f, 700.f, 280.f, 400.f)); // ������ ������ ����ġ�� �ʿ� ������ MaxExp�� 0
}

// Called when the game starts or when spawned
void AMain::BeginPlay()
{
	if (GetWorld()->GetName().Contains("Start")) return;

	Super::BeginPlay();

	InitializeManagers();
	BindComponentEvents();

	if (this->GetName().Contains("Boy")) SetStat(EPlayerStat::Gender, 1);
	if (this->GetName().Contains("Girl")) SetStat(EPlayerStat::Gender, 2);

	// ������ ���� ����
	Storage = GetWorld()->SpawnActor<AItemStorage>(ObjectStorage);
}

void AMain::InitializeManagers()
{
	GameManager = Cast<UGameManager>(GetWorld()->GetGameInstance());
	if (GameManager)
	{
		DialogueManager = GameManager->GetDialogueManager();
		NPCManager = GameManager->GetNPCManager();
		UIManager = GameManager->GetUIManager();
	}
}

void AMain::BindComponentEvents()
{
	CombatSphere->OnComponentBeginOverlap.AddDynamic(this, &AMain::CombatSphereOnOverlapBegin);
	CombatSphere->OnComponentEndOverlap.AddDynamic(this, &AMain::CombatSphereOnOverlapEnd);
	ItemSphere->OnComponentBeginOverlap.AddDynamic(this, &AMain::ItemSphereOnOverlapBegin);
	ItemSphere->OnComponentEndOverlap.AddDynamic(this, &AMain::ItemSphereOnOverlapEnd);
}

void AMain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (GetLevel()->GetName().Contains("Start")) return;

    if (!MainPlayerController)
        MainPlayerController = Cast<AMainPlayerController>(GetController());

	if(!MainAnimInstance)
		MainAnimInstance = Cast<UMainAnimInstance>(GetMesh()->GetAnimInstance());

	if (CombatTarget) 
	{
		CombatTargetLocation = CombatTarget->GetActorLocation(); // ���� Ÿ���� ��ġ ���� �޾ƿ���
		UIManager->SetEnemyLocation(CombatTargetLocation); // ȭ��ǥ�� ü�� ���� ���� ����

        if (CombatTarget->GetEnemyMovementStatus() == EEnemyMovementStatus::EMS_Dead) // ���� ���� Ÿ���� �׾��ٸ�
        {
			if(Targets.Contains(CombatTarget)) 
				Targets.Remove(CombatTarget); //Ÿ���� ���� �� �迭���� ����

			UnsetCombatTarget();
        }
	}
}

// Called to bind functionality to input
void AMain::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	// �׼��� Ű�� �����ų� ���� ��� �ѹ��� ����
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMain::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("LMB", IE_Pressed, this, &AMain::LMBDown); // ���� ���콺 ��ư �ٿ�

	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &AMain::Attack);

	PlayerInputComponent->BindAction("Targeting", IE_Pressed, this, &AMain::Targeting);

	PlayerInputComponent->BindAction("SkipCombat", IE_Pressed, GameManager, &UGameManager::SkipCombat);

	PlayerInputComponent->BindAction("ToggleMenu", IE_Pressed, UIManager, &UUIManager::ToggleMenu);

	PlayerInputComponent->BindAction("TriggerNextDialogue", IE_Pressed, DialogueManager, &UDialogueManager::TriggerNextDialogue);

    PlayerInputComponent->BindAction("DisplayControlGuide", IE_Pressed, UIManager, &UUIManager::DisplayControlGuide);

	PlayerInputComponent->BindAction("Escape", IE_Pressed, GameManager, &UGameManager::EscapeToSafeLocation);

	PlayerInputComponent->BindAction("UsePotion", IE_Pressed, this, &AMain::UsePotion);

	PlayerInputComponent->BindAction("StartFirstDungeon", IE_Pressed, GameManager, &UGameManager::StartFirstDungeon);


	// Axis�� �� �����Ӹ��� ȣ��
	//��Ű �̸���, bind�� �Լ��� �ִ� Ŭ������ �ν��Ͻ�, bind�� �Լ��� �ּ�
	PlayerInputComponent->BindAxis("MoveForward", this, &AMain::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMain::MoveRight);
	PlayerInputComponent->BindAxis("Run", this, &AMain::Run);

	PlayerInputComponent->BindAxis("Turn", this, &AMain::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &AMain::LookUpAtRate);

	PlayerInputComponent->BindAxis("CameraZoom", this, &AMain::CameraZoom);
}

bool AMain::IsInAir()
{
	return MainAnimInstance->bIsInAir;
}

void AMain::Move(float Value, EAxis::Type Axis)
{
	if ((Controller != nullptr) && (Value != 0.0f) && bCanMove)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation(); // ȸ���� ��ȯ �Լ�
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(Axis);
		AddMovementInput(Direction, Value);

		if (bRunning && PlayerStats[EPlayerStat::SP] > 0.f) // �޸��� �ִ� ���� + ���¹̳��� 0�̻��� �� ���¹̳� ����
			AddSP(-1);
	}
}

// �� �����Ӹ��� Ű�� ���ȴ��� �� ���ȴ��� Ȯ�ε� ��
// Ű�� �� �������� Value�� 0
void AMain::MoveForward(float Value)
{
	Move(Value, EAxis::X);
}

void AMain::MoveRight(float Value)
{
	Move(Value, EAxis::Y);
}

void AMain::Run(float Value)
{
	float CurrentSP = PlayerStats[EPlayerStat::SP];
	if (!Value || CurrentSP <= 0.f) // ����ƮŰ �� ���� �ְų� ���¹̳��� 0 ������ ��
	{
		bRunning = false;
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed; //�ӵ� ����

		if (CurrentSP < PlayerStats[EPlayerStat::MaxSP] && !recoverySP) // ���¹̳� Full ���°� �ƴϸ�
		{ // ���¹̳� �ڵ� ȸ��
			if (!GetWorldTimerManager().IsTimerActive(SPTimer))
			{
				recoverySP = true;
				GetWorldTimerManager().SetTimer(SPTimer, this, &AMain::RecoverySP, SPDelay, true);
			}
		}
	}
	else if(!bRunning && CurrentSP >= MIN_STAMINA_TO_RUN)
	{
		bRunning = true;
		GetCharacterMovement()->MaxWalkSpeed = RunSpeed; // �ӵ� ����		
	}
}

void AMain::Jump()
{
	// �װų� ��ȭ ���� ���� ���� �Ұ�, �������� ������ ������ ���� �Ұ�
	if (MovementStatus != EMovementStatus::EMS_Dead
		&& !DialogueManager->IsDialogueUIVisible()
		&& bCanMove)
	{
		Super::Jump();
	}
}

void AMain::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AMain::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AMain::CameraZoom(const float Value)
{
	if (Value == 0.f || !Controller) return;

	// Ư�� �������� �� �Ұ���
	{
		if (DialogueManager->GetDialogueNum() == 0) return;

		if (UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("cave")
			&& DialogueManager->GetDialogueNum() == 19)
			return;

		if (DialogueManager->IsDialogueUIVisible() && DialogueManager->GetDialogueNum() == 11) 
			return;
	}
	
	const float NewTargetArmLength = CameraBoom->TargetArmLength + Value * ZoomStep;
	CameraBoom->TargetArmLength = FMath::Clamp(NewTargetArmLength, MinZoomLength, MaxZoomLength);
}

void AMain::LMBDown() //Left Mouse Button Down
{
	// Targeting Off
	if (CombatTarget)
	{
		UnsetCombatTarget();
	}

	// ���� ��� ��� ����
	if (DialogueManager->IsDialogueUIVisible() && DialogueManager->GetDialogueUI()->GetCurrentState() != 3
		&& !UIManager->IsMenuVisible())
	{
		if (DialogueManager->GetDialogueUI()->IsInputDisabled()) return;
		else DialogueManager->GetDialogueUI()->Interact();
	}


	if (!MainAnimInstance || !NormalMontage) return;
	if (MainAnimInstance->Montage_IsPlaying(NormalMontage) == true) return; // �ߺ� ��� ����

	// ������ ��ȣ�ۿ� ��Ÿ�� ����
	if (ActiveOverlappingItem && !EquippedWeapon) // �������� ������ ����, ������ ���Ⱑ ���� ��
	{
		PlayMontageWithItem(); // ������ ���� ��Ÿ�� ���
		MainAnimInstance->Montage_JumpToSection(FName("PickWand"), NormalMontage);
	}

	if (DialogueManager->GetDialogueNum() == 9)
	{
		// pick up the yellow stone
		if (ActiveOverlappingItem)
		{
			PlayMontageWithItem();
			MainAnimInstance->Montage_JumpToSection(FName("PickItem"), NormalMontage);
		}

		if (CurrentOverlappedActor && ItemInHand)
		{
			if (CurrentOverlappedActor->GetName().Contains("MovingStone")  // �������� ���Ͱ� �����̴� ���̰�
				&& ItemInHand->GetName().Contains("Yellow")) // �տ� �ִ� �������� yellow stone�� ��
			{
				PlayMontageWithItem();
				MainAnimInstance->Montage_JumpToSection(FName("PutStone"), NormalMontage); // put yellow stone
			}
		}
	}

	// ������ ���� ������
	if (CurrentOverlappedActor && CurrentOverlappedActor->GetName().Contains("DivinumPraesidium"))
	{
		if (DialogueManager->GetDialogueNum() == 21)
		{
			if (DialogueManager->GetDialogueUI()->GetSelectedReply() != 1 || DialogueManager->IsDialogueUIVisible()) return;

			PlayMontageWithItem();
			MainAnimInstance->Montage_JumpToSection(FName("PickStone"), NormalMontage); // �� ì���
		}
	}
}

void AMain::PlayMontageWithItem()
{
	bCanMove = false; // �̵� ���� �Ұ�

	if (MainAnimInstance->Montage_IsPlaying(NormalMontage) == true) return; // �ߺ� ��� ����

	FRotator LookAtYaw;

	if (DialogueManager->GetDialogueNum() == 9 && ItemInHand)
	{
		if (ItemInHand->GetName().Contains("Yellow") && CurrentOverlappedActor != nullptr)
			LookAtYaw = GetLookAtRotationYaw(CurrentOverlappedActor->GetActorLocation());
	}
	else if (DialogueManager->GetDialogueNum() == 21 && CurrentOverlappedActor->GetName().Contains("Divinum"))
	{
		LookAtYaw = GetLookAtRotationYaw(CurrentOverlappedActor->GetActorLocation());
	}
	else if(ActiveOverlappingItem)
		LookAtYaw = GetLookAtRotationYaw(ActiveOverlappingItem->GetActorLocation());

	SetActorRotation(LookAtYaw); // ������ �������� ȸ��

	// ��Ÿ�� ���
	MainAnimInstance->Montage_Play(NormalMontage);
}

void AMain::Attack()
{
	// Ư�� ��Ȳ�� ���� �Ұ�
	if (DialogueManager->GetDialogueNum() < 3 || DialogueManager->GetDialogueNum() > 20) return;

	if (EquippedWeapon && !bAttacking && MovementStatus != EMovementStatus::EMS_Dead 
		&& !DialogueManager->IsDialogueUIVisible())
	{
		SetSkillNum(MainPlayerController->WhichKeyDown()); // ���� Ű�� � ��ų ������� ����
		
		if (PlayerStats[EPlayerStat::Level] < GetSkillNum()) return;

		ToSpawn = AttackSkillData->FindRow<FAttackSkillData>("Attack", "")->Skills[GetSkillNum()-1];

		bAttacking = true;
		SetInterpToEnemy(true);

		if (MainAnimInstance && CombatMontage)
		{
            bCanMove = false; // �̵� ���� �Ұ�
			MainAnimInstance->Montage_Play(CombatMontage);
			MainAnimInstance->Montage_JumpToSection(FName("Attack"), CombatMontage);
			Spawn(); // ���� ����
		}
	}
}

void AMain::AttackEnd()
{
    bCanMove = true; // �̵� ����
	bAttacking = false;
	SetInterpToEnemy(false);
}

void AMain::CombatSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		AEnemy* Enemy = Cast<AEnemy>(OtherActor);
		if (Enemy)
		{
			bOverlappingCombatSphere = true;

			if(Targets.Contains(Enemy)) return;

			Targets.Add(Enemy); // Ÿ���� ���� �� �迭�� �߰�
		}
	}
}

void AMain::CombatSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		AEnemy* Enemy = Cast<AEnemy>(OtherActor);
		if (Enemy)
		{
			if (Targets.Contains(Enemy)) Targets.Remove(Enemy); //Ÿ���� ���� �� �迭���� ����

			if (Targets.Num() == 0) // Ÿ���� ���� ���Ͱ� ������
			{
				bOverlappingCombatSphere = false;
			}

			if (CombatTarget == Enemy) // ���� (Ÿ���õ�)���� Ÿ�ٰ� (���� ������ ���� ���Ͱ�)�����ϴٸ�
			{
				UnsetCombatTarget();
			}
		}
	}
}

void AMain::Targeting() //Targeting using Tap key
{
	if (bOverlappingCombatSphere) //There is a enemy in combatsphere
	{
		 //Ÿ���ε����� �� Ÿ�� ���� �� �� �̻��̸� �ٽ� 0���� �ʱ�ȭ
		if(Targets.Num() != 0) TargetIndex %= Targets.Num();

		//There is already exist targeted enemy, then targetArrow remove
		if (UIManager->IsTargetArrowVisible())
		{
			UIManager->RemoveTargetArrow();
			UIManager->RemoveEnemyHPBar();
		}

		if (Targets.Num() == 0) return; // Ÿ���� ���� ���Ͱ� ������ ����

		if (!bAutoTargeting) // ���� Ÿ����
		{
			// ���� ���ʹ� Ÿ���� �Ұ�
			if (Targets[TargetIndex]->GetEnemyMovementStatus() == EEnemyMovementStatus::EMS_Dead) return;

            CombatTarget = Targets[TargetIndex];
            TargetIndex++;
		}
		
		bHasCombatTarget = true;

		UIManager->DisplayTargetArrow();
		UIManager->DisplayEnemyHPBar();
	}
}

void AMain::UnsetCombatTarget()
{
	// ���� Ÿ�� ����
	CombatTarget = nullptr;
	bHasCombatTarget = false;

	if (UIManager->IsTargetArrowVisible())
	{// ȭ��ǥ �� ���� ü�¹� ����
		UIManager->RemoveTargetArrow();
		UIManager->RemoveEnemyHPBar();
	}
}

void AMain::Spawn() 
{
	float CurrentMP = PlayerStats[EPlayerStat::MP];
	if (ToSpawn && CurrentMP >= MIN_MANA_TO_CAST) // ���� ��뿡 �ʿ��� �ּ� MP�� 15
	{
		//If player have not enough MP, then player can't use magic
		float MPCost = 10.f + GetSkillNum() * 5;
        if (CurrentMP < MPCost) return;

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

				// 1, 3�� ���� ������ ������ �� ��ġ���� ����
				if (GetSkillNum() != 1 && GetSkillNum() != 3 && CombatTarget)
				{
					spawnLocation = CombatTarget->GetActorLocation();
				}

				if (MagicAttack.IsValid())
				{
					MagicAttack->Destroy();  // ������ MagicAttack ����
					MagicAttack.Reset();     // ����Ʈ ������ �ʱ�ȭ
				}

				MagicAttack = MakeWeakObjectPtr<AMagicSkill>(world->SpawnActor<AMagicSkill>(ToSpawn, spawnLocation, rotator, spawnParams));
				if (MagicAttack.IsValid())
				{
					MagicAttack->SetInstigator(MainPlayerController);

					// �����Է� �̵��ϴ� ������ ������ ���� �������� ���������.
					if ((GetSkillNum() == 1 || GetSkillNum() == 3) &&CombatTarget)
						MagicAttack->SetTarget(CombatTarget);
				}
				
			}
		}), 0.6f, false); // 0.6�� �� ����, �ݺ�X

		CurrentMP -= MPCost;
		SetStat(EPlayerStat::MP, CurrentMP);

		if(!GetWorldTimerManager().IsTimerActive(MPTimer))
			GetWorldTimerManager().SetTimer(MPTimer, this, &AMain::RecoveryMP, MPDelay, true);
	}
}

float AMain::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	float CurrentHP = PlayerStats[EPlayerStat::HP];
	if (CurrentHP - DamageAmount <= 0.f)
	{
		CurrentHP = 0.f;
		Die();
	}
	else
	{
		CurrentHP -= DamageAmount;
		// HP �ڵ� ȸ��
		if(!GetWorldTimerManager().IsTimerActive(HPTimer))
			GetWorldTimerManager().SetTimer(HPTimer, this, &AMain::RecoveryHP, HPDelay, true);	
	}

	SetStat(EPlayerStat::HP, CurrentHP);

	if (UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("boss")) // ���� ��������
	{
		MagicAttack = Cast<AMagicSkill>(DamageCauser);
		if (MagicAttack == nullptr) return DamageAmount;
		
		if (MagicAttack->GetCaster() == ECasterType::Boss) // ���� ���Ϳ��� ���� ����
		{
			for (auto NPC : NPCManager->GetNPCMap())
			{
				if (NPC.Value->GetAgroTargets().Num() == 0) // Npc�� �ν� ������ �ƹ��� ���� ��
				{
					AEnemy* BossEnemy = Cast<AEnemy>(UGameplayStatics::GetActorOfClass(GetWorld(), ABoss::StaticClass()));
					NPC.Value->MoveToTarget(BossEnemy); // ���� ���Ϳ��� �̵�
					NPC.Value->ClearPlayerFollowTimer();
					NPC.Value->GetAIController()->StopMovement();
				}
			}
		}
	}
	return DamageAmount;
}

void AMain::Die()
{
	if (MovementStatus == EMovementStatus::EMS_Dead) return;

	SetMovementStatus(EMovementStatus::EMS_Dead);

	// �ڵ� ȸ�� ����
	GetWorldTimerManager().ClearTimer(HPTimer);
	GetWorldTimerManager().ClearTimer(MPTimer);
	GetWorldTimerManager().ClearTimer(SPTimer);

	if (bAttacking) AttackEnd();

	if (MainAnimInstance && CombatMontage)
	{
		bCanMove = false;
		MainAnimInstance->Montage_Play(CombatMontage);
		MainAnimInstance->Montage_JumpToSection(FName("Death"));
	}
}

void AMain::DeathEnd()
{
	GetMesh()->bPauseAnims = true;
	GetMesh()->bNoSkeletonUpdate = true;

	FTimerHandle DeathTimer;
	GetWorldTimerManager().SetTimer(DeathTimer, this, &AMain::Revive, DeathDelay);
}

void AMain::Revive() // if player is dead, spawn player at the initial location
{
	if (GetWorld()->GetName().Contains("Boss"))
	{
		this->SetActorLocation(FVector(8.f, 1978.f, 184.f));
	}
	else
	{
		APlayerStart* PlayerStart = Cast<APlayerStart>(UGameplayStatics::GetActorOfClass(GetWorld(), APlayerStart::StaticClass()));
		if (!PlayerStart) return;

		this->SetActorLocation(PlayerStart->GetActorLocation());
	}

	if (MainAnimInstance && CombatMontage)
	{
		GetMesh()->bPauseAnims = false;
		MainAnimInstance->Montage_Play(CombatMontage);
		MainAnimInstance->Montage_JumpToSection(FName("Revival"));
		GetMesh()->bNoSkeletonUpdate = false;
		SetStat(EPlayerStat::HP, PlayerStats[EPlayerStat::HP] + REVIVE_HP_AMOUNT);
	}
}

void AMain::RevivalEnd()
{
	SetMovementStatus(EMovementStatus::EMS_Normal);

	// �ڵ� ȸ�� �簳
	GetWorldTimerManager().SetTimer(HPTimer, this, &AMain::RecoveryHP, HPDelay, true);
	GetWorldTimerManager().SetTimer(MPTimer, this, &AMain::RecoveryMP, MPDelay, true);
	GetWorldTimerManager().SetTimer(SPTimer, this, &AMain::RecoverySP, SPDelay, true);
}

void AMain::ItemSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		if (auto actor = Cast<AItem>(OtherActor)) return; // �������� �� �������̸� ����X
		if (auto actor = Cast<AYaroCharacter>(OtherActor)) return; // �������� �� npc�� ����X

		CurrentOverlappedActor = OtherActor;	
	}
}

void AMain::ItemSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (OtherActor == CurrentOverlappedActor)
    {
        CurrentOverlappedActor = nullptr;
	}
}

float AMain::GetStat(EPlayerStat StatName) const
{
	return PlayerStats[StatName];
}

void AMain::SetStat(EPlayerStat StatName, float Value)
{
	PlayerStats[StatName] = Value;

	if (StatName == EPlayerStat::Level)
	{
		FLevelStats* NewStats = LevelData.Find(Value);
		if (NewStats)
		{
			SetStat(EPlayerStat::MaxHP, NewStats->MaxHP);
			SetStat(EPlayerStat::MaxMP, NewStats->MaxMP);
			SetStat(EPlayerStat::MaxSP, NewStats->MaxSP);
			SetStat(EPlayerStat::MaxExp, NewStats->MaxExp);
		}
	}
}

void AMain::AddHP(float Value)
{
	float HP = PlayerStats[EPlayerStat::HP] + Value;
	SetStat(EPlayerStat::HP, FMath::Clamp(HP, 0.0f, PlayerStats[EPlayerStat::MaxHP]));
}

void AMain::RecoveryHP()
{
	float CurrentHP = PlayerStats[EPlayerStat::HP] + HP_RECOVERY_AMOUNT;

	if (CurrentHP >= PlayerStats[EPlayerStat::MaxHP])
	{
		CurrentHP = PlayerStats[EPlayerStat::MaxHP];
		GetWorldTimerManager().ClearTimer(HPTimer);
	}
	SetStat(EPlayerStat::HP, CurrentHP);
}

void AMain::RecoveryMP()
{
	float CurrentMP = PlayerStats[EPlayerStat::MP] + MP_RECOVERY_AMOUNT;

	if (CurrentMP >= PlayerStats[EPlayerStat::MaxMP])
	{
		CurrentMP = PlayerStats[EPlayerStat::MaxMP];
		GetWorldTimerManager().ClearTimer(MPTimer);
	}
	SetStat(EPlayerStat::MP, CurrentMP);
}

void AMain::RecoverySP()
{
	float CurrentSP = PlayerStats[EPlayerStat::SP] + SP_RECOVERY_AMOUNT;

	if (CurrentSP >= PlayerStats[EPlayerStat::MaxSP])
	{
		CurrentSP = PlayerStats[EPlayerStat::MaxSP];
		GetWorldTimerManager().ClearTimer(SPTimer);
		recoverySP = false;
	}
	SetStat(EPlayerStat::SP, CurrentSP);
}

void AMain::GainExp(float Value)
{
	if (PlayerStats[EPlayerStat::Level] == 5) return; // �ְ� ������ ���� ����

	float CurrentExp = PlayerStats[EPlayerStat::Exp];
	float MaxExp = PlayerStats[EPlayerStat::MaxExp];

	CurrentExp += Value;
	SetStat(EPlayerStat::Exp, CurrentExp);

	if (CurrentExp >= MaxExp) // ���� �������� ����ġ�� ��� ä���ٸ�
	{
		LevelUp();
	}
}

void AMain::LevelUp()
{
	float CurrentLevel = PlayerStats[EPlayerStat::Level];

	float CurrentExp = PlayerStats[EPlayerStat::Exp];
	float MaxExp = PlayerStats[EPlayerStat::MaxExp];

	CurrentLevel += 1; // ���� ��
	if (LevelUpSound)
		UAudioComponent* AudioComponent = UGameplayStatics::SpawnSound2D(this, LevelUpSound);

	// ����ġ ��ġ update
	if (CurrentExp == MaxExp) CurrentExp = 0.f;
	else CurrentExp -= MaxExp;

	SetStat(EPlayerStat::Exp, CurrentExp);

	if (CurrentLevel == 5) CurrentExp = MaxExp;

	// �ý��� �޼��� ǥ��
	switch ((int)CurrentLevel)
	{
		case 2:
			UIManager->SetSystemMessage(7);
			break;
		case 3:
			UIManager->SetSystemMessage(8);
			break;
		case 4:
			UIManager->SetSystemMessage(9);
			break;
		case 5:
			UIManager->SetSystemMessage(10);
			break;
	}

	SetStat(EPlayerStat::Level, CurrentLevel);

	FLevelStats* NewStats = LevelData.Find(CurrentLevel);
	if (NewStats)
	{
		SetStat(EPlayerStat::HP, NewStats->MaxHP);
		SetStat(EPlayerStat::MP, NewStats->MaxMP);
		SetStat(EPlayerStat::SP, NewStats->MaxSP);
	}

	FTimerHandle Timer;
	GetWorld()->GetTimerManager().SetTimer(Timer, FTimerDelegate::CreateLambda([&]()
		{
			UIManager->RemoveSystemMessage();
		}), 3.f, false);
}

const TMap<FString, TSubclassOf<class AItem>>* AMain::GetItemMap()
{
	if (Storage != nullptr) return &(Storage->ItemMap);
	return nullptr;
}

void AMain::RecoverWithLogo() // get school icon in second stage
{
	float CurrentHP = PlayerStats[EPlayerStat::HP] + 150.f;
	float CurrentMP = PlayerStats[EPlayerStat::MP] + 50.f;
	float CurrentSP = PlayerStats[EPlayerStat::SP] + 50.f;

	float MaxHP = PlayerStats[EPlayerStat::MaxHP];
	float MaxMP = PlayerStats[EPlayerStat::MaxMP];
	float MaxSP = PlayerStats[EPlayerStat::MaxSP];

	if (CurrentHP >= MaxHP) CurrentHP = MaxHP;
	if (CurrentMP >= MaxMP) CurrentMP = MaxMP;
	if (CurrentSP >= MaxSP) CurrentSP = MaxSP;

	SetStat(EPlayerStat::HP, CurrentHP);
	SetStat(EPlayerStat::MP, CurrentMP);
	SetStat(EPlayerStat::SP, CurrentSP);
}

void AMain::UsePotion()
{
	if (PlayerStats[EPlayerStat::PotionNum] <= 0 || DialogueManager->GetDialogueNum() < 3) return;

	SetStat(EPlayerStat::HP, PlayerStats[EPlayerStat::MaxHP]);
	SetStat(EPlayerStat::MP, PlayerStats[EPlayerStat::MaxMP]);
	SetStat(EPlayerStat::SP, PlayerStats[EPlayerStat::MaxSP]);
	SetStat(EPlayerStat::PotionNum, 0);
}