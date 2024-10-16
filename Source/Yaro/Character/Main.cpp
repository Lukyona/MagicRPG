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
	MaxHP = 300.f;
	HP = 300.f;
	MaxMP = 150.f;
	MP = 150.f;
	MaxSP = 300.f;
	SP = 300.f;

	Level = 1;
	Exp = 0.f;
	MaxExp = 60.f;

	HPDelay = 3.f;
	MPDelay = 2.f;
	SPDelay = 0.5f;

	DeathDelay = 3.f;

	MovementStatus = EMovementStatus::EMS_Normal;

	PotionNum = 0;

	bESCDown = false;

}

// Called when the game starts or when spawned
void AMain::BeginPlay()
{
	Super::BeginPlay();

	CombatSphere->OnComponentBeginOverlap.AddDynamic(this, &AMain::CombatSphereOnOverlapBegin);
	CombatSphere->OnComponentEndOverlap.AddDynamic(this, &AMain::CombatSphereOnOverlapEnd);

	ItemSphere->OnComponentBeginOverlap.AddDynamic(this, &AMain::ItemSphereOnOverlapBegin);
	ItemSphere->OnComponentEndOverlap.AddDynamic(this, &AMain::ItemSphereOnOverlapEnd);

	// ���� ĳ���� ���� ���� ����
	if (this->GetName().Contains("Boy")) Gender = 1;
	if (this->GetName().Contains("Girl")) Gender = 2;

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
		if (MainPlayerController)
		{
			MainPlayerController->EnemyLocation = CombatTargetLocation; // ȭ��ǥ�� ü�� ���� ���� ����
		}

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

            if (MainPlayerController->bTargetArrowVisible) 
            {// ȭ��ǥ �� ���� ü�¹� ����
                MainPlayerController->RemoveTargetArrow();
                MainPlayerController->RemoveEnemyHPBar();
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
			SP -= 1.f;
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
			SP -= 1.f;
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
		if (MainPlayerController->DialogueNum == 0) return;

		if (UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("cave")
			&& MainPlayerController->DialogueNum == 19)
			return;

		if (MainPlayerController->bDialogueUIVisible 
			&& MainPlayerController->DialogueNum == 11) 
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
		if (ActiveOverlappingItem && MainPlayerController->DialogueNum == 9)
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
				if (MainPlayerController->DialogueNum == 9)
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
			if (MainPlayerController->DialogueNum == 21 && MainAnimInstance && NormalMontage)
			{
				if (MainPlayerController->DialogueUI->SelectedReply != 1 || MainPlayerController->bDialogueUIVisible) return;

				PlayMontageWithItem();
				MainAnimInstance->Montage_JumpToSection(FName("PickStone"), NormalMontage); // �� ì���
			}
		}
	}

	// Targeting Off
	if (CombatTarget)
	{
		if (MainPlayerController->bTargetArrowVisible)
		{ // ȭ��ǥ �� ü�¹� ����
			MainPlayerController->RemoveTargetArrow();
			MainPlayerController->RemoveEnemyHPBar();
		}
		// ���� Ÿ�� ����
		CombatTarget = nullptr;
		bHasCombatTarget = false;
	}

	// ���� ��� ��� ����
	if (MainPlayerController->bDialogueUIVisible 
		&& MainPlayerController->DialogueUI->CurrentState != 3 
		&& !MainPlayerController->bMenuVisible)
	{
        if (MainPlayerController->DialogueUI->bDisableMouseAndKeyboard) return;
		else MainPlayerController->DialogueUI->Interact();
	}
}

void AMain::LMBUp()
{
	bLMBDown = false;
}

