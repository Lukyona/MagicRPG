// Fill out your copyright notice in the Description page of Project Settings.

#include "Main.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/world.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/PlayerCameraManager.h"
#include "Yaro/Weapon.h"
#include "Components/SphereComponent.h"
#include "Yaro/Character/Enemies/Enemy.h"
#include "Components/ArrowComponent.h"
#include "Yaro/MagicSkill.h"
#include "Yaro/System/MainPlayerController.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Yaro/ItemStorage.h"
#include "Yaro/DialogueUI.h"
#include "Yaro/Character/YaroCharacter.h"
#include "Yaro/Character/MainAnimInstance.h"
#include "Yaro/Structs/AttackSkillData.h"
#include "AIController.h"
#include "Yaro/System/GameManager.h"
#include "Yaro/System/DialogueManager.h"
#include "Yaro/System/NPCManager.h"
#include "Yaro/System/UIManager.h"

// Sets default values
AMain::AMain()
{//��ħ CreateAbstractDefaultSubobject
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


	// Configure character movement
	WalkSpeed = 350.f;
	RunSpeed = 600.f;
	GetCharacterMovement()->JumpZVelocity = 400.f;
	GetCharacterMovement()->SetWalkableFloorAngle(50.f);
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	// ���� ���� ����
	CombatSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatSphere"));
	CombatSphere->SetupAttachment(GetRootComponent());
	CombatSphere->InitSphereRadius(600.f);
	CombatSphere->SetRelativeLocation(FVector(100.f, 0.f, 0.f));
	CombatSphere->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);

	bHasCombatTarget = false;
	targetIndex = 0;

	// ������ ��ȣ�ۿ� ���� ����
	ItemSphere = CreateDefaultSubobject<USphereComponent>(TEXT("ItemSphere"));
	ItemSphere->SetupAttachment(GetRootComponent());
	ItemSphere->InitSphereRadius(80.f);
	ItemSphere->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);


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


	HPDelay = 3.f;
	MPDelay = 2.f;
	SPDelay = 0.5f;

	DeathDelay = 3.f;

	MovementStatus = EMovementStatus::EMS_Normal;

	bESCDown = false;
}

// Called when the game starts or when spawned
void AMain::BeginPlay()
{
	if (GetWorld()->GetName().Contains("Start")) return;

	Super::BeginPlay();

	GameManager = Cast<UGameManager>(GetWorld()->GetGameInstance());
	if (GameManager)
	{
		DialogueManager = GameManager->GetDialogueManager();
		NPCManager = GameManager->GetNPCManager();
		UIManager = GameManager->GetUIManager();
	}

	CombatSphere->OnComponentBeginOverlap.AddDynamic(this, &AMain::CombatSphereOnOverlapBegin);
	CombatSphere->OnComponentEndOverlap.AddDynamic(this, &AMain::CombatSphereOnOverlapEnd);

	ItemSphere->OnComponentBeginOverlap.AddDynamic(this, &AMain::ItemSphereOnOverlapBegin);
	ItemSphere->OnComponentEndOverlap.AddDynamic(this, &AMain::ItemSphereOnOverlapEnd);

	// ���� ĳ���� ���� ���� ����
	if (this->GetName().Contains("Boy")) SetStat(EPlayerStat::Gender, 1);
	if (this->GetName().Contains("Girl")) SetStat(EPlayerStat::Gender, 2);

	// ������ ���� ����
	Storage = GetWorld()->SpawnActor<AItemStorage>(ObjectStorage);
}

