// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Student.h"
#include "Main.generated.h"

UENUM(BlueprintType)
enum class EMovementStatus :uint8
{
	EMS_Normal			UMETA(DeplayName = "Normal"),
	EMS_Dead			UMETA(DeplayName = "Dead"),

};

UENUM(BlueprintType)
enum class EPlayerStat :uint8
{
	Gender,
	MaxHP,
	HP,
	MaxMP,
	MP,
	MaxSP, 
	SP,
	Level,
	MaxExp,
	Exp,
	PotionNum,
};

USTRUCT()
struct FLevelStats
{
	GENERATED_BODY()

	float MaxExp = 0.f;

	float MaxHP = 0.f;

	float MaxMP = 0.f;

	float MaxSP = 0.f;

	FLevelStats()
		: MaxExp(0.f), MaxHP(0.f), MaxMP(0.f), MaxSP(0.f)
	{
	}

	FLevelStats(float InMaxExp, float InMaxHP, float InMaxMP, float InMaxSP)
		: MaxExp(InMaxExp), MaxHP(InMaxHP), MaxMP(InMaxMP), MaxSP(InMaxSP)
	{
	}
};

UCLASS()
class YARO_API AMain : public AStudent
{
	GENERATED_BODY()

public: // Constructor and overrides
	AMain();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Jump() override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	void CombatSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	void CombatSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;
	void Attack() override;
	void AttackEnd() override;
	void Spawn() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class AMainPlayerController* MainPlayerController;

	UPROPERTY()
	class UMainAnimInstance* MainAnimInstance;

	//Managers
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UGameManager* GameManager;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UDialogueManager* DialogueManager;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UUIManager* UIManager;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UNPCManager* NPCManager;

	// Camera settings
	// CameraBoom positioning the camera behind the player, 카메라붐은 플레이어 뒤에 카메라를 위치시킴
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	// About CameraZoom
	float MinZoomLength = 100.f;
	float MaxZoomLength = 800.f;
	float ZoomStep = 30.f;

	// Base turn rates to scale turning functions for the camera
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Camera)
	float BaseTurnRate;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Camera)
	float BaseLookUpRate;

	// Movements
	EMovementStatus MovementStatus = EMovementStatus::EMS_Normal;
	bool bRunning = false;
	bool bCanMove = true;
	bool bFallenInDungeon = false; // 플레이어가 던전 범위 밖으로 추락했을 때 true
	int32 FallCount = 0;

	// Stats
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Stats")
	TMap<EPlayerStat, float> PlayerStats;

	// 스탯 자동 회복
	FTimerHandle HPTimer, MPTimer, SPTimer;
	float HPDelay = 3.f;
	float MPDelay = 2.f;
	float SPDelay = 0.5f;
	bool recoverySP = false;

	TMap<int32, FLevelStats> LevelData;


	// Combat
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TArray<AEnemy*> Targets; // 범위에 들어온 타겟팅 가능 몹 배열
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bHasCombatTarget = false;
	int32 TargetIndex = 0;
	bool bAutoTargeting = false;
	float DeathDelay = 3.f;


	// Item
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Items)
	class USphereComponent* ItemSphere;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Items)
	class AItem* ActiveOverlappingItem;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Items)
	class AActor* CurrentOverlappedActor;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items)
	class AWeapon* EquippedWeapon;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Items)
	class AItem* ItemInHand;
	UPROPERTY(EditDefaultsOnly, Category = Items)
	TSubclassOf<class AItemStorage> ObjectStorage;
	UPROPERTY()
	class AItemStorage* Storage;


	UPROPERTY()
	class USoundBase* LevelUpSound;


	// Initialization
	void InitializeCamera();
	void InitializeStats();
	USphereComponent* CreateSphereComponent(FName Name, float Radius, FVector RelativeLocation);
	void InitializeManagers();
	void BindComponentEvents();
	void InitializeLevelData();


public: // Getters and Setters
	AMainPlayerController* GetMainPlayerController() { return MainPlayerController; }

	//Camera
	class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	//Movements
	EMovementStatus GetMovementStatus() { return MovementStatus; }
	void SetMovementStatus(EMovementStatus Status) { MovementStatus = Status; }

	UFUNCTION(BlueprintCallable)
	void SetCanMove(bool value) { bCanMove = value; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	int32 GetFallCount() { return FallCount; }
	UFUNCTION(BlueprintCallable)
	void SetFallCount(int32 Count) { FallCount = Count; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsFallenInDungeon() { return bFallenInDungeon; }
	UFUNCTION(BlueprintCallable)
	void SetFallenInDungeon(bool Value) { bFallenInDungeon = Value; }

	UFUNCTION(BlueprintCallable)
	bool IsDead() { return MovementStatus == EMovementStatus::EMS_Dead ? true : false; }

	bool IsInAir();

	//Stats
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetStat(EPlayerStat StatName) const;
	UFUNCTION(BlueprintCallable)
	void SetStat(EPlayerStat StatName, float Value);

	//Item
	const TMap<FString, TSubclassOf<class AItem>>* AMain::GetItemMap();
	
	void SetActiveOverlappingItem(AItem* Item) { ActiveOverlappingItem = Item; }

	AWeapon* GetEquippedWeapon() { return EquippedWeapon; }
	void SetEquippedWeapon(AWeapon* wp) { EquippedWeapon = wp; }

	AItem* GetItemInHand() { return ItemInHand; }
	void SetItemInHand(AItem* item) { ItemInHand = item; }

	// Combat
	TArray<AEnemy*> GetTargets() { return Targets; }

	void SetAutoTargeting(bool value) { bAutoTargeting = value; }

public: // Core Methods
	// Movements
	void Move(float Value, EAxis::Type Axis);
	void MoveForward(float Value); // Called for forwards/backwards input
	void MoveRight(float Value); // Called for side to side input
	void Run(float Value); // Called for shift input

	// Camera
	// @param(매개변수 설명) Rate This is a normalized rate, 1.0 means 100% of desired turn rate
	void TurnAtRate(float Rate);
	// @param(매개변수 설명) Rate This is a normalized rate,  1.0 means 100% of desired look up/down rate
	void LookUpAtRate(float Rate);
	void CameraZoom(const float Value);

	//Stats
	void AddHP(float Value);
	void AddSP(float Value) { SetStat(EPlayerStat::SP, PlayerStats[EPlayerStat::SP] + Value); }

	void RecoveryHP();
	void RecoveryMP();
	void RecoverySP();

	void GainExp(float Value);
	void LevelUp();

	//Item
	void PlayMontageWithItem();

    UFUNCTION()
    virtual void ItemSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
    UFUNCTION()
    virtual void ItemSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	//Combat
	void Targeting();
	void UnsetCombatTarget();

	void Die();

	UFUNCTION(BlueprintCallable)
	void DeathEnd();

	void Revive();

	UFUNCTION(BlueprintCallable)
	void RevivalEnd();

	UFUNCTION(BlueprintCallable)
	void RecoverWithLogo();

	void UsePotion();


	void LMBDown();

};