void AMain::Attack()
{
	// Ư�� ��Ȳ�� ���� �Ұ�
	if (MainPlayerController->DialogueNum < 3 || MainPlayerController->DialogueNum > 20 ) return;

	if (EquippedWeapon && !bAttacking && MovementStatus != EMovementStatus::EMS_Dead 
		&& !MainPlayerController->bDialogueUIVisible)
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
				MainPlayerController->RemoveTargetArrow();
				MainPlayerController->RemoveEnemyHPBar();
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
		if (MainPlayerController->bTargetArrowVisible)
		{
			MainPlayerController->RemoveTargetArrow();
			MainPlayerController->RemoveEnemyHPBar();
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

		MainPlayerController->DisplayTargetArrow(); 
		MainPlayerController->DisplayEnemyHPBar();
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
			for (int i = 0; i < NPCList.Num(); i++)
			{
				if (NPCList[i]->GetAgroTargets().Num() == 0) // Npc�� �ν� ������ �ƹ��� ���� ��
				{
					AEnemy* BossEnemy = Cast<AEnemy>(UGameplayStatics::GetActorOfClass(GetWorld(), NPCList[i]->GetBoss()));
					NPCList[i]->MoveToTarget(BossEnemy); // ���� ���Ϳ��� �̵�
					GetWorldTimerManager().ClearTimer(NPCList[i]->GetMoveTimer());
					NPCList[i]->GetAIController()->StopMovement();
					//UE_LOG(LogTemp, Log, TEXT("yesyesyes main %s"), *NPCList[i]->GetName());
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

void AMain::Jump()
{
	// �װų� ��ȭ ���� ���� ���� �Ұ�, �������� ������ ������ ���� �Ұ�
	if (MovementStatus != EMovementStatus::EMS_Dead
		&& !MainPlayerController->bDialogueUIVisible
		&& bCanMove)
	{
		Super::Jump();
	}
}

void AMain::Revive() // if player is dead, spawn player at the initial location
{
	if(MainPlayerController->DialogueNum <= 4) // first dungeon
		this->SetActorLocation(FVector(-192.f, 5257.f, 3350.f));
	else if(MainPlayerController->DialogueNum <= 15) // second dungeon
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

	if (MainPlayerController->DialogueUI->CurrentState != 3 && !MainPlayerController->bMenuVisible)
	{   
        if (MainPlayerController->DialogueUI->bDisableMouseAndKeyboard) return;
        else MainPlayerController->DialogueUI->Interact();
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


void AMain::SaveGame()
{
	if (MainPlayerController->DialogueNum >= 23) return;

	if (MainPlayerController->bDialogueUIVisible || MainPlayerController->bFallenPlayer || MainAnimInstance->bIsInAir
		|| MainPlayerController->DialogueNum == 21)
	{
		GetWorld()->GetTimerManager().SetTimer(SaveTimer, this, &AMain::SaveGame, 1.f, false);

        return;
	} 

    //UE_LOG(LogTemp, Log, TEXT("SaveGame"));

	UYaroSaveGame* SaveGameInstance = Cast<UYaroSaveGame>(UGameplayStatics::CreateSaveGameObject(UYaroSaveGame::StaticClass()));

	SaveGameInstance->PlayerGender = Gender;
	SaveGameInstance->CharacterStats.HP = HP;
	SaveGameInstance->CharacterStats.MaxHP = MaxHP;
	SaveGameInstance->CharacterStats.MP = MP;
	SaveGameInstance->CharacterStats.MaxMP = MaxMP;
	SaveGameInstance->CharacterStats.SP = SP;
	SaveGameInstance->CharacterStats.MaxSP = MaxSP;
	SaveGameInstance->CharacterStats.Level = Level;
	SaveGameInstance->CharacterStats.Exp = Exp;
	SaveGameInstance->CharacterStats.MaxExp = MaxExp;
	SaveGameInstance->CharacterStats.PotionNum = PotionNum;

    SaveGameInstance->DialogueNum = MainPlayerController->DialogueNum;
    SaveGameInstance->CharacterStats.FallingCount = MainPlayerController->FallingCount;

	SaveGameInstance->CharacterStats.Location = GetActorLocation();
	SaveGameInstance->CharacterStats.Rotation = GetActorRotation();

	if (MainPlayerController->DialogueNum < 23)
	{
		SaveGameInstance->NpcInfo.MomoLocation = Momo->GetActorLocation();
		SaveGameInstance->NpcInfo.LukoLocation = Luko->GetActorLocation();
		SaveGameInstance->NpcInfo.VovoLocation = Vovo->GetActorLocation();
		SaveGameInstance->NpcInfo.ViviLocation = Vivi->GetActorLocation();
		SaveGameInstance->NpcInfo.ZiziLocation = Zizi->GetActorLocation();
	}
    
	if (MainPlayerController->DialogueNum < 4)
		SaveGameInstance->NpcInfo.TeamMoveIndex = Vivi->GetIndex();

	SaveGameInstance->DeadEnemyList = Enemies;

	if (ItemInHand)
	{
		SaveGameInstance->CharacterStats.ItemName = ItemInHand->GetName();
	}

	UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveGameInstance->SaveName, SaveGameInstance->UserIndex);

	if (MainPlayerController->DialogueNum == 18 && MainPlayerController->SystemMessageNum == 12) return;

	GetWorld()->GetTimerManager().SetTimer(SaveTimer, this, &AMain::SaveGame, 1.f, false);
}

void AMain::LoadGame()
{
	UYaroSaveGame* LoadGameInstance = Cast<UYaroSaveGame>(UGameplayStatics::CreateSaveGameObject(UYaroSaveGame::StaticClass()));

	LoadGameInstance = Cast<UYaroSaveGame>(UGameplayStatics::LoadGameFromSlot(LoadGameInstance->SaveName, LoadGameInstance->UserIndex));

	MainPlayerController = Cast<AMainPlayerController>(GetController());
    MainPlayerController->DialogueNum = LoadGameInstance->DialogueNum;
    MainPlayerController->FallingCount = LoadGameInstance->CharacterStats.FallingCount;

	HP = LoadGameInstance->CharacterStats.HP;
	MaxHP = LoadGameInstance->CharacterStats.MaxHP;
	MP = LoadGameInstance->CharacterStats.MP;
	MaxMP = LoadGameInstance->CharacterStats.MaxMP;
	SP = LoadGameInstance->CharacterStats.SP;
	MaxSP = LoadGameInstance->CharacterStats.MaxSP;
	Level = LoadGameInstance->CharacterStats.Level;
	Exp = LoadGameInstance->CharacterStats.Exp;
	MaxExp = LoadGameInstance->CharacterStats.MaxExp;
	PotionNum = LoadGameInstance->CharacterStats.PotionNum;


	Enemies = LoadGameInstance->DeadEnemyList;
	
	if (MainPlayerController->DialogueNum < 4)
	{
		Vovo->SetIndex(LoadGameInstance->NpcInfo.TeamMoveIndex);
        Vivi->SetIndex(LoadGameInstance->NpcInfo.TeamMoveIndex);
        Zizi->SetIndex(LoadGameInstance->NpcInfo.TeamMoveIndex);
	}

    if (SP < MaxSP && !recoverySP)
    {
        recoverySP = true;
        GetWorldTimerManager().SetTimer(SPTimer, this, &AMain::RecoverySP, SPDelay, true);
    }

    if (HP < MaxHP)
        GetWorldTimerManager().SetTimer(HPTimer, this, &AMain::RecoveryHP, HPDelay, true);

    if (MP < MaxMP)
		GetWorldTimerManager().SetTimer(MPTimer, this, &AMain::RecoveryMP, MPDelay, true);


    if (ObjectStorage)
    {
        if (Storage)
        {
            FString ItemName = LoadGameInstance->CharacterStats.ItemName;

            if (ItemName.Contains("Yellow") && Storage->ItemMap.Contains("YellowStone")) // ������ �� �տ� ���� ���� ���¿�����
            {
                AItem* Item = GetWorld()->SpawnActor<AItem>(Storage->ItemMap["YellowStone"]);
                Item->PickUp(this);
            }

        }
    }

	if (MainPlayerController->DialogueNum != 5)
	{
		if (MainPlayerController->DialogueNum == 15 && UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("boss")) return;

		SetActorLocation(LoadGameInstance->CharacterStats.Location);
		SetActorRotation(LoadGameInstance->CharacterStats.Rotation);

		if (MainPlayerController->DialogueNum == 19) return;

		Momo->SetActorLocation(LoadGameInstance->NpcInfo.MomoLocation);
		Luko->SetActorLocation(LoadGameInstance->NpcInfo.LukoLocation);
		Vovo->SetActorLocation(LoadGameInstance->NpcInfo.VovoLocation);
		Vivi->SetActorLocation(LoadGameInstance->NpcInfo.ViviLocation);
		Zizi->SetActorLocation(LoadGameInstance->NpcInfo.ZiziLocation);
	}
}

void AMain::CheckDialogueRequirement() // ����� ���൵�� ���� npc�� �̵� �� ��ġ ����
{
	int count = 0;

	switch (MainPlayerController->DialogueNum)
	{
		case 3: // after golem died
			for (int i = 0; i < Enemies.Num(); i++)
			{
				if (Enemies[i].Contains("Golem") && Enemies.Num() == 9)
				{
					Momo->GetAIController()->MoveToLocation(FVector(594.f, -1543.f, 2531.f));
					Luko->GetAIController()->MoveToLocation(FVector(494.f, -1629.f, 2561.f));
					Vovo->GetAIController()->MoveToLocation(FVector(903.f, -1767.f, 2574.f));
					Vivi->GetAIController()->MoveToLocation(FVector(790.f, -1636.f, 2566.f));
					Zizi->GetAIController()->MoveToLocation(FVector(978.f, -1650.f, 2553.f));

					MainPlayerController->DisplayDialogueUI();
					return;
				}
			}
			Luko->MoveToPlayer();
			Momo->MoveToPlayer();
			Vovo->MoveToLocation();
			Vivi->MoveToLocation();
			Zizi->MoveToLocation();
			//NpcGo = true;
			break;
		case 4: // npc move to boat and wait player
            Momo->SetActorRotation(FRotator(0.f, 85.f, 0.f));
            Luko->SetActorRotation(FRotator(0.f, 103.f, 0.f));
            Vivi->SetActorRotation(FRotator(0.f, 97.f, 0.f));
            Zizi->SetActorRotation(FRotator(0.f, 94.f, 0.f));
            Vovo->SetActorRotation(FRotator(0.f, 91.f, 0.f));

            Momo->GetAIController()->MoveToLocation(FVector(660.f, 1035.f, 1840.f));
            Luko->GetAIController()->MoveToLocation(FVector(598.f, 1030.f, 1840.f));
            Vivi->GetAIController()->MoveToLocation(FVector(710.f, 995.f, 1840.f));
            Zizi->GetAIController()->MoveToLocation(FVector(690.f, 930.f, 1840.f));
            Vovo->GetAIController()->MoveToLocation(FVector(630.f, 970.f, 1840.f));
			break;
		 case 9: //if ItemInHand is null, the stone have to put on the floor (this is check in blueprint)
			MainPlayerController->SystemMessageNum = 10;
			MainPlayerController->SetSystemMessage();
            MainPlayerController->DialogueUI->AllNpcLookAtPlayer();
            for (int i = 0; i < NPCList.Num(); i++)
            {
                NPCList[i]->GetAIController()->StopMovement();
                GetWorld()->GetTimerManager().ClearTimer(NPCList[i]->GetMoveTimer());
            }
			Momo->GetAIController()->MoveToLocation(FVector(5307.f, -3808.f, -2122.f));
			Luko->GetAIController()->MoveToLocation(FVector(5239.f, -3865.f, -2117.f));
			Vovo->GetAIController()->MoveToLocation(FVector(5433.f, -3855.f, -2117.f));
			Vivi->GetAIController()->MoveToLocation(FVector(5392.f, -3686.f, -2117.f));
			Zizi->GetAIController()->MoveToLocation(FVector(5538.f, -3696.f, -2115.f));
			break;
		 case 12:
			 for (auto enemy : Enemies)
			 {
				 if(enemy.Contains("spider")) // Event enemies in second dungeon
					 count++;

				 if (count == 5)
				 {
					 MainPlayerController->bCalculateOn = true;
					 MainPlayerController->DisplayDialogueUI();
				 }
			 }
			 break;
		 case 14:
			 for (auto enemy : Enemies)
			 {
				 if (enemy.Contains("monster")) // Final enemies in second dungeon
					 count++;

				 if (count == 3) MainPlayerController->DisplayDialogueUI();
			 }
			 break;
		 case 16:
			 Vivi->GetAIController()->MoveToLocation(FVector(105.f, 3176.f, 182.f));
			 Momo->GetAIController()->MoveToLocation(FVector(-86.f, 3263.f, 177.f));
			 Luko->GetAIController()->MoveToLocation(FVector(184.f, 3317.f, 182.f));
			 Vovo->GetAIController()->MoveToLocation(FVector(-140.f, 3370.f, 182.f));
			 Zizi->GetAIController()->MoveToLocation(FVector(68.f, 3398.f, 184.f));
			 MainPlayerController->DialogueUI->SelectedReply = 2;
			 break;
		 case 17:
			 Vivi->GetAIController()->MoveToLocation(FVector(100.f, 1997.f, 182.f));
			 Momo->GetAIController()->MoveToLocation(FVector(-86.f, 2150.f, 177.f));
			 Luko->GetAIController()->MoveToLocation(FVector(171.f, 2130.f, 182.f));
			 Vovo->GetAIController()->MoveToLocation(FVector(-160.f, 2060.f, 182.f));
			 Zizi->GetAIController()->MoveToLocation(FVector(18.f, 2105.f, 184.f));
			 break;
		 case 18:
			 if(Enemies.Num() == 5)
				 MainPlayerController->DisplayDialogueUI();
			 break;
		 case 19:
			 if (Enemies.Num() == 5)//������, ��Ż�� �̵�
			 {
				 Momo->GetAIController()->MoveToLocation(FVector(8.f, -3585.f, 684.f));
				 Luko->GetAIController()->MoveToLocation(FVector(8.f, -3585.f, 684.f));
				 Vovo->GetAIController()->MoveToLocation(FVector(8.f, -3585.f, 684.f));
				 Vivi->GetAIController()->MoveToLocation(FVector(8.f, -3585.f, 684.f));
				 Zizi->GetAIController()->MoveToLocation(FVector(8.f, -3585.f, 684.f));
				 MainPlayerController->SystemMessageNum = 13;
				 MainPlayerController->SetSystemMessage();
			 }
			 else if (Enemies.Num() == 0) //����
			 {
				 MainPlayerController->DisplayDialogueUI();
			 }
			 break;
		case 20: // �� ������ �̵�
			Momo->GetAIController()->MoveToLocation(FVector(-4660.f, 118.f, 393.f));
			Luko->GetAIController()->MoveToLocation(FVector(-4545.f, -241.f, 401.f));
			Vovo->GetAIController()->MoveToLocation(FVector(-4429.f, 103.f, 396.f));
			Vivi->GetAIController()->MoveToLocation(FVector(-4355.f, -195.f, 405.f));
			Zizi->GetAIController()->MoveToLocation(FVector(-4695.f, -190.f, 394.f));
			Momo->SetActorRotation(FRotator(0.f, 296.f, 0.f));
			Luko->SetActorRotation(FRotator(0.f, 97.f, 0.f));
			Vovo->SetActorRotation(FRotator(0.f, 219.f, 0.f));
			Vivi->SetActorRotation(FRotator(0.f, 145.f, 0.f));
			Zizi->SetActorRotation(FRotator(0.f, 49.f, 0.f));
			break;
		case 22:
			Momo->GetCharacterMovement()->MaxWalkSpeed = 600.f;
			Vivi->GetCharacterMovement()->MaxWalkSpeed = 500.f;
			Zizi->GetCharacterMovement()->MaxWalkSpeed = 500.f;
			Vovo->GetCharacterMovement()->MaxWalkSpeed = 450.f;
			Luko->GetCharacterMovement()->MaxWalkSpeed = 450.f;
			Momo->GetAIController()->MoveToLocation(FVector(508.f, 120.f, 100.f));
			Luko->GetAIController()->MoveToLocation(FVector(311.f, -78.f, 103.f));
			Vovo->GetAIController()->MoveToLocation(FVector(469.f, -22.f, 103.f));
			Vivi->GetAIController()->MoveToLocation(FVector(267.f, 65.f, 101.f));
			Zizi->GetAIController()->MoveToLocation(FVector(591.f, 28.f, 104.f));
			break;
		case 23:
			Zizi->GetAIController()->MoveToLocation(FVector(2560.f, 330.f, 157.f));
			Vovo->GetAIController()->MoveToLocation(FVector(2570.f, 335.f, 154.f));
			Luko->GetAIController()->MoveToLocation(FVector(1517.f, 335.f, 155.f));
			Momo->GetAIController()->MoveToLocation(FVector(1530.f, 330.f, 150.f));
			Vivi->GetAIController()->MoveToLocation(FVector(625.f, 330.f, 153.f));
			break;
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
}

void AMain::RecoveryMP()
{
	MP += 5.f;
	if (MP >= MaxMP)
	{
		MP = MaxMP;
		GetWorldTimerManager().ClearTimer(MPTimer);
	}
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
}

void AMain::ESCDown()
{
	bESCDown = true;

	if (MainPlayerController)
	{
		MainPlayerController->ToggleMenu();
	}
}

void AMain::ESCUp()
{
	bESCDown = false;
}

void AMain::GetExp(float exp)
{
	if (Level == 5) return; // �ְ� ������ ���� ����

	Exp += exp;

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
				MainPlayerController->SystemMessageNum = 6;
				MainPlayerController->SetSystemMessage();
				MaxHP = 350.f;
				MaxMP = 175.f;
				MaxSP = 325.f;
				break;
			case 3:
				MaxExp = 250.f;
                MainPlayerController->SystemMessageNum = 7;
                MainPlayerController->SetSystemMessage();
                MaxHP = 450.f;
                MaxMP = 200.f;
                MaxSP = 350.f;
				break;
			case 4:
				MaxExp = 360.f;
                MainPlayerController->SystemMessageNum = 8;
                MainPlayerController->SetSystemMessage();
                MaxHP = 600.f;
                MaxMP = 230.f;
                MaxSP = 375.f;
				break;
            case 5:
                MainPlayerController->SystemMessageNum = 9;
                MainPlayerController->SetSystemMessage();
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

        FTimerHandle Timer;
        GetWorld()->GetTimerManager().SetTimer(Timer, FTimerDelegate::CreateLambda([&]()
        {
            MainPlayerController->RemoveSystemMessage();
        }), 3.f, false);
	}
}

void AMain::PlayMontageWithItem()
{
	if (MainAnimInstance->Montage_IsPlaying(NormalMontage) == true) return; // �ߺ� ��� ����

	bCanMove = false; // �̵� ���� �Ұ�

	FRotator LookAtYaw;

	if (MainPlayerController->DialogueNum == 9 && ItemInHand)
	{
		if(ItemInHand->GetName().Contains("Yellow"))
			LookAtYaw = GetLookAtRotationYaw(CurrentOverlappedActor->GetActorLocation());
	}
	else if (MainPlayerController->DialogueNum == 21 && CurrentOverlappedActor->GetName().Contains("Divinum"))
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
	if (MainPlayerController->bManualVisible) MainPlayerController->RemoveManual();
	else MainPlayerController->DisplayManual();
}

void AMain::Escape() // ��� Ż��, ���� �̵�
{
	if (MainPlayerController->DialogueNum >= 6
		&& !MainPlayerController->bDialogueUIVisible
		&& MovementStatus != EMovementStatus::EMS_Dead)
	{
		if (MainPlayerController->DialogueNum <= 8)
		{
            SetActorLocation(FVector(4620.f, -3975.f, -2117.f));
		}
		else if (MainPlayerController->DialogueNum <= 11)
		{
            SetActorLocation(FVector(5165.f, -2307.f, -2117.f));
		}
		else if(MainPlayerController->DialogueNum <= 15)
		{
			SetActorLocation(FVector(2726.f, -3353.f, -500.f));
		}
	}
}

bool AMain::CanTalkWithNpc()
{
	for (int i = 0; i < NPCList.Num(); i++)
	{
		float distance = GetDistanceTo(NPCList[i]);
		//UE_LOG(LogTemp, Log, TEXT("%s %f"), *NPCList[i]->GetName(), distance);

		if (distance >= 1300.f) // npc�� �ʹ� �ָ� ��ȭ �Ұ�
			return false;
	}
	return true;
}

void AMain::AllNpcMoveToPlayer()
{
	for (int i = 0; i < NPCList.Num(); i++)
	{
		NPCList[i]->MoveToPlayer();
	}
}

void AMain::SkipCombat() // ���� ��ŵ, ���� ����
{
	if (MainPlayerController->bDialogueUIVisible || !bCanSkip || bSkip || MovementStatus == EMovementStatus::EMS_Dead) return;

	if (MainPlayerController->DialogueNum < 4) // first dungeon
	{
		if (MainPlayerController->DialogueNum <= 2) return;
		SkipFirstDungeon.Broadcast();
	}
	else if (MainPlayerController->DialogueNum < 15)
	{
		if (MainPlayerController->DialogueNum <= 10) return;
		SkipSecondDungeon.Broadcast();
	}
	else if (MainPlayerController->DialogueNum < 19)
	{
		if (MainPlayerController->DialogueNum <= 17) return;
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
}

void AMain::SetLevel5() // level cheat, ��� �ִ� ���� ����
{
	if (MainPlayerController->DialogueNum < 3 || Level == 5) return;

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

	if (!MainPlayerController->bSystemMessageVisible)
	{
		MainPlayerController->SystemMessageNum = 15;
		MainPlayerController->SetSystemMessage();
		FTimerHandle Timer;
		GetWorld()->GetTimerManager().SetTimer(Timer, FTimerDelegate::CreateLambda([&]()
		{
			MainPlayerController->RemoveSystemMessage();
		}), 3.f, false);
	}
}

void AMain::UsePotion()
{
	if (PotionNum <= 0 || MainPlayerController->DialogueNum < 3) return;

	PotionNum--;

	HP = MaxHP;
	MP = MaxMP;
	SP = MaxSP;
}