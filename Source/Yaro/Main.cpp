// Fill out your copyright notice in the Description page of Project Settings.


#include "Main.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/world.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/PlayerCameraManager.h"
#include "Weapon.h"
#include "Animation/AnimInstance.h"
#include "Components/SphereComponent.h"
#include "Enemy.h"
#include "Components/ArrowComponent.h"
#include "MagicSkill.h"
#include "Kismet/KismetMathLibrary.h"
#include "MainPlayerController.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "YaroSaveGame.h"
#include "ItemStorage.h"
#include "DialogueUI.h"
#include "YaroCharacter.h"

// Sets default values
AMain::AMain()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

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
	FollowCamera = CreateAbstractDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	// Attach the camera to the end of the boom and let the boom adjust to match
	// the controller orientation
	FollowCamera->bUsePawnControlRotation = false;

	// Set out turn rates for input
	BaseTurnRate = 65.f;
	BaseLookUpRate = 65.f;

	// Don't rotate when the controller rotates.
	// Let that just affect the camera.
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;


	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.f, 0.0f); //... at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 400.f;
	GetCharacterMovement()->AirControl = 0.2f;
	GetCharacterMovement()->MaxStepHeight = 50.f;
	GetCharacterMovement()->SetWalkableFloorAngle(50.f);
	GetCharacterMovement()->MaxWalkSpeed = 350.f;

	MaxHP = 200.f;
	HP = 200.f;
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

	InterpSpeed = 15.f;
	bInterpToEnemy = false;

	CombatSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatSphere"));
	CombatSphere->SetupAttachment(GetRootComponent());
	CombatSphere->InitSphereRadius(600.f);
	CombatSphere->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);

	bOverlappingCombatSphere = false;
	bHasCombatTarget = false;
	targetIndex = 0;

	AttackArrow = CreateAbstractDefaultSubobject<UArrowComponent>(TEXT("AttackArrow"));
	AttackArrow->SetupAttachment(GetRootComponent());
	AttackArrow->SetRelativeLocation(FVector(160.f, 4.f, 26.f));

	MovementStatus = EMovementStatus::EMS_Normal;

	DeathDelay = 3.f;

	bESCDown = false;

	InteractionRange = CreateDefaultSubobject<UCapsuleComponent>(TEXT("InteractionRange"));
	InteractionRange->SetupAttachment(GetMesh());
	InteractionRange->SetRelativeLocation(FVector(0.f, 70.f, 90.f));
	InteractionRange->SetRelativeRotation(FRotator(0.f, 0.f, 90.f));
	InteractionRange->SetRelativeScale3D(FVector(1.5f, 1.f, 2.2f));

}


// Called when the game starts or when spawned
void AMain::BeginPlay()
{
	Super::BeginPlay();

	CombatSphere->OnComponentBeginOverlap.AddDynamic(this, &AMain::CombatSphereOnOverlapBegin);
	CombatSphere->OnComponentEndOverlap.AddDynamic(this, &AMain::CombatSphereOnOverlapEnd);


	if (this->GetName().Contains("Boy")) Gender = 1;
	if (this->GetName().Contains("Girl")) Gender = 2;

	Storage = GetWorld()->SpawnActor<AItemStorage>(ObjectStorage);

	InteractionRange->OnComponentBeginOverlap.AddDynamic(this, &AMain::InteractionRangeOnOverlapBegin);
	InteractionRange->OnComponentEndOverlap.AddDynamic(this, &AMain::InteractionRangeOnOverlapEnd);

}

