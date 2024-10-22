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
	PlayerStats.Add("Gender", 0.f);
	PlayerStats.Add("MaxHP", 300.f);
	PlayerStats.Add("HP", 300.f);
	PlayerStats.Add("MaxMP", 150.f);
	PlayerStats.Add("MP", 150.f);
	PlayerStats.Add("MaxSP", 300.f);
	PlayerStats.Add("SP", 300.f);
	PlayerStats.Add("Level", 1.f);
	PlayerStats.Add("Exp", 0.f);
	PlayerStats.Add("MaxExp", 60.f);
	PlayerStats.Add("PotionNum", 0.f);


	HPDelay = 3.f;
	MPDelay = 2.f;
	SPDelay = 0.5f;

	DeathDelay = 3.f;

	MovementStatus = EMovementStatus::EMS_Normal;

	bESCDown = false;

}

bool AMain::IsInAir()
{
	return MainAnimInstance->bIsInAir;;
}

// Called when the game starts or when spawned
void AMain::BeginPlay()
{
	Super::BeginPlay();

	GameManager = Cast<UGameManager>(GetWorld()->GetGameInstance());
	if (GameManager)
	{
		DialogueManager = GameManager->GetDialogueManager();
		UIManager = GameManager->GetUIManager();
		NPCManager = GameManager->GetNPCManager();
		if (NPCManager)
			NPCManager->InitializeNPCs();
	}

	CombatSphere->OnComponentBeginOverlap.AddDynamic(this, &AMain::CombatSphereOnOverlapBegin);
	CombatSphere->OnComponentEndOverlap.AddDynamic(this, &AMain::CombatSphereOnOverlapEnd);

	ItemSphere->OnComponentBeginOverlap.AddDynamic(this, &AMain::ItemSphereOnOverlapBegin);
	ItemSphere->OnComponentEndOverlap.AddDynamic(this, &AMain::ItemSphereOnOverlapEnd);

	// ���� ĳ���� ���� ���� ����
	if (this->GetName().Contains("Boy")) SetStat("Gender", 1);
	if (this->GetName().Contains("Girl")) SetStat("Gender", 2);

	// ������ ���� ����
	Storage = GetWorld()->SpawnActor<AItemStorage>(ObjectStorage);
}

// Called every frame
void AMain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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

	PlayerInputComponent->BindAction("SkipCombat", IE_Pressed, this, &AMain::SkipCombat);

	PlayerInputComponent->BindAction("ESC", IE_Pressed, this, &AMain::ESCDown);
	PlayerInputComponent->BindAction("ESC", IE_Released, this, &AMain::ESCUp);

	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &AMain::StartDialogue);

    PlayerInputComponent->BindAction("ShowManual", IE_Pressed, this, &AMain::ShowManual);

	PlayerInputComponent->BindAction("Escape", IE_Pressed, this, &AMain::Escape);

	PlayerInputComponent->BindAction("LevelCheat", IE_Pressed, this, &AMain::SetLevel5);

	PlayerInputComponent->BindAction("UsePotion", IE_Pressed, this, &AMain::UsePotion);


	// Axis�� �� �����Ӹ��� ȣ��
	//��Ű �̸���, bind�� �Լ��� �ִ� Ŭ������ �ν��Ͻ�, bind�� �Լ��� �ּ�
	PlayerInputComponent->BindAxis("MoveForward", this, &AMain::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMain::MoveRight);
	PlayerInputComponent->BindAxis("Run", this, &AMain::Run);

	PlayerInputComponent->BindAxis("Turn", this, &AMain::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &AMain::LookUpAtRate);

	PlayerInputComponent->BindAxis("CameraZoom", this, &AMain::CameraZoom);
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

		if (bRunning && SP > 0.f) // �޸��� �ִ� ���� + ���¹̳��� 0�̻��� �� ���¹̳� ����
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

		if (bRunning && SP > 0.f)// �޸��� �ִ� ���� + ���¹̳��� 0�̻��� �� ���¹̳� ����
			AddSP(-1);
	}
}