// Called every frame
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
			for (int i = 0; i < Targets.Num(); i++)
			{
				if (CombatTarget == Targets[i]) //already exist
				{
					Targets.Remove(CombatTarget); //Ÿ���� ���� �� �迭���� ����
				}
			}
			// ���� Ÿ�� ����
            CombatTarget = nullptr;
            bHasCombatTarget = false;

            if (UIManager->IsTargetArrowVisible()) 
            {// ȭ��ǥ �� ���� ü�¹� ����
				UIManager->RemoveTargetArrow();
				UIManager->RemoveEnemyHPBar();
            }
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
	PlayerInputComponent->BindAction("LMB", IE_Released, this, &AMain::LMBUp);

	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &AMain::Attack);

	PlayerInputComponent->BindAction("Targeting", IE_Pressed, this, &AMain::Targeting);

	PlayerInputComponent->BindAction("SkipCombat", IE_Pressed, GameManager, &UGameManager::SkipCombat);

	PlayerInputComponent->BindAction("ESC", IE_Pressed, this, &AMain::ESCDown);
	PlayerInputComponent->BindAction("ESC", IE_Released, this, &AMain::ESCUp);

	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &AMain::StartDialogue);

    PlayerInputComponent->BindAction("ShowControlGuide", IE_Pressed, this, &AMain::ShowControlGuide);

	PlayerInputComponent->BindAction("Escape", IE_Pressed, this, &AMain::Escape);

	PlayerInputComponent->BindAction("LevelCheat", IE_Pressed, this, &AMain::SetLevel5);

	PlayerInputComponent->BindAction("UsePotion", IE_Pressed, this, &AMain::UsePotion);

	PlayerInputComponent->BindAction("Start", IE_Pressed, this, &AMain::StartMisson);



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

// �� �����Ӹ��� Ű�� ���ȴ��� �� ���ȴ��� Ȯ�ε� ��
// Ű�� �� �������� Value�� 0
void AMain::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f) && bCanMove)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation(); // ȸ���� ��ȯ �Լ�
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);

		if (bRunning && PlayerStats[EPlayerStat::SP] > 0.f) // �޸��� �ִ� ���� + ���¹̳��� 0�̻��� �� ���¹̳� ����
			AddSP(-1);
	}
}

void AMain::MoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f) && bCanMove)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation(); // ȸ���� ��ȯ �Լ�
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);

		if (bRunning && PlayerStats[EPlayerStat::SP] > 0.f)// �޸��� �ִ� ���� + ���¹̳��� 0�̻��� �� ���¹̳� ����
			AddSP(-1);
	}
}

void AMain::Run(float Value)
{
	float currentSP = PlayerStats[EPlayerStat::SP];
	if (!Value || currentSP <= 0.f) // ����ƮŰ �� ���� �ְų� ���¹̳��� 0 ������ ��
	{
		bRunning = false;
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed; //�ӵ� ����

		if (currentSP < PlayerStats[EPlayerStat::MaxSP] && !recoverySP) // ���¹̳� Full ���°� �ƴϸ�
		{ // ���¹̳� �ڵ� ȸ��
			if (!GetWorldTimerManager().IsTimerActive(SPTimer))
			{
				recoverySP = true;
				GetWorldTimerManager().SetTimer(SPTimer, this, &AMain::RecoverySP, SPDelay, true);
			}
		}
	}
	else if(!bRunning && currentSP >= 30.f) // ����ƮŰ�� �����ְ� ���¹̳� 5 �̻� �޸��� ���°� �ƴϸ�
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
	bLMBDown = true;
	// Targeting Off
	if (CombatTarget)
	{
		if (UIManager->IsTargetArrowVisible())
		{ // ȭ��ǥ �� ü�¹� ����
			UIManager->RemoveTargetArrow();
			UIManager->RemoveEnemyHPBar();
		}
		// ���� Ÿ�� ����
		CombatTarget = nullptr;
		bHasCombatTarget = false;
	}

	// ���� ��� ��� ����
	if (DialogueManager->IsDialogueUIVisible()
		&& DialogueManager->GetDialogueUI()->GetCurrentState() != 3
		&& !UIManager->IsMenuVisible())
	{
		if (DialogueManager->GetDialogueUI()->IsInputDisabled()) return;
		else DialogueManager->GetDialogueUI()->Interact();
	}


	if (!MainAnimInstance || !NormalMontage) return;
	if (MainAnimInstance->Montage_IsPlaying(NormalMontage) == true) return; // �ߺ� ��� ����

	// ������ ��ȣ�ۿ� ��Ÿ�� ����
	// �������� ������ ����, ������ ���Ⱑ ���� ��
	if (ActiveOverlappingItem && !EquippedWeapon)
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

void AMain::LMBUp()
{
	bLMBDown = false;
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
	if (DialogueManager->GetDialogueNum() < 3 || DialogueManager->GetDialogueNum() > 20 ) return;

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
			for (int i = 0; i < Targets.Num(); i++)
			{
				if (Enemy == Targets[i]) //already exist
					return;
			}
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
			for (int i = 0; i < Targets.Num(); i++)
			{
				if (Enemy == Targets[i]) // already exist
				{
					Targets.Remove(Enemy); //Ÿ���� ���� �� �迭���� ����
				}
			}

			if (Targets.Num() == 0) // Ÿ���� ���� ���Ͱ� ������
			{
				bOverlappingCombatSphere = false;
			}

			if (CombatTarget == Enemy) // ���� (Ÿ���õ�)���� Ÿ�ٰ� (���� ������ ���� ���Ͱ�)�����ϴٸ�
			{
				// ȭ��ǥ �� ü�¹� ���� & ���� Ÿ�� ����
				UIManager->RemoveTargetArrow();
				UIManager->RemoveEnemyHPBar();
				CombatTarget = nullptr;
				bHasCombatTarget = false;
			}
		}
	}
}