// Called every frame
void AMain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    if (!MainPlayerController)
        MainPlayerController = Cast<AMainPlayerController>(GetController());

	if (bInterpToEnemy && CombatTarget)
	{
		FRotator LookAtYaw = GetLookAtRotationYaw(CombatTarget->GetActorLocation());
		FRotator InterpRotation = FMath::RInterpTo(GetActorRotation(), LookAtYaw, DeltaTime, InterpSpeed); //smooth transition

		SetActorRotation(InterpRotation);
	}

    if (bInterpToNpc && TargetNpc)
    {
        FRotator LookAtYaw = GetLookAtRotationYaw(TargetNpc->GetActorLocation());
        FRotator InterpRotation = FMath::RInterpTo(GetActorRotation(), LookAtYaw, DeltaTime, InterpSpeed); //smooth transition

        SetActorRotation(InterpRotation);
    }

	if (CombatTarget)
	{
		CombatTargetLocation = CombatTarget->GetActorLocation();
		if (MainPlayerController)
		{
			MainPlayerController->EnemyLocation = CombatTargetLocation;
		}

        if (CombatTarget->EnemyMovementStatus == EEnemyMovementStatus::EMS_Dead)
        {
            CombatTarget = nullptr;
            bHasCombatTarget = false;
            if (MainPlayerController->bTargetArrowVisible)
            {
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

	// 액션은 키를 누르거나 놓는 즉시 한번만 실행
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMain::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("LMB", IE_Pressed, this, &AMain::LMBDown);
	PlayerInputComponent->BindAction("LMB", IE_Released, this, &AMain::LMBUp);

	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &AMain::Attack);

	PlayerInputComponent->BindAction("Targeting", IE_Pressed, this, &AMain::Targeting);

	PlayerInputComponent->BindAction("ESC", IE_Pressed, this, &AMain::ESCDown);
	PlayerInputComponent->BindAction("ESC", IE_Released, this, &AMain::ESCUp);

	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &AMain::StartDialogue);

    PlayerInputComponent->BindAction("ShowManual", IE_Pressed, this, &AMain::ShowManual);


	// Axis는 매 프레임마다 호출
	//“키 이름”, bind할 함수가 있는 클래스의 인스턴스, bind할 함수의 주소
	PlayerInputComponent->BindAxis("MoveForward", this, &AMain::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMain::MoveRight);
	PlayerInputComponent->BindAxis("Run", this, &AMain::Run);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AMain::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AMain::LookUpAtRate);

	PlayerInputComponent->BindAxis("CameraZoom", this, &AMain::CameraZoom);

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

		if (bRunning && SP >= 0.f)// 달리고 있는 상태 + 스태미나가 0이상일 때 스태미나 감소
		{
			SP -= 1.f;
		}
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

		if (bRunning && SP > 0.f)// 달리고 있는 상태 + 스태미나가 0이상일 때 스태미나 감소
		{
			SP -= 1.f;
		}
	}
}

void AMain::Run(float Value)
{
	if (!Value || SP <= 0.f) //쉬프트키 안 눌려 있거나 스태미나가 0 이하일 때
	{
		bRunning = false;
		GetCharacterMovement()->MaxWalkSpeed = 350.f; //속도 하향

		if (SP < MaxSP && !recoverySP)
		{
			recoverySP = true;
			GetWorldTimerManager().SetTimer(SPTimer, this, &AMain::RecoverySP, SPDelay, true);
		}
	}
	else if(!bRunning && SP >= 1.f) //쉬프트키가 눌려있고 달리는 상태가 아니면
	{
		bRunning = true;
		GetCharacterMovement()->MaxWalkSpeed = 600.f; //속도 상향		
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

	if (MainPlayerController->DialogueNum == 0) return;

	const float NewTargetArmLength = CameraBoom->TargetArmLength + Value * ZoomStep;
	CameraBoom->TargetArmLength = FMath::Clamp(NewTargetArmLength, MinZoomLength, MaxZoomLength);
}

void AMain::LMBDown() //Left Mouse Button Down
{
	bLMBDown = true;

	if (ActiveOverlappingItem && !EquippedWeapon)
	{
        UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && NormalMontage)
		{
			if (AnimInstance->Montage_IsPlaying(NormalMontage) == true) return;
			bCanMove = false;
            FRotator LookAtYaw = GetLookAtRotationYaw(ActiveOverlappingItem->GetActorLocation());
            SetActorRotation(LookAtYaw);

			AnimInstance->Montage_Play(NormalMontage);
			AnimInstance->Montage_JumpToSection(FName("PickWand"), NormalMontage);
		}
	}

	// Targeting Off
	if (CombatTarget)
	{
		if (MainPlayerController->bTargetArrowVisible)
		{
			MainPlayerController->RemoveTargetArrow();
			MainPlayerController->RemoveEnemyHPBar();
		}
		CombatTarget = nullptr;
		bHasCombatTarget = false;
	}

	if (MainPlayerController->bDialogueUIVisible && MainPlayerController->DialogueUI->CurrentState != 3 && !MainPlayerController->bMenuVisible)
	{
        if (MainPlayerController->DialogueUI->bDisableMouseAndKeyboard) return;
		else MainPlayerController->DialogueUI->Interact();
 
	}
    
}

