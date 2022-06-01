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

// Sets default values
AMain::AMain()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Create CameraBoom (pulls towards the playter if there's a collision), 콜리전이 있으면 카메라를 플레이어쪽으로 당김 
	CameraBoom = CreateAbstractDefaultSubobject<USpringArmComponent>(TEXT("Camera"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 500.f; //Camera follows at this distance
	CameraBoom->bUsePawnControlRotation = true; // Rotate arm based on controller
	// but npc, enemy들도 여기에 콜리전으로 해당되어 게임 플레이가 불편하므로 콜리전 테스트 끔
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->TargetArmLength = DefaultArmLength;

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

	MaxHP = 100.f;
	HP = 65.f;
	MaxMP = 100.f;
	MP = 50.f;
	MaxSP = 300.f;
	SP = 200.f;

	InterpSpeed = 15.f;
	bInterpToEnemy = false;

	CombatSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatSphere"));
	CombatSphere->SetupAttachment(GetRootComponent());
	CombatSphere->InitSphereRadius(600.f);

	bOverlappingCombatSphere = false;
	bHasCombatTarget = false;
	targetIndex = 0;

	AttackArrow = CreateAbstractDefaultSubobject<UArrowComponent>(TEXT("AttackArrow"));
	AttackArrow->SetupAttachment(GetRootComponent());
}


// Called when the game starts or when spawned
void AMain::BeginPlay()
{
	Super::BeginPlay();

	CombatSphere->OnComponentBeginOverlap.AddDynamic(this, &AMain::CombatSphereOnOverlapBegin);
	CombatSphere->OnComponentEndOverlap.AddDynamic(this, &AMain::CombatSphereOnOverlapEnd);

	MainPlayerController = Cast<AMainPlayerController>(GetController());
}

// Called every frame
void AMain::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bInterpToEnemy && CombatTarget)
	{
		FRotator LookAtYaw = GetLookAtRotationYaw(CombatTarget->GetActorLocation());
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
	}
}

// Called to bind functionality to input
void AMain::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	// 액션은 키를 누르거나 놓는 즉시 한번만 실행
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("LMB", IE_Pressed, this, &AMain::LMBDown);
	PlayerInputComponent->BindAction("LMB", IE_Released, this, &AMain::LMBUp);

	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &AMain::Attack);

	PlayerInputComponent->BindAction("Targeting", IE_Pressed, this, &AMain::Targeting);


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
	if ((Controller != nullptr) && (Value != 0.0f) && (!bAttacking))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation(); // 회전자 반환 함수
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AMain::MoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f) && (!bAttacking))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation(); // 회전자 반환 함수
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
	}
}

void AMain::Run(float Value)
{
	if (!Value || SP <= 0.f) //쉬프트키 안 눌려 있거나 스태미나가 0 이하일 때
	{
		bRunning = false;
		GetCharacterMovement()->MaxWalkSpeed = 350.f; //속도 하향
		if (SP < MaxSP)
		{
			SP += 0.1f;
		}
	}
	else if(!bRunning && SP >= 1.f) //쉬프트키가 눌려있고 달리는 상태가 아니면
	{
		bRunning = true;
		GetCharacterMovement()->MaxWalkSpeed = 600.f; //속도 상향		
	}
	
	if (bRunning && SP >= 0.f)// 달리고 있는 상태 + 스태미나가 5이상일 때 스태미나 감소
	{
		SP -= 1.f;
		//UE_LOG(LogTemp, Log, TEXT("Text, %f"), SP);
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

	const float NewTargetArmLength = CameraBoom->TargetArmLength + Value * ZoomStep;
	CameraBoom->TargetArmLength = FMath::Clamp(NewTargetArmLength, MinZoomLength, MaxZoomLength);
}

void AMain::LMBDown() //Left Mouse Button
{
	bLMBDown = true;

	if (ActiveOverlappingItem && !EquippedWeapon)
	{
		AWeapon* Weapon = Cast<AWeapon>(ActiveOverlappingItem);
		if (Weapon)
		{
			Weapon->Equip(this);
			SetActiveOverlappingItem(nullptr);
		}
	}

	// Targeting Off
	if (CombatTarget)
	{
		if (MainPlayerController->bTargetArrowVisible)
		{
			MainPlayerController->RemoveTargetArrow();
		}
		CombatTarget = nullptr;
		bHasCombatTarget = false;
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
	if (EquippedWeapon)
	{
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

void AMain::AttackEnd()
{
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
				MainPlayerController->bTargetArrowVisible = false;
				MainPlayerController->RemoveTargetArrow();
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
			MainPlayerController->bTargetArrowVisible = false;
			MainPlayerController->RemoveTargetArrow();
		}
		bHasCombatTarget = true;
		CombatTarget = Targets[targetIndex];
		//UE_LOG(LogTemp, Log, TEXT("%s"), *(CombatTarget->GetName()));
		targetIndex++;

		MainPlayerController->DisplayTargetArrow(); 
	}
}

void AMain::Spawn()
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

					world->SpawnActor<AMagicSkill>(ToSpawn, spawnLocation, rotator, spawnParams);
				}
			}), 0.6f, false); // 0.6초 뒤 실행, 반복X
	}

}