void AMain::Targeting() //Targeting using Tap key
{
	if (bOverlappingCombatSphere) //There is a enemy in combatsphere
	{
		//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Targeting"));
		if (targetIndex >= Targets.Num()) //Ÿ���ε����� �� Ÿ�� ���� �� �� �̻��̸� �ٽ� 0���� �ʱ�ȭ
		{
			targetIndex = 0;
		}

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
			if (Targets[targetIndex]->GetEnemyMovementStatus() == EEnemyMovementStatus::EMS_Dead) return;

            CombatTarget = Targets[targetIndex];
            targetIndex++;
		}
		
		bHasCombatTarget = true;

		UIManager->DisplayTargetArrow();
		UIManager->DisplayEnemyHPBar();
	}
}

void AMain::Spawn() 
{
	float currentMP = PlayerStats[EPlayerStat::MP];
	if (ToSpawn && currentMP >= 15) // ���� ��뿡 �ʿ��� �ּ� MP�� 15
	{
		//If player have not enough MP, then player can't use magic
		float MPCost = 10.f + GetSkillNum() * 5;
        if (currentMP < MPCost) return;

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

				// �����Է� �̵��ϴ� ������ ������ ���� �������� ���������.
				if ((GetSkillNum() == 1 || GetSkillNum() == 3) && MagicAttack.IsValid() && CombatTarget)
					MagicAttack->Target = CombatTarget;
			}
		}), 0.6f, false); // 0.6�� �� ����, �ݺ�X

		currentMP -= MPCost;
		SetStat(EPlayerStat::MP, currentMP);

		if(!GetWorldTimerManager().IsTimerActive(MPTimer))
			GetWorldTimerManager().SetTimer(MPTimer, this, &AMain::RecoveryMP, MPDelay, true);
	}
}