void AMain::Run(float Value)
{
	if (!Value || SP <= 0.f) // ����ƮŰ �� ���� �ְų� ���¹̳��� 0 ������ ��
	{
		bRunning = false;
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed; //�ӵ� ����

		if (SP < MaxSP && !recoverySP) // ���¹̳� Full ���°� �ƴϸ�
		{ // ���¹̳� �ڵ� ȸ��
			if (!GetWorldTimerManager().IsTimerActive(SPTimer))
			{
				recoverySP = true;
				GetWorldTimerManager().SetTimer(SPTimer, this, &AMain::RecoverySP, SPDelay, true);
			}
		}
	}
	else if(!bRunning && SP >= 5.f) // ����ƮŰ�� �����ְ� ���¹̳� 5 �̻� �޸��� ���°� �ƴϸ�
	{
		bRunning = true;
		GetCharacterMovement()->MaxWalkSpeed = RunSpeed; // �ӵ� ����		
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

	// ������ ��ȣ�ۿ� ��Ÿ�� ����
	{
		// �������� ������ ����, ������ ���Ⱑ ���� ��
		if (ActiveOverlappingItem && !EquippedWeapon)
		{
			if (MainAnimInstance && NormalMontage)
			{
				PlayMontageWithItem(); // ������ ���� ��Ÿ�� ���
				MainAnimInstance->Montage_JumpToSection(FName("PickWand"), NormalMontage);
			}
		}

		// pick up the yellow stone
		if (ActiveOverlappingItem && DialogueManager->GetDialogueNum() == 9)
		{
			if (MainAnimInstance && NormalMontage)
			{
				PlayMontageWithItem();
				MainAnimInstance->Montage_JumpToSection(FName("PickItem"), NormalMontage);
			}
		}

		if (CurrentOverlappedActor && ItemInHand)
		{
			if (CurrentOverlappedActor->GetName().Contains("MovingStone")  // �������� ���Ͱ� �����̴� ���̰�
				&& ItemInHand->GetName().Contains("Yellow")) // �տ� �ִ� �������� yellow stone�� ��
			{
				if (DialogueManager->GetDialogueNum() == 9)
				{
					if (MainAnimInstance && NormalMontage)
					{
						PlayMontageWithItem();
						MainAnimInstance->Montage_JumpToSection(FName("PutStone"), NormalMontage); // put yellow stone
					}
				}
			}
		}

		// ������ ���� ������
		if (CurrentOverlappedActor && CurrentOverlappedActor->GetName().Contains("DivinumPraesidium"))
		{
			if (DialogueManager->GetDialogueNum() == 21 && MainAnimInstance && NormalMontage)
			{
				if (DialogueManager->GetDialogueUI()->SelectedReply != 1 || DialogueManager->IsDialogueUIVisible()) return;

				PlayMontageWithItem();
				MainAnimInstance->Montage_JumpToSection(FName("PickStone"), NormalMontage); // �� ì���
			}
		}
	}

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
		&& DialogueManager->GetDialogueUI()->CurrentState != 3
		&& !UIManager->IsMenuVisible())
	{
		if (DialogueManager->GetDialogueUI()->bDisableMouseAndKeyboard) return;
		else DialogueManager->GetDialogueUI()->Interact();
	}
}

void AMain::LMBUp()
{
	bLMBDown = false;
}