void AMain::LMBUp()
{
	bLMBDown = false;
}

void AMain::SetInterpToEnemy(bool Interp)
{
	bInterpToEnemy = Interp;
}

FRotator AMain::GetLookAtRotationYaw(FVector Target)
{
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Target);
	FRotator LookAtRotationYaw(0.f, LookAtRotation.Yaw, 0.f);
	return LookAtRotationYaw;
}

void AMain::Attack()
{
	if (MainPlayerController->DialogueNum < 3) return;

	if (EquippedWeapon && !bAttacking && MovementStatus != EMovementStatus::EMS_Dead && !MainPlayerController->bDialogueUIVisible)
	{
		SkillNum = MainPlayerController->WhichKeyDown();
		UBlueprintGeneratedClass* LoadedBP = LoadObject<UBlueprintGeneratedClass>(GetWorld(), TEXT("/Game/Blueprint/MagicAttacks/MainCharacter/Wind_Hit_Attack.WindAttack_C")); //초기화 안 하면 ToSpawn에 초기화되지 않은 변수 넣었다고 오류남
		switch (SkillNum)
		{
			case 1:
				LoadedBP = LoadObject<UBlueprintGeneratedClass>(GetWorld(), TEXT("/Game/Blueprint/MagicAttacks/MainCharacter/1_Wind_Hit_Attack.1_Wind_Hit_Attack_C"));
				break;
			case 2:
				if (Level < 2) return;
				LoadedBP = LoadObject<UBlueprintGeneratedClass>(GetWorld(), TEXT("/Game/Blueprint/MagicAttacks/MainCharacter/2_ShockAttack.2_ShockAttack_C"));
				break;
			case 3:
                if (Level < 3) return;
				LoadedBP = LoadObject<UBlueprintGeneratedClass>(GetWorld(), TEXT("/Game/Blueprint/MagicAttacks/MainCharacter/3_Electricball_Hit_Attack.3_Electricball_Hit_Attack_C"));
				break;
            case 4:
                if (Level < 4) return;
                LoadedBP = LoadObject<UBlueprintGeneratedClass>(GetWorld(), TEXT("/Game/Blueprint/MagicAttacks/MainCharacter/4_PoisonAttack.4_PoisonAttack_C"));
                break;
            case 5:
                if (Level < 5) return;
                LoadedBP = LoadObject<UBlueprintGeneratedClass>(GetWorld(), TEXT("/Game/Blueprint/MagicAttacks/MainCharacter/5_EarthAttack.5_EarthAttack_C"));
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
            bCanMove = false;

			AnimInstance->Montage_Play(CombatMontage);
			AnimInstance->Montage_JumpToSection(FName("Attack"), CombatMontage);

			Spawn();			
		}
	}

}

