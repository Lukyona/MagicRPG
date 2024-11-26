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
	//Create CameraBoom (pulls towards the player if there's a collision), 콜리전이 있으면 카메라를 플레이어쪽으로 당김 
	CameraBoom = CreateAbstractDefaultSubobject<USpringArmComponent>(TEXT("Camera"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 500.f; //Camera follows at this distance
	CameraBoom->bUsePawnControlRotation = true; // Rotate arm based on controller

	// but npc, enemy들도 여기에 콜리전으로 해당되어 게임 플레이가 불편하므로 콜리전 테스트 끔
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
	//상태 정보 초기화
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
	LevelData.Add(5, FLevelStats(0.f, 700.f, 280.f, 400.f)); // 마지막 레벨은 경험치가 필요 없으니 MaxExp는 0
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

	// 아이템 정보 관련
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
		CombatTargetLocation = CombatTarget->GetActorLocation(); // 전투 타겟의 위치 정보 받아오기
		UIManager->SetEnemyLocation(CombatTargetLocation); // 화살표와 체력 정보 띄우기 위함

        if (CombatTarget->GetEnemyMovementStatus() == EEnemyMovementStatus::EMS_Dead) // 현재 전투 타겟이 죽었다면
        {
			if(Targets.Contains(CombatTarget)) 
				Targets.Remove(CombatTarget); //타겟팅 가능 몹 배열에서 제거

			UnsetCombatTarget();
        }
	}
}

// Called to bind functionality to input
void AMain::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	// 액션은 키를 누르거나 놓는 즉시 한번만 실행
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMain::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("LMB", IE_Pressed, this, &AMain::LMBDown); // 왼쪽 마우스 버튼 다운

	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &AMain::Attack);

	PlayerInputComponent->BindAction("Targeting", IE_Pressed, this, &AMain::Targeting);

	PlayerInputComponent->BindAction("SkipCombat", IE_Pressed, GameManager, &UGameManager::SkipCombat);

	PlayerInputComponent->BindAction("ToggleMenu", IE_Pressed, UIManager, &UUIManager::ToggleMenu);

	PlayerInputComponent->BindAction("TriggerNextDialogue", IE_Pressed, DialogueManager, &UDialogueManager::TriggerNextDialogue);

    PlayerInputComponent->BindAction("DisplayControlGuide", IE_Pressed, UIManager, &UUIManager::DisplayControlGuide);

	PlayerInputComponent->BindAction("Escape", IE_Pressed, GameManager, &UGameManager::EscapeToSafeLocation);

	PlayerInputComponent->BindAction("UsePotion", IE_Pressed, this, &AMain::UsePotion);

	PlayerInputComponent->BindAction("StartFirstDungeon", IE_Pressed, GameManager, &UGameManager::StartFirstDungeon);


	// Axis는 매 프레임마다 호출
	//“키 이름”, bind할 함수가 있는 클래스의 인스턴스, bind할 함수의 주소
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
		const FRotator Rotation = Controller->GetControlRotation(); // 회전자 반환 함수
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(Axis);
		AddMovementInput(Direction, Value);

		if (bRunning && PlayerStats[EPlayerStat::SP] > 0.f) // 달리고 있는 상태 + 스태미나가 0이상일 때 스태미나 감소
			AddSP(-1);
	}
}

// 매 프레임마다 키가 눌렸는지 안 눌렸는지 확인될 것
// 키가 안 눌렸으면 Value는 0
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
	if (!Value || CurrentSP <= 0.f) // 쉬프트키 안 눌려 있거나 스태미나가 0 이하일 때
	{
		bRunning = false;
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed; //속도 하향

		if (CurrentSP < PlayerStats[EPlayerStat::MaxSP] && !recoverySP) // 스태미나 Full 상태가 아니면
		{ // 스태미나 자동 회복
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
		GetCharacterMovement()->MaxWalkSpeed = RunSpeed; // 속도 상향		
	}
}