void AMain::Attack()
{
	// Ư�� ��Ȳ�� ���� �Ұ�
	if (DialogueManager->GetDialogueNum() < 3 || DialogueManager->GetDialogueNum() > 20 ) return;

	if (EquippedWeapon && !bAttacking && MovementStatus != EMovementStatus::EMS_Dead 
		&& !DialogueManager->IsDialogueUIVisible())
	{
		SetSkillNum(MainPlayerController->WhichKeyDown()); // ���� Ű�� � ��ų ������� ����
		
		if (Level < GetSkillNum()) return;

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

void AMain::Spawn() //Spawn Magic
{
	if (ToSpawn && MP >= 15) // ���� ��뿡 �ʿ��� �ּ� MP�� 15
	{
		//If player have not enough MP, then player can't use magic
		float MPCost = 10.f + GetSkillNum() * 5;
        if (MP < MPCost) return;

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

		MP -= MPCost;
		SetStat("MP", MP);

		if(!GetWorldTimerManager().IsTimerActive(MPTimer))
			GetWorldTimerManager().SetTimer(MPTimer, this, &AMain::RecoveryMP, MPDelay, true);
	}
}

float AMain::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	if (HP - DamageAmount <= 0.f)
	{
		HP = 0.f;
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
		HP -= DamageAmount;
		// HP �ڵ� ȸ��
		if(!GetWorldTimerManager().IsTimerActive(HPTimer))
			GetWorldTimerManager().SetTimer(HPTimer, this, &AMain::RecoveryHP, HPDelay, true);	
	}

	if (UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("boss")) // ���� ��������
	{
		MagicAttack = Cast<AMagicSkill>(DamageCauser);

		int TargetIndex = MagicAttack->index;

		if (TargetIndex == 11) // ���� ���Ϳ��� ���� ����
		{
			for (auto NPC : NPCManager->GetNPCMap())
			{
				if (NPC.Value->GetAgroTargets().Num() == 0) // Npc�� �ν� ������ �ƹ��� ���� ��
				{
					AEnemy* BossEnemy = Cast<AEnemy>(UGameplayStatics::GetActorOfClass(GetWorld(), NPC.Value->GetBoss()));
					NPC.Value->MoveToTarget(BossEnemy); // ���� ���Ϳ��� �̵�
					GetWorldTimerManager().ClearTimer(NPC.Value->GetMoveTimer());
					NPC.Value->GetAIController()->StopMovement();
					//UE_LOG(LogTemp, Log, TEXT("yesyesyes main %s"), *NPCList[i]->GetName());
				}
			}
		}
	}
	SetStat("HP", HP);
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
		HP += 50.f;
		SetStat("HP", HP);
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

	if (DialogueManager->GetDialogueUI()->CurrentState != 3 && !UIManager->IsMenuVisible())
	{   
	    if (DialogueManager->GetDialogueUI()->bDisableMouseAndKeyboard) return;
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
	if (SP < MaxSP && !recoverySP)
	{
		recoverySP = true;
		GetWorldTimerManager().SetTimer(SPTimer, this, &AMain::RecoverySP, SPDelay, true);
	}

	if (HP < MaxHP)
		GetWorldTimerManager().SetTimer(HPTimer, this, &AMain::RecoveryHP, HPDelay, true);

	if (MP < MaxMP)
		GetWorldTimerManager().SetTimer(MPTimer, this, &AMain::RecoveryMP, MPDelay, true);

}

float AMain::GetStat(FString StatName) const
{
	if (PlayerStats.Contains(StatName))
	{
		return PlayerStats[StatName];
	}
	return 0.f;
}

void AMain::SetStat(FString StatName, float Value)
{
	if (PlayerStats.Contains(StatName))
	{
		PlayerStats[StatName] = Value;
	}
}

void AMain::RecoveryHP()
{
	HP += 5.f;
	if (HP >= MaxHP)
	{
		HP = MaxHP;
		GetWorldTimerManager().ClearTimer(HPTimer);
	}
	SetStat("HP", HP);
}

void AMain::RecoveryMP()
{
	MP += 5.f;
	if (MP >= MaxMP)
	{
		MP = MaxMP;
		GetWorldTimerManager().ClearTimer(MPTimer);
	}
	SetStat("MP", MP);
}

void AMain::RecoverySP()
{
	SP += 1.f;
	if (SP >= MaxSP)
	{
		SP = MaxSP;
		GetWorldTimerManager().ClearTimer(SPTimer);
		recoverySP = false;
	}
	SetStat("SP", SP);
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
	if (Level == 5) return; // �ְ� ������ ���� ����

	Exp += Value;

	if (Exp >= MaxExp) // ���� �������� ����ġ�� ��� ä���ٸ�
	{
        Level += 1; // ���� ��
        if (LevelUpSound)
            UAudioComponent* AudioComponent = UGameplayStatics::SpawnSound2D(this, LevelUpSound);

		// ����ġ ��ġ update
		if (Exp == MaxExp)
		{
            Exp = 0.f;
		}
		else
		{
			float tmp = Exp - MaxExp;
			Exp = tmp;
		}

		if (Level == 5) Exp = MaxExp;

		// ������ ���� ���� �ɷ�ġ �ִ�ġ �ʱ�ȭ
		switch (Level)
		{
			case 2:
				MaxExp = 150.f;
				UIManager->SetSystemMessage(6);
				MaxHP = 350.f;
				MaxMP = 175.f;
				MaxSP = 325.f;
				break;
			case 3:
				MaxExp = 250.f;
				UIManager->SetSystemMessage(7);
                MaxHP = 450.f;
                MaxMP = 200.f;
                MaxSP = 350.f;
				break;
			case 4:
				MaxExp = 360.f;
				UIManager->SetSystemMessage(8);
                MaxHP = 600.f;
                MaxMP = 230.f;
                MaxSP = 375.f;
				break;
            case 5:
				UIManager->SetSystemMessage(9);
                MaxHP = 700.f;
                MaxMP = 280.f;
                MaxSP = 400.f;
                break;
		}

		HP += 100.f;
		MP += 50.f;
		SP += 100.f;

		if (HP > MaxHP) HP = MaxHP;
		if (MP > MaxMP) MP = MaxMP;
		if (SP > MaxSP) SP = MaxSP;


		SetStat("HP", HP);
		SetStat("MaxHP", MaxHP);
		SetStat("MP", MP);
		SetStat("MaxMP", MaxMP);
		SetStat("SP", SP);
		SetStat("MaxSP", MaxSP);
		SetStat("Level", Level);
		SetStat("Exp", Exp);
		SetStat("MaxExp", MaxExp);

        FTimerHandle Timer;
        GetWorld()->GetTimerManager().SetTimer(Timer, FTimerDelegate::CreateLambda([&]()
        {
				UIManager->RemoveSystemMessage();
        }), 3.f, false);
	}
}

const TMap<FString, TSubclassOf<class AItem>>* AMain::GetItemMap()
{
	if (Storage != nullptr) return &(Storage->ItemMap);
	return nullptr;
}

void AMain::PlayMontageWithItem()
{
	if (MainAnimInstance->Montage_IsPlaying(NormalMontage) == true) return; // �ߺ� ��� ����

	bCanMove = false; // �̵� ���� �Ұ�

	FRotator LookAtYaw;

	if (DialogueManager->GetDialogueNum() == 9 && ItemInHand)
	{
		if(ItemInHand->GetName().Contains("Yellow"))
			LookAtYaw = GetLookAtRotationYaw(CurrentOverlappedActor->GetActorLocation());
	}
	else if (DialogueManager->GetDialogueNum() == 21 && CurrentOverlappedActor->GetName().Contains("Divinum"))
	{
		LookAtYaw = GetLookAtRotationYaw(CurrentOverlappedActor->GetActorLocation());
	}
	else
		LookAtYaw = GetLookAtRotationYaw(ActiveOverlappingItem->GetActorLocation());

	SetActorRotation(LookAtYaw); // ������ �������� ȸ��

	// ��Ÿ�� ���
	MainAnimInstance->Montage_Play(NormalMontage);
}

void AMain::ShowManual()
{
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



void AMain::SkipCombat() // ���� ��ŵ, ���� ����
{
	if (DialogueManager->IsDialogueUIVisible() || !GameManager->IsSkippable() || GameManager->IsSkipping()
		|| MovementStatus == EMovementStatus::EMS_Dead) return;

	if (DialogueManager->GetDialogueNum() < 4) // first dungeon
	{
		if (DialogueManager->GetDialogueNum() <= 2) return;
		SkipFirstDungeon.Broadcast();
	}
	else if (DialogueManager->GetDialogueNum() < 15)
	{
		if (DialogueManager->GetDialogueNum() <= 10) return;
		SkipSecondDungeon.Broadcast();
	}
	else if (DialogueManager->GetDialogueNum() < 19)
	{
		if (DialogueManager->GetDialogueNum() <= 17) return;
		SkipFinalDungeon.Broadcast();
	}
	else return;
}

void AMain::RecoverWithLogo() // get school icon in second stage
{
	HP += 150.f;
	MP += 50.f;
	SP += 50.f;

	if (HP >= MaxHP) HP = MaxHP;
	if (MP >= MaxMP) MP = MaxMP;
	if (SP >= MaxSP) SP = MaxSP;

	SetStat("HP", HP);
	SetStat("MP", MP);
	SetStat("SP", SP);
}

void AMain::SetLevel5() // level cheat, ��� �ִ� ���� ����
{
	if (DialogueManager->GetDialogueNum() < 3 || Level == 5) return;

	if (LevelUpSound != nullptr)
		UAudioComponent* AudioComponent = UGameplayStatics::SpawnSound2D(this, LevelUpSound);

	Level = 5;
	Exp = MaxExp;

	MaxHP = 700.f;
	MaxMP = 280.f;
	MaxSP = 400.f;

	HP = MaxHP;
	MP = MaxMP;
	SP = MaxSP;

	SetStat("HP", HP);
	SetStat("MaxHP", MaxHP);
	SetStat("MP", MP);
	SetStat("MaxMP", MaxMP);
	SetStat("SP", SP);
	SetStat("MaxSP", MaxSP);
	SetStat("Level", Level);
	SetStat("Exp", Exp);
	SetStat("MaxExp", MaxExp);

	if (!UIManager->IsSystemMessageVisible())
	{
		UIManager->SetSystemMessage(15);
		FTimerHandle Timer;
		GetWorld()->GetTimerManager().SetTimer(Timer, FTimerDelegate::CreateLambda([&]()
		{
				UIManager->RemoveSystemMessage();
		}), 3.f, false);
	}
}

void AMain::UsePotion()
{
	if (PotionNum <= 0 || DialogueManager->GetDialogueNum() < 3) return;

	PotionNum--;

	HP = MaxHP;
	MP = MaxMP;
	SP = MaxSP;

	SetStat("HP", HP);
	SetStat("MP", MP);
	SetStat("SP", SP);
	SetStat("PotionNum", PotionNum);
}