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
{//고침 CreateAbstractDefaultSubobject
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


	// Configure character movement
	WalkSpeed = 350.f;
	RunSpeed = 600.f;
	GetCharacterMovement()->JumpZVelocity = 400.f;
	GetCharacterMovement()->SetWalkableFloorAngle(50.f);
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	// 공격 범위 설정
	CombatSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatSphere"));
	CombatSphere->SetupAttachment(GetRootComponent());
	CombatSphere->InitSphereRadius(600.f);
	CombatSphere->SetRelativeLocation(FVector(100.f, 0.f, 0.f));
	CombatSphere->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);

	bHasCombatTarget = false;
	targetIndex = 0;

	// 아이템 상호작용 범위 설정
	ItemSphere = CreateDefaultSubobject<USphereComponent>(TEXT("ItemSphere"));
	ItemSphere->SetupAttachment(GetRootComponent());
	ItemSphere->InitSphereRadius(80.f);
	ItemSphere->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);


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

	// 현재 캐릭터 성별 정보 저장
	if (this->GetName().Contains("Boy")) SetStat(EPlayerStat::Gender, 1);
	if (this->GetName().Contains("Girl")) SetStat(EPlayerStat::Gender, 2);

	// 아이템 정보 관련
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
		CombatTargetLocation = CombatTarget->GetActorLocation(); // 전투 타겟의 위치 정보 받아오기
		UIManager->SetEnemyLocation(CombatTargetLocation); // 화살표와 체력 정보 띄우기 위함

        if (CombatTarget->GetEnemyMovementStatus() == EEnemyMovementStatus::EMS_Dead) // 현재 전투 타겟이 죽었다면
        {
			for (int i = 0; i < Targets.Num(); i++)
			{
				if (CombatTarget == Targets[i]) //already exist
				{
					Targets.Remove(CombatTarget); //타겟팅 가능 몹 배열에서 제거
				}
			}
			// 전투 타겟 해제
            CombatTarget = nullptr;
            bHasCombatTarget = false;

            if (UIManager->IsTargetArrowVisible()) 
            {// 화살표 및 몬스터 체력바 제거
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

	// 액션은 키를 누르거나 놓는 즉시 한번만 실행
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMain::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("LMB", IE_Pressed, this, &AMain::LMBDown); // 왼쪽 마우스 버튼 다운
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

// 매 프레임마다 키가 눌렸는지 안 눌렸는지 확인될 것
// 키가 안 눌렸으면 Value는 0
void AMain::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f) && bCanMove)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation(); // 회전자 반환 함수
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);

		if (bRunning && PlayerStats[EPlayerStat::SP] > 0.f) // 달리고 있는 상태 + 스태미나가 0이상일 때 스태미나 감소
			AddSP(-1);
	}
}

void AMain::MoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f) && bCanMove)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation(); // 회전자 반환 함수
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);

		if (bRunning && PlayerStats[EPlayerStat::SP] > 0.f)// 달리고 있는 상태 + 스태미나가 0이상일 때 스태미나 감소
			AddSP(-1);
	}
}