void AMain::Jump()
{
	// 죽거나 대화 중일 때는 점프 불가, 수동으로 움직임 막았을 때도 불가
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

	// 특정 시점에는 줌 불가능
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

	// 다음 대사 출력 관련
	if (DialogueManager->IsDialogueUIVisible() && DialogueManager->GetDialogueUI()->GetCurrentState() != 3
		&& !UIManager->IsMenuVisible())
	{
		if (DialogueManager->GetDialogueUI()->IsInputDisabled()) return;
		else DialogueManager->GetDialogueUI()->Interact();
	}


	if (!MainAnimInstance || !NormalMontage) return;
	if (MainAnimInstance->Montage_IsPlaying(NormalMontage) == true) return; // 중복 재생 방지

	// 아이템 상호작용 몽타주 관련
	if (ActiveOverlappingItem && !EquippedWeapon) // 오버랩된 아이템 존재, 장착한 무기가 없을 때
	{
		PlayMontageWithItem(); // 아이템 관련 몽타주 재생
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
			if (CurrentOverlappedActor->GetName().Contains("MovingStone")  // 오버랩된 액터가 움직이는 돌이고
				&& ItemInHand->GetName().Contains("Yellow")) // 손에 있는 아이템이 yellow stone일 때
			{
				PlayMontageWithItem();
				MainAnimInstance->Montage_JumpToSection(FName("PutStone"), NormalMontage); // put yellow stone
			}
		}
	}

	// 마지막 과제 아이템
	if (CurrentOverlappedActor && CurrentOverlappedActor->GetName().Contains("DivinumPraesidium"))
	{
		if (DialogueManager->GetDialogueNum() == 21)
		{
			if (DialogueManager->GetDialogueUI()->GetSelectedReply() != 1 || DialogueManager->IsDialogueUIVisible()) return;

			PlayMontageWithItem();
			MainAnimInstance->Montage_JumpToSection(FName("PickStone"), NormalMontage); // 돌 챙기기
		}
	}
}

void AMain::PlayMontageWithItem()
{
	bCanMove = false; // 이동 조작 불가

	if (MainAnimInstance->Montage_IsPlaying(NormalMontage) == true) return; // 중복 재생 방지

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

	SetActorRotation(LookAtYaw); // 아이템 방향으로 회전

	// 몽타주 재생
	MainAnimInstance->Montage_Play(NormalMontage);
}

void AMain::Attack()
{
	// 특정 상황엔 공격 불가
	if (DialogueManager->GetDialogueNum() < 3 || DialogueManager->GetDialogueNum() > 20) return;

	if (EquippedWeapon && !bAttacking && MovementStatus != EMovementStatus::EMS_Dead 
		&& !DialogueManager->IsDialogueUIVisible())
	{
		SetSkillNum(MainPlayerController->WhichKeyDown()); // 눌린 키로 어떤 스킬 사용인지 구분
		
		if (PlayerStats[EPlayerStat::Level] < GetSkillNum()) return;

		ToSpawn = AttackSkillData->FindRow<FAttackSkillData>("Attack", "")->Skills[GetSkillNum()-1];

		bAttacking = true;
		SetInterpToEnemy(true);

		if (MainAnimInstance && CombatMontage)
		{
            bCanMove = false; // 이동 조작 불가
			MainAnimInstance->Montage_Play(CombatMontage);
			MainAnimInstance->Montage_JumpToSection(FName("Attack"), CombatMontage);
			Spawn(); // 마법 스폰
		}
	}
}

