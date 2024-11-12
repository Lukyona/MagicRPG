// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Student.h"
#include "Main.generated.h"

// 블루프린트에서 쓰려면 다이나믹 멀티캐스트여야함
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDele_Dynamic);

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


UCLASS()
class YARO_API AMain : public AStudent
{
	GENERATED_BODY()

protected:
	    UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UGameManager* GameManager;

	    UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UDialogueManager* DialogueManager;

	    UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UUIManager* UIManager;

	    UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UNPCManager* NPCManager;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	EMovementStatus MovementStatus;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Controller")
	class AMainPlayerController* MainPlayerController;

	class UMainAnimInstance* MainAnimInstance;

	UPROPERTY(EditDefaultsOnly, Category = "SavedData")
	TSubclassOf<class AItemStorage> ObjectStorage;

	class AItemStorage* Storage;

	// CameraBoom positioning the camera behind the player, 카메라붐은 플레이어 뒤에 카메라를 위치시킴
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	// About CameraZoom
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "CameraZoom")
	float MinZoomLength = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "CameraZoom")
	float MaxZoomLength = 800.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "CameraZoom")
	float ZoomStep = 30.f;

	// Base turn rates to scale turning functions for the camera
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Camera)
	float BaseTurnRate;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Camera)
	float BaseLookUpRate;

	//달리는 상태인지 확인
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement)
	bool bRunning;

	bool bCanMove = true;

	bool bFallenInDungeon = false; // 플레이어가 던전 범위 밖으로 추락했을 때 true

	int32 FallCount = 0;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Stats")
	TMap<EPlayerStat, float> PlayerStats;


	// 스탯 자동 회복
	FTimerHandle HPTimer;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float HPDelay;

	FTimerHandle MPTimer;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float MPDelay;

	FTimerHandle SPTimer;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float SPDelay;

	bool recoverySP = false;


	//About Combat System
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TArray<AEnemy*> Targets; // 범위에 들어온 타겟팅 가능 몹 배열

	int targetIndex;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bHasCombatTarget;

	bool bAutoTargeting = false;


	FTimerHandle DeathTimer;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	float DeathDelay;


	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Items)
	class AItem* ActiveOverlappingItem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items)
	class AWeapon* EquippedWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Items)
	class AItem* ItemInHand;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Items)
	class USphereComponent* ItemSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Items)
	class AActor* CurrentOverlappedActor;


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class USoundBase* LevelUpSound;

	bool bLMBDown;
	bool bESCDown;


public:
	// Sets default values for this character's properties
	AMain();

	AMainPlayerController* GetMainPlayerController() { return MainPlayerController; }

	FORCEINLINE void SetMovementStatus(EMovementStatus Status) { MovementStatus = Status; }
	FORCEINLINE EMovementStatus GetMovementStatus() { return MovementStatus; }

	UFUNCTION(BlueprintCallable)
	void SetCanMove(bool value) { bCanMove = value; }

	void InitializeStats();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetStat(EPlayerStat StatName) const;
	UFUNCTION(BlueprintCallable)
	void SetStat(EPlayerStat StatName, float Value);

	void AddHP(float Value) { SetStat(EPlayerStat::HP, PlayerStats[EPlayerStat::HP] + Value);}
	void AddMP(float Value) { SetStat(EPlayerStat::MP, PlayerStats[EPlayerStat::MP] + Value); }
	void AddSP(float Value) { SetStat(EPlayerStat::SP, PlayerStats[EPlayerStat::SP] + Value); }

	void RecoveryHP();
	void RecoveryMP();
	void RecoverySP();

	void GainExp(float Value);

	const TMap<FString, TSubclassOf<class AItem>>* AMain::GetItemMap();
	
	FORCEINLINE void SetActiveOverlappingItem(AItem* Item) { ActiveOverlappingItem = Item; }

	void SetEquippedWeapon(AWeapon* wp) { EquippedWeapon = wp; }
	AWeapon* GetEquippedWeapon() { return EquippedWeapon; }

	void SetItemInHand(AItem* item) { ItemInHand = item; }
	AItem* GetItemInHand() { return ItemInHand; }

	UFUNCTION(BlueprintCallable)
	void SetFallCount(int32 Count) { FallCount = Count; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	int32 GetFallCount() { return FallCount; }

	UFUNCTION(BlueprintCallable)
	void SetFallenInDungeon(bool Value) { bFallenInDungeon = Value; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsFallenInDungeon() { return bFallenInDungeon; }

	bool IsInAir();
	bool IsDead() { return MovementStatus == EMovementStatus::EMS_Dead ? true : false; }

	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void Jump() override;

	// Called for forwards/backwards input
	void MoveForward(float Value);

	// Called for side to side input
	void MoveRight(float Value);

	// Called for shift input
	void Run(float Value);

	// Called via(means ~를 통해) input to turn at a given rate
	// @param(매개변수 설명) Rate This is a normalized rate, i.e.(means 다시 말해, 바로) 1.0 means 100% of desired turn rate
	void TurnAtRate(float Rate);

	// Called via(means ~를 통해) input to look up/down at a given rate
	// @param(매개변수 설명) Rate This is a normalized rate, i.e.(means 다시 말해, 바로) 1.0 means 100% of desired look up/down rate
	void LookUpAtRate(float Rate);

	void CameraZoom(const float Value);

	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	void LMBDown();
	void LMBUp();

	void ESCDown();
	void ESCUp();
	
    UFUNCTION()
    virtual void ItemSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
    UFUNCTION()
    virtual void ItemSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void CombatSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	void CombatSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;

	void PlayMontageWithItem();
 
	TArray<AEnemy*> GetTargets() { return Targets; }

	void SetAutoTargeting(bool value) { bAutoTargeting = value; }

	void Targeting();

	void Attack() override;
	void AttackEnd() override;

	void Spawn() override;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	void Die();

	UFUNCTION(BlueprintCallable)
	void DeathEnd();

	void Revive();

	UFUNCTION(BlueprintCallable)
	void RevivalEnd();

	void Escape(); // press E key, spawn player at the other location

	void ShowControlGuide();

	void StartDialogue();




	UFUNCTION(BlueprintCallable)
	void RecoverWithLogo();

	void UsePotion();

	void SkipCombat();
	void SetLevel5();


	UPROPERTY(BlueprintAssignable, BlueprintCallable) // 레벨 블루프린트에서 바인딩함
	FDele_Dynamic PlaneUp;

	UPROPERTY(BlueprintAssignable, BlueprintCallable) // 레벨 블루프린트에서 바인딩함
	FDele_Dynamic Ending;

	UPROPERTY(BlueprintAssignable, BlueprintCallable) // 레벨 블루프린트에서 바인딩함
	FDele_Dynamic SkipFirstDungeon;

	UPROPERTY(BlueprintAssignable, BlueprintCallable) // 레벨 블루프린트에서 바인딩함
	FDele_Dynamic SkipSecondDungeon;

	UPROPERTY(BlueprintAssignable, BlueprintCallable) // 레벨 블루프린트에서 바인딩함
	FDele_Dynamic SkipFinalDungeon;
};