float AMain::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	float currentHP = PlayerStats[EPlayerStat::HP];
	if (currentHP - DamageAmount <= 0.f)
	{
		currentHP = 0.f;
		Die();
		if (DamageCauser) // Ȯ�� �ʿ�
		{
			AEnemy* Enemy = Cast<AEnemy>(DamageCauser);
			if (Enemy)
			{
				Enemy->SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Idle);
			}
		}
	}
	else
	{
		currentHP -= DamageAmount;
		// HP �ڵ� ȸ��
		if(!GetWorldTimerManager().IsTimerActive(HPTimer))
			GetWorldTimerManager().SetTimer(HPTimer, this, &AMain::RecoveryHP, HPDelay, true);	
	}

	SetStat(EPlayerStat::HP, currentHP);

	if (UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("boss")) // ���� ��������
	{
		MagicAttack = Cast<AMagicSkill>(DamageCauser);
		if (MagicAttack == nullptr) return DamageAmount;

		int TargetIndex = MagicAttack->index;

		if (TargetIndex == 11) // ���� ���Ϳ��� ���� ����
		{
			for (auto NPC : NPCManager->GetNPCMap())
			{
				if (NPC.Value->GetAgroTargets().Num() == 0) // Npc�� �ν� ������ �ƹ��� ���� ��
				{
					AEnemy* BossEnemy = Cast<AEnemy>(UGameplayStatics::GetActorOfClass(GetWorld(), NPC.Value->GetBoss()));
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

	// �ڵ� ȸ�� ����
	GetWorldTimerManager().ClearTimer(HPTimer);
	GetWorldTimerManager().ClearTimer(MPTimer);
	GetWorldTimerManager().ClearTimer(SPTimer);

	if (bAttacking) AttackEnd();

	SetMovementStatus(EMovementStatus::EMS_Dead);

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
	GetWorldTimerManager().SetTimer(DeathTimer, this, &AMain::Revive, DeathDelay);
}

void AMain::Revive() // if player is dead, spawn player at the initial location
{
	if(DialogueManager->GetDialogueNum() <= 4) // first dungeon
		this->SetActorLocation(FVector(-192.f, 5257.f, 3350.f));
	else if(DialogueManager->GetDialogueNum() <= 15) // second dungeon
        this->SetActorLocation(FVector(3910.f, -3920.f,-2115.f));
	else											// boss level
		this->SetActorLocation(FVector(8.f, 1978.f, 184.f));

	if (MainAnimInstance && CombatMontage)
	{
		GetMesh()->bPauseAnims = false;
		MainAnimInstance->Montage_Play(CombatMontage);
		MainAnimInstance->Montage_JumpToSection(FName("Revival"));
		GetMesh()->bNoSkeletonUpdate = false;
		SetStat(EPlayerStat::HP, PlayerStats[EPlayerStat::HP] + 50);
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

void AMain::StartDialogue()
{
	if (!MainPlayerController)
		MainPlayerController = Cast<AMainPlayerController>(GetController());

	if (DialogueManager->GetDialogueUI()->GetCurrentState() != 3 && !UIManager->IsMenuVisible())
	{   
	    if (DialogueManager->GetDialogueUI()->IsInputDisabled()) return;
	    else DialogueManager->GetDialogueUI()->Interact();
	}
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

void AMain::InitializeStats()
{
	if (PlayerStats[EPlayerStat::SP] < PlayerStats[EPlayerStat::MaxSP] && !recoverySP)
	{
		recoverySP = true;
		GetWorldTimerManager().SetTimer(SPTimer, this, &AMain::RecoverySP, SPDelay, true);
	}

	if (PlayerStats[EPlayerStat::HP] < PlayerStats[EPlayerStat::MaxHP])
		GetWorldTimerManager().SetTimer(HPTimer, this, &AMain::RecoveryHP, HPDelay, true);

	if (PlayerStats[EPlayerStat::MP] < PlayerStats[EPlayerStat::MaxMP])
		GetWorldTimerManager().SetTimer(MPTimer, this, &AMain::RecoveryMP, MPDelay, true);
}

float AMain::GetStat(EPlayerStat StatName) const
{
	if (PlayerStats.Contains(StatName))
	{
		return PlayerStats[StatName];
	}
	return 0.f;
}

void AMain::SetStat(EPlayerStat StatName, float Value)
{
	if (PlayerStats.Contains(StatName))
	{
		PlayerStats[StatName] = Value;
	}
}

void AMain::RecoveryHP()
{
	float currentHP = PlayerStats[EPlayerStat::HP] + 5.f;

	if (currentHP >= PlayerStats[EPlayerStat::MaxHP])
	{
		currentHP = PlayerStats[EPlayerStat::MaxHP];
		GetWorldTimerManager().ClearTimer(HPTimer);
	}
	SetStat(EPlayerStat::HP, currentHP);
}

void AMain::RecoveryMP()
{
	float currentMP = PlayerStats[EPlayerStat::MP] + 5.f;

	if (currentMP >= PlayerStats[EPlayerStat::MaxMP])
	{
		currentMP = PlayerStats[EPlayerStat::MaxMP];
		GetWorldTimerManager().ClearTimer(MPTimer);
	}
	SetStat(EPlayerStat::MP, currentMP);
}

void AMain::RecoverySP()
{
	float currentSP = PlayerStats[EPlayerStat::SP] +1.f;

	if (currentSP >= PlayerStats[EPlayerStat::MaxSP])
	{
		currentSP = PlayerStats[EPlayerStat::MaxSP];
		GetWorldTimerManager().ClearTimer(SPTimer);
		recoverySP = false;
	}
	SetStat(EPlayerStat::SP, currentSP);
}

void AMain::ESCDown()
{
	bESCDown = true;

	if (MainPlayerController)
	{
		UIManager->ToggleMenu();
	}
}

void AMain::ESCUp()
{
	bESCDown = false;
}

void AMain::GainExp(float Value)
{
	float currentLevel = PlayerStats[EPlayerStat::Level];
	if (currentLevel == 5) return; // �ְ� ������ ���� ����

	float currentExp = PlayerStats[EPlayerStat::Exp];
	float MaxExp = PlayerStats[EPlayerStat::MaxExp];

	currentExp += Value;

	float currentHP = PlayerStats[EPlayerStat::HP];
	float MaxHP = PlayerStats[EPlayerStat::MaxHP];
	float currentMP = PlayerStats[EPlayerStat::MP];
	float MaxMP = PlayerStats[EPlayerStat::MaxMP];
	float currentSP = PlayerStats[EPlayerStat::SP];
	float MaxSP = PlayerStats[EPlayerStat::MaxSP];

	if (currentExp >= MaxExp) // ���� �������� ����ġ�� ��� ä���ٸ�
	{
		currentLevel += 1; // ���� ��
        if (LevelUpSound)
            UAudioComponent* AudioComponent = UGameplayStatics::SpawnSound2D(this, LevelUpSound);

		// ����ġ ��ġ update
		if (currentExp == MaxExp)
		{
			currentExp = 0.f;
		}
		else
		{
			currentExp -= MaxExp;
		}

		if (currentLevel == 5) currentExp = MaxExp;

		// ������ ���� ���� �ɷ�ġ �ִ�ġ �ʱ�ȭ
		switch ((int)currentLevel)
		{
			case 2:
				MaxExp = 150.f;
				UIManager->SetSystemMessage(7);
				MaxHP = 350.f;
				MaxMP = 175.f;
				MaxSP = 325.f;
				break;
			case 3:
				MaxExp = 250.f;
				UIManager->SetSystemMessage(8);
                MaxHP = 450.f;
                MaxMP = 200.f;
                MaxSP = 350.f;
				break;
			case 4:
				MaxExp = 360.f;
				UIManager->SetSystemMessage(9);
                MaxHP = 600.f;
                MaxMP = 230.f;
                MaxSP = 375.f;
				break;
            case 5:
				UIManager->SetSystemMessage(10);
                MaxHP = 700.f;
                MaxMP = 280.f;
                MaxSP = 400.f;
                break;
		}

		currentHP += 100.f;
		currentMP += 50.f;
		currentSP += 100.f;

		if (currentHP > MaxHP) currentHP = MaxHP;
		if (currentMP > MaxMP) currentMP = MaxMP;
		if (currentSP > MaxSP) currentSP = MaxSP;


		SetStat(EPlayerStat::HP, currentHP);
		SetStat(EPlayerStat::MaxHP, MaxHP);
		SetStat(EPlayerStat::MP, currentMP);
		SetStat(EPlayerStat::MaxMP, MaxMP);
		SetStat(EPlayerStat::SP, currentSP);
		SetStat(EPlayerStat::MaxSP, MaxSP);
		SetStat(EPlayerStat::Level, currentLevel);
		SetStat(EPlayerStat::MaxExp, MaxExp);

        FTimerHandle Timer;
        GetWorld()->GetTimerManager().SetTimer(Timer, FTimerDelegate::CreateLambda([&]()
        {
				UIManager->RemoveSystemMessage();
        }), 3.f, false);
	}

	SetStat(EPlayerStat::Exp, currentExp);

}

const TMap<FString, TSubclassOf<class AItem>>* AMain::GetItemMap()
{
	if (Storage != nullptr) return &(Storage->ItemMap);
	return nullptr;
}

void AMain::StartMisson()
{
	if (DialogueManager->GetDialogueNum() == 3 && UIManager->IsSystemMessageVisible())
	{
		UIManager->RemoveSystemMessage();

		MainPlayerController->SetCinematicMode(false, true, true);

		NPCManager->GetNPC("Vovo")->MoveToLocation();
		NPCManager->GetNPC("Vivi")->MoveToLocation();
		NPCManager->GetNPC("Zizi")->MoveToLocation();

		NPCManager->GetNPC("Momo")->MoveToPlayer();
		NPCManager->GetNPC("Luko")->MoveToPlayer();

		FString SoundPath = TEXT("/Game/SoundEffectsAndBgm/the-buccaneers-haul.the-buccaneers-haul");
		USoundBase* LoadedSound = LoadObject<USoundBase>(nullptr, *SoundPath);
		if (LoadedSound)
		{
			UGameplayStatics::PlaySound2D(this, LoadedSound);
		}
	}
}

void AMain::ShowControlGuide()
{
	if (DialogueManager->GetDialogueNum() < 2 || UIManager->GetSystemMessageNum() < 3) return;

	if (UIManager->IsControlGuideVisible()) UIManager->RemoveControlGuide();
	else UIManager->DisplayControlGuide();
}

void AMain::Escape() // ��� Ż��, ���� �̵�
{
	if (DialogueManager->GetDialogueNum() >= 6
		&& !DialogueManager->IsDialogueUIVisible()
		&& MovementStatus != EMovementStatus::EMS_Dead)
	{
		if (DialogueManager->GetDialogueNum() <= 8)
		{
            SetActorLocation(FVector(4620.f, -3975.f, -2117.f));
		}
		else if (DialogueManager->GetDialogueNum() <= 11)
		{
            SetActorLocation(FVector(5165.f, -2307.f, -2117.f));
		}
		else if(DialogueManager->GetDialogueNum() <= 15)
		{
			SetActorLocation(FVector(2726.f, -3353.f, -500.f));
		}
	}
}

void AMain::RecoverWithLogo() // get school icon in second stage
{
	float currentHP = PlayerStats[EPlayerStat::HP] + 150.f;
	float currentMP = PlayerStats[EPlayerStat::MP] + 50.f;
	float currentSP = PlayerStats[EPlayerStat::SP] + 50.f;

	float MaxHP = PlayerStats[EPlayerStat::MaxHP];
	float MaxMP = PlayerStats[EPlayerStat::MaxMP];
	float MaxSP = PlayerStats[EPlayerStat::MaxSP];

	if (currentHP >= MaxHP) currentHP = MaxHP;
	if (currentMP >= MaxMP) currentMP = MaxMP;
	if (currentSP >= MaxSP) currentSP = MaxSP;

	SetStat(EPlayerStat::HP, currentHP);
	SetStat(EPlayerStat::MP, currentMP);
	SetStat(EPlayerStat::SP, currentSP);
}

void AMain::SetLevel5() // level cheat, ��� �ִ� ���� ����
{
	if (DialogueManager->GetDialogueNum() < 3 || PlayerStats[EPlayerStat::Level] == 5) return;

	if (LevelUpSound != nullptr)
		UAudioComponent* AudioComponent = UGameplayStatics::SpawnSound2D(this, LevelUpSound);

	SetStat(EPlayerStat::MaxHP, 700.f);
	SetStat(EPlayerStat::MaxMP, 280.f);
	SetStat(EPlayerStat::MaxSP, 400.f);
	SetStat(EPlayerStat::HP, 700.f);
	SetStat(EPlayerStat::MP, 480.f);
	SetStat(EPlayerStat::SP, 400.f);
	SetStat(EPlayerStat::Level, 5);
	SetStat(EPlayerStat::Exp, PlayerStats[EPlayerStat::MaxExp]);

	if (!UIManager->IsSystemMessageVisible())
	{
		UIManager->SetSystemMessage(16);
		FTimerHandle Timer;
		GetWorld()->GetTimerManager().SetTimer(Timer, FTimerDelegate::CreateLambda([&]()
		{
				UIManager->RemoveSystemMessage();
		}), 3.f, false);
	}
}

void AMain::UsePotion()
{
	if (PlayerStats[EPlayerStat::PotionNum] <= 0 || DialogueManager->GetDialogueNum() < 3) return;

	SetStat(EPlayerStat::HP, PlayerStats[EPlayerStat::MaxHP]);
	SetStat(EPlayerStat::MP, PlayerStats[EPlayerStat::MaxMP]);
	SetStat(EPlayerStat::SP, PlayerStats[EPlayerStat::MaxSP]);
	SetStat(EPlayerStat::PotionNum, 0);
}