void AMain::Run(float Value)
{
	float currentSP = PlayerStats[EPlayerStat::SP];
	if (!Value || currentSP <= 0.f) // 쉬프트키 안 눌려 있거나 스태미나가 0 이하일 때
	{
		bRunning = false;
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed; //속도 하향

		if (currentSP < PlayerStats[EPlayerStat::MaxSP] && !recoverySP) // 스태미나 Full 상태가 아니면
		{ // 스태미나 자동 회복
			if (!GetWorldTimerManager().IsTimerActive(SPTimer))
			{
				recoverySP = true;
				GetWorldTimerManager().SetTimer(SPTimer, this, &AMain::RecoverySP, SPDelay, true);
			}
		}
	}
	else if(!bRunning && currentSP >= 30.f) // 쉬프트키가 눌려있고 스태미나 5 이상에 달리는 상태가 아니면
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
	bLMBDown = true;
	// Targeting Off
	if (CombatTarget)
	{
		if (UIManager->IsTargetArrowVisible())
		{ // 화살표 및 체력바 제거
			UIManager->RemoveTargetArrow();
			UIManager->RemoveEnemyHPBar();
		}
		// 전투 타겟 해제
		CombatTarget = nullptr;
		bHasCombatTarget = false;
	}

	// 다음 대사 출력 관련
	if (DialogueManager->IsDialogueUIVisible()
		&& DialogueManager->GetDialogueUI()->GetCurrentState() != 3
		&& !UIManager->IsMenuVisible())
	{
		if (DialogueManager->GetDialogueUI()->IsInputDisabled()) return;
		else DialogueManager->GetDialogueUI()->Interact();
	}


	if (!MainAnimInstance || !NormalMontage) return;
	if (MainAnimInstance->Montage_IsPlaying(NormalMontage) == true) return; // 중복 재생 방지

	// 아이템 상호작용 몽타주 관련
	// 오버랩된 아이템 존재, 장착한 무기가 없을 때
	if (ActiveOverlappingItem && !EquippedWeapon)
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

void AMain::LMBUp()
{
	bLMBDown = false;
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
	if (DialogueManager->GetDialogueNum() < 3 || DialogueManager->GetDialogueNum() > 20 ) return;

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
			for (int i = 0; i < Targets.Num(); i++)
			{
				if (Enemy == Targets[i]) //already exist
					return;
			}
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
			for (int i = 0; i < Targets.Num(); i++)
			{
				if (Enemy == Targets[i]) // already exist
				{
					Targets.Remove(Enemy); //타겟팅 가능 몹 배열에서 제거
				}
			}

			if (Targets.Num() == 0) // 타겟팅 가능 몬스터가 없으면
			{
				bOverlappingCombatSphere = false;
			}

			if (CombatTarget == Enemy) // 현재 (타겟팅된)전투 타겟과 (전투 범위를 나간 몬스터가)동일하다면
			{
				// 화살표 및 체력바 제거 & 전투 타겟 해제
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
		if (targetIndex >= Targets.Num()) //타겟인덱스가 총 타겟 가능 몹 수 이상이면 다시 0으로 초기화
		{
			targetIndex = 0;
		}

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
	if (ToSpawn && currentMP >= 15) // 마법 사용에 필요한 최소 MP가 15
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

				// 적에게로 이동하는 공격은 공격이 어디로 날라갈지를 정해줘야함.
				if ((GetSkillNum() == 1 || GetSkillNum() == 3) && MagicAttack.IsValid() && CombatTarget)
					MagicAttack->Target = CombatTarget;
			}
		}), 0.6f, false); // 0.6초 뒤 실행, 반복X

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
		if (DamageCauser) // 확인 필요
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
		// HP 자동 회복
		if(!GetWorldTimerManager().IsTimerActive(HPTimer))
			GetWorldTimerManager().SetTimer(HPTimer, this, &AMain::RecoveryHP, HPDelay, true);	
	}

	SetStat(EPlayerStat::HP, currentHP);

	if (UGameplayStatics::GetCurrentLevelName(GetWorld()).Contains("boss")) // 보스 스테이지
	{
		MagicAttack = Cast<AMagicSkill>(DamageCauser);
		if (MagicAttack == nullptr) return DamageAmount;

		int TargetIndex = MagicAttack->index;

		if (TargetIndex == 11) // 보스 몬스터에게 공격 당함
		{
			for (auto NPC : NPCManager->GetNPCMap())
			{
				if (NPC.Value->GetAgroTargets().Num() == 0) // Npc의 인식 범위에 아무도 없을 때
				{
					AEnemy* BossEnemy = Cast<AEnemy>(UGameplayStatics::GetActorOfClass(GetWorld(), NPC.Value->GetBoss()));
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

	// 자동 회복 종료
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

	// 자동 회복 재개
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
	if (currentLevel == 5) return; // 최고 레벨일 때는 리턴

	float currentExp = PlayerStats[EPlayerStat::Exp];
	float MaxExp = PlayerStats[EPlayerStat::MaxExp];

	currentExp += Value;

	float currentHP = PlayerStats[EPlayerStat::HP];
	float MaxHP = PlayerStats[EPlayerStat::MaxHP];
	float currentMP = PlayerStats[EPlayerStat::MP];
	float MaxMP = PlayerStats[EPlayerStat::MaxMP];
	float currentSP = PlayerStats[EPlayerStat::SP];
	float MaxSP = PlayerStats[EPlayerStat::MaxSP];

	if (currentExp >= MaxExp) // 다음 레벨로의 경험치를 모두 채웠다면
	{
		currentLevel += 1; // 레벨 업
        if (LevelUpSound)
            UAudioComponent* AudioComponent = UGameplayStatics::SpawnSound2D(this, LevelUpSound);

		// 경험치 수치 update
		if (currentExp == MaxExp)
		{
			currentExp = 0.f;
		}
		else
		{
			currentExp -= MaxExp;
		}

		if (currentLevel == 5) currentExp = MaxExp;

		// 레벨에 따른 상태 능력치 최대치 초기화
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

void AMain::Escape() // 긴급 탈출, 순간 이동
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

void AMain::SetLevel5() // level cheat, 즉시 최대 레벨 도달
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