void AMain::AttackEnd()
{
    bCanMove = true;

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
				{
					return;
				}
			}
			Targets.Add(Enemy); 
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
				if (Enemy == Targets[i]) //already exist
				{
					Targets.Remove(Enemy); //타겟팅 가능 몹 배열에서 제거

				}
			}
			if (Targets.Num() == 0)
			{
				bOverlappingCombatSphere = false;
			}

			if (CombatTarget == Enemy)
			{
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
		if (targetIndex >= Targets.Num()) //타겟인덱스가 총 타겟 가능 몹 수 이상이면 다시 0으로 초기화
		{
			targetIndex = 0;
		}
		//There is already exist targeted enemy, then targetArrow remove
		if (MainPlayerController->bTargetArrowVisible)
		{
			MainPlayerController->RemoveTargetArrow();
			MainPlayerController->RemoveEnemyHPBar();
		}
		bHasCombatTarget = true;
		if (Targets.Num() != 0 && !bAutoTargeting)
		{
            CombatTarget = Targets[targetIndex];
            targetIndex++;
		}


		MainPlayerController->DisplayTargetArrow(); 
		MainPlayerController->DisplayEnemyHPBar();
	}
}

void AMain::Spawn() //Spawn Magic
{
	if (ToSpawn && MP >= 15)
	{
        //UE_LOG(LogTemp, Log, TEXT("Spawn"));

		//If player have not enough MP, then player can't use magic
		switch (SkillNum)
		{
        case 2:
            if (MP < 20.f) return;
            break;
        case 3:
            if (MP < 25.f) return;
            break;
        case 4:
            if (MP < 30.f) return;
            break;
        case 5:
            if (MP < 35.f) return;
            break;
		}	


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
					if (SkillNum != 1 && SkillNum != 3 && CombatTarget)
					{
						spawnLocation = CombatTarget->GetActorLocation();
					}

					MagicAttack = world->SpawnActor<AMagicSkill>(ToSpawn, spawnLocation, rotator, spawnParams);	
					// 적에게로 이동하는 공격은 공격이 어디로 날라갈지를 정해줘야함.
					if ((SkillNum == 1 || SkillNum == 3) && MagicAttack && CombatTarget) MagicAttack->Target = CombatTarget;
				}
			}), 0.6f, false); // 0.6초 뒤 실행, 반복X

		switch (SkillNum)// decrease MP
		{
			case 1:
				MP -= 15.f;
				break;
			case 2:
				MP -= 20.f;
				break;
			case 3:
				MP -= 25.f;
				break;
            case 4:
                MP -= 30.f;
                break;
            case 5:
                MP -= 35.f;
                break;
		}

		//if(CombatTarget) CombatTarget->bAttackFromPlayer = true;
		GetWorldTimerManager().SetTimer(MPTimer, this, &AMain::RecoveryMP, MPDelay, true);
	}
}


float AMain::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	if (HP - DamageAmount <= 0.f)
	{
		HP = 0.f;
		Die();
		if (DamageCauser)
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
		GetWorldTimerManager().SetTimer(HPTimer, this, &AMain::RecoveryHP, HPDelay, true);	
	}

	return DamageAmount;
}

void AMain::Die()
{
	if (MovementStatus == EMovementStatus::EMS_Dead) return;

	GetWorldTimerManager().ClearTimer(HPTimer);
	GetWorldTimerManager().ClearTimer(MPTimer);
	GetWorldTimerManager().ClearTimer(SPTimer);

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && CombatMontage)
	{
		bCanMove = false;
		AnimInstance->Montage_Play(CombatMontage);
		AnimInstance->Montage_JumpToSection(FName("Death"));
	}
	SetMovementStatus(EMovementStatus::EMS_Dead);

}

void AMain::DeathEnd()
{
	GetMesh()->bPauseAnims = true;
	GetMesh()->bNoSkeletonUpdate = true;
	GetWorldTimerManager().SetTimer(DeathTimer, this, &AMain::Revive, DeathDelay);
}

void AMain::Jump()
{
	if (MovementStatus != EMovementStatus::EMS_Dead && !MainPlayerController->bDialogueUIVisible && bCanMove) // 죽거나 대화 중일 때는 점프 불가, 수동으로 움직임 막았을 때도 불가
	{
		Super::Jump();
	}
}

void AMain::Revive() // if player is dead, spawn player at the initial location
{
	this->SetActorLocation(FVector(-192.f, 5257.f, 3350.f));
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && CombatMontage)
	{
		GetMesh()->bPauseAnims = false;
		AnimInstance->Montage_Play(CombatMontage);
		AnimInstance->Montage_JumpToSection(FName("Revival"));
		GetMesh()->bNoSkeletonUpdate = false;
		HP += 50.f;
	}
}