void AMain::AttackEnd()
{
    bCanMove = true; // 이동 가능
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

			Targets.Add(Enemy); // 타겟팅 가능 몹 배열에 추가
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
			if (Targets.Contains(Enemy)) Targets.Remove(Enemy); //타겟팅 가능 몹 배열에서 제거

			if (Targets.Num() == 0) // 타겟팅 가능 몬스터가 없으면
			{
				bOverlappingCombatSphere = false;
			}

			if (CombatTarget == Enemy) // 현재 (타겟팅된)전투 타겟과 (전투 범위를 나간 몬스터가)동일하다면
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
		 //타겟인덱스가 총 타겟 가능 몹 수 이상이면 다시 0으로 초기화
		if(Targets.Num() != 0) TargetIndex %= Targets.Num();

		//There is already exist targeted enemy, then targetArrow remove
		if (UIManager->IsTargetArrowVisible())
		{
			UIManager->RemoveTargetArrow();
			UIManager->RemoveEnemyHPBar();
		}

		if (Targets.Num() == 0) return; // 타겟팅 가능 몬스터가 없으면 리턴

		if (!bAutoTargeting) // 수동 타겟팅
		{
			// 죽은 몬스터는 타겟팅 불가
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
	// 전투 타겟 해제
	CombatTarget = nullptr;
	bHasCombatTarget = false;

	if (UIManager->IsTargetArrowVisible())
	{// 화살표 및 몬스터 체력바 제거
		UIManager->RemoveTargetArrow();
		UIManager->RemoveEnemyHPBar();
	}
}

void AMain::Spawn() 
{
	float CurrentMP = PlayerStats[EPlayerStat::MP];
	if (ToSpawn && CurrentMP >= MIN_MANA_TO_CAST) // 마법 사용에 필요한 최소 MP가 15
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

				// 1, 3번 제외 나머지 공격은 적 위치에서 스폰
				if (GetSkillNum() != 1 && GetSkillNum() != 3 && CombatTarget)
				{
					spawnLocation = CombatTarget->GetActorLocation();
				}

				if (MagicAttack.IsValid())
				{
					MagicAttack->Destroy();  // 기존의 MagicAttack 제거
					MagicAttack.Reset();     // 스마트 포인터 초기화
				}

				MagicAttack = MakeWeakObjectPtr<AMagicSkill>(world->SpawnActor<AMagicSkill>(ToSpawn, spawnLocation, rotator, spawnParams));
				if (MagicAttack.IsValid())
				{
					MagicAttack->SetInstigator(MainPlayerController);

					// 적에게로 이동하는 공격은 공격이 어디로 날라갈지를 정해줘야함.
					if ((GetSkillNum() == 1 || GetSkillNum() == 3) &&CombatTarget)
						MagicAttack->SetTarget(CombatTarget);
				}
				
			}
		}), 0.6f, false); // 0.6초 뒤 실행, 반복X

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
		// HP 자동 회복
		if(!GetWorldTimerManager().IsTimerActive(HPTimer))
			GetWorldTimerManager().SetTimer(HPTimer, this, &AMain::RecoveryHP, HPDelay, true);	
	}

	SetStat(EPlayerStat::HP, CurrentHP);

	if (UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("boss")) // 보스 스테이지
	{
		MagicAttack = Cast<AMagicSkill>(DamageCauser);
		if (MagicAttack == nullptr) return DamageAmount;
		
		if (MagicAttack->GetCaster() == ECasterType::Boss) // 보스 몬스터에게 공격 당함
		{
			for (auto NPC : NPCManager->GetNPCMap())
			{
				if (NPC.Value->GetAgroTargets().Num() == 0) // Npc의 인식 범위에 아무도 없을 때
				{
					AEnemy* BossEnemy = Cast<AEnemy>(UGameplayStatics::GetActorOfClass(GetWorld(), ABoss::StaticClass()));
					NPC.Value->MoveToTarget(BossEnemy); // 보스 몬스터에게 이동
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

	// 자동 회복 종료
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

	// 자동 회복 재개
	GetWorldTimerManager().SetTimer(HPTimer, this, &AMain::RecoveryHP, HPDelay, true);
	GetWorldTimerManager().SetTimer(MPTimer, this, &AMain::RecoveryMP, MPDelay, true);
	GetWorldTimerManager().SetTimer(SPTimer, this, &AMain::RecoverySP, SPDelay, true);
}

void AMain::ItemSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		if (auto actor = Cast<AItem>(OtherActor)) return; // 오버랩된 게 아이템이면 실행X
		if (auto actor = Cast<AYaroCharacter>(OtherActor)) return; // 오버랩된 게 npc면 실행X

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
	if (PlayerStats[EPlayerStat::Level] == 5) return; // 최고 레벨일 때는 리턴

	float CurrentExp = PlayerStats[EPlayerStat::Exp];
	float MaxExp = PlayerStats[EPlayerStat::MaxExp];

	CurrentExp += Value;
	SetStat(EPlayerStat::Exp, CurrentExp);

	if (CurrentExp >= MaxExp) // 다음 레벨로의 경험치를 모두 채웠다면
	{
		LevelUp();
	}
}

void AMain::LevelUp()
{
	float CurrentLevel = PlayerStats[EPlayerStat::Level];

	float CurrentExp = PlayerStats[EPlayerStat::Exp];
	float MaxExp = PlayerStats[EPlayerStat::MaxExp];

	CurrentLevel += 1; // 레벨 업
	if (LevelUpSound)
		UAudioComponent* AudioComponent = UGameplayStatics::SpawnSound2D(this, LevelUpSound);

	// 경험치 수치 update
	if (CurrentExp == MaxExp) CurrentExp = 0.f;
	else CurrentExp -= MaxExp;

	SetStat(EPlayerStat::Exp, CurrentExp);

	if (CurrentLevel == 5) CurrentExp = MaxExp;

	// 시스템 메세지 표시
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