void AMain::RevivalEnd()
{
	SetMovementStatus(EMovementStatus::EMS_Normal);
	GetWorldTimerManager().SetTimer(HPTimer, this, &AMain::RecoveryHP, HPDelay, true);
	GetWorldTimerManager().SetTimer(MPTimer, this, &AMain::RecoveryMP, MPDelay, true);
	GetWorldTimerManager().SetTimer(SPTimer, this, &AMain::RecoverySP, SPDelay, true);
}

void AMain::InteractionRangeOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (auto actor = Cast<AMain>(OtherActor)) return;

	if (auto actor = Cast<AEnemy>(OtherActor)) return; // 오버랩된 게 Enemy라면 코드 실행X

	if (OtherActor)
	{
		if(!InteractTarget) InteractTarget = Cast<ACharacter>(OtherActor);

	}
}

void AMain::InteractionRangeOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (InteractTarget)
	{
		InteractTarget = nullptr;
	}
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


void AMain::SaveGame()
{

	if (MainPlayerController->bDialogueUIVisible)
	{
        GetWorld()->GetTimerManager().SetTimer(SaveTimer, this, &AMain::SaveGame, 5.f, false);
        return;

	}
    UE_LOG(LogTemp, Log, TEXT("SaveGame"));

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


    SaveGameInstance->DialogueNum = MainPlayerController->DialogueNum;


	SaveGameInstance->CharacterStats.Location = GetActorLocation();
	SaveGameInstance->CharacterStats.Rotation = GetActorRotation();

	if (EquippedWeapon) SaveGameInstance->CharacterStats.WeaponName = EquippedWeapon->Name;

	//Storage->EnemyMap.Empty();

	//if (Enemies.Num() != 0)
	//{
	//	for (int i = 0; i < Enemies.Num(); i++)
	//	{
	//		SaveGameInstance->EnemyInfo.EnemyIndex = Cast<AEnemy>(Enemies[i])->Index;
	//		SaveGameInstance->EnemyInfo.Location = Cast<AEnemy>(Enemies[i])->GetActorLocation();
	//		SaveGameInstance->EnemyInfo.Rotation = Cast<AEnemy>(Enemies[i])->GetActorRotation();


	//		SaveGameInstance->EnemyInfoArray.Add(SaveGameInstance->EnemyInfo);
	//		
	//		TSubclassOf<class AEnemy> instance = Enemies[i]->GetClass();

	//	
	//		Storage->EnemyMap.Add(Enemies[i]->Index, instance);
	//	}
	//	UE_LOG(LogTemp, Log, TEXT("%d, arraynum"), SaveGameInstance->EnemyInfoArray.Num());

	//}

	UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveGameInstance->SaveName, SaveGameInstance->UserIndex);
    
	GetWorld()->GetTimerManager().SetTimer(SaveTimer, this, &AMain::SaveGame, 5.f, false);

}

void AMain::LoadGame()
{
	UYaroSaveGame* LoadGameInstance = Cast<UYaroSaveGame>(UGameplayStatics::CreateSaveGameObject(UYaroSaveGame::StaticClass()));

	LoadGameInstance = Cast<UYaroSaveGame>(UGameplayStatics::LoadGameFromSlot(LoadGameInstance->SaveName, LoadGameInstance->UserIndex));

	MainPlayerController = Cast<AMainPlayerController>(GetController());
	MainPlayerController->DialogueNum = LoadGameInstance->DialogueNum;

	HP = LoadGameInstance->CharacterStats.HP;
	MaxHP = LoadGameInstance->CharacterStats.MaxHP;
	MP = LoadGameInstance->CharacterStats.MP;
	MaxMP = LoadGameInstance->CharacterStats.MaxMP;
	SP = LoadGameInstance->CharacterStats.SP;
	MaxSP = LoadGameInstance->CharacterStats.MaxSP;
	Level = LoadGameInstance->CharacterStats.Level;
	Exp = LoadGameInstance->CharacterStats.Exp;
	MaxExp = LoadGameInstance->CharacterStats.MaxExp;

	SetActorLocation(LoadGameInstance->CharacterStats.Location);
	SetActorRotation(LoadGameInstance->CharacterStats.Rotation);

    if (SP < MaxSP && !recoverySP)
    {
        recoverySP = true;
        GetWorldTimerManager().SetTimer(SPTimer, this, &AMain::RecoverySP, SPDelay, true);
    }

    if (HP < MaxHP)
        GetWorldTimerManager().SetTimer(HPTimer, this, &AMain::RecoveryHP, HPDelay, true);

    if (MP < MaxMP)
		GetWorldTimerManager().SetTimer(MPTimer, this, &AMain::RecoveryMP, MPDelay, true);
	//if (ObjectStorage)
	//{
	//	if (Storage)
	//	{
	//		FString WeaponName = LoadGameInstance->CharacterStats.WeaponName;

	//		if (Storage->WeaponMap.Contains(WeaponName))
	//		{
	//			AWeapon* WeaponToEquip = GetWorld()->SpawnActor<AWeapon>(Storage->WeaponMap[WeaponName]);
	//			WeaponToEquip->Equip(this);
	//		}


	//		//for (int i = 0; i < LoadGameInstance->EnemyInfoArray.Num(); i++)
	//		//{
	//		//	//FString EnemyName = LoadGameInstance->EnemyInfoArray[i].EnemyName;
	//		//	int index = LoadGameInstance->EnemyInfoArray[i].EnemyIndex;
	//		//	if (Storage->EnemyMap.Contains(index))
	//		//	{
	//		//		AEnemy* Enemy = GetWorld()->SpawnActor<AEnemy>(Storage->EnemyMap[index]);
	//		//		Enemy->SetActorLocation(LoadGameInstance->EnemyInfoArray[i].Location);
	//		//		Enemy->SetActorRotation(LoadGameInstance->EnemyInfoArray[i].Rotation);

	//		//	}
	//		//}
	//	}
	//}
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
	Exp += exp;

	if (Exp >= MaxExp)
	{
        Level += 1;
        if (LevelUpSound != nullptr)
            UAudioComponent* AudioComponent = UGameplayStatics::SpawnSound2D(this, LevelUpSound);

		if (Exp == MaxExp)
		{
            Exp = 0.f;
		}
		else
		{
			float tmp = Exp - MaxExp;
			Exp = tmp;
		}

		if (Level == 5) Exp = 100.f;

		switch (Level)
		{
			case 2:
				MaxExp = 150.f;
				MainPlayerController->SystemMessageNum = 6;
				MainPlayerController->SetSystemMessage();
				MaxHP = 225.f;
				MaxMP = 175.f;
				MaxSP = 325.f;
				break;
			case 3:
				MaxExp = 250.f;
                MainPlayerController->SystemMessageNum = 7;
                MainPlayerController->SetSystemMessage();
                MaxHP = 250.f;
                MaxMP = 200.f;
                MaxSP = 350.f;
				break;
			case 4:
				MaxExp = 400.f;
                MainPlayerController->SystemMessageNum = 8;
                MainPlayerController->SetSystemMessage();
                MaxHP = 275.f;
                MaxMP = 225.f;
                MaxSP = 375.f;
				break;
            case 5:
                MainPlayerController->SystemMessageNum = 9;
                MainPlayerController->SetSystemMessage();
                MaxHP = 300.f;
                MaxMP = 250.f;
                MaxSP = 400.f;
                break;
		}

        FTimerHandle Timer;
        GetWorld()->GetTimerManager().SetTimer(Timer, FTimerDelegate::CreateLambda([&]()
            {
                MainPlayerController->RemoveSystemMessage();
            }), 3.f, false);
	}
}

void AMain::ShowManual()
{
	if (MainPlayerController->bManualVisible) MainPlayerController->RemoveManual();
	else MainPlayerController->DisplayManual();
}