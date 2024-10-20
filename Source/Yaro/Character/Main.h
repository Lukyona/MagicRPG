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

UCLASS()
class YARO_API AMain : public AStudent
{
	GENERATED_BODY()

	UPROPERTY()
	class UGameManager* GameManager;

	UPROPERTY()
		class UDialogueManager* DialogueManager;

	UPROPERTY()
		class UUIManager* UIManager;

	UPROPERTY()
		class UNPCManager* NPCManager;



protected:


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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraZoom")
	float MinZoomLength = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraZoom")
	float MaxZoomLength = 800.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraZoom")
	float ZoomStep = 30.f;

	// Base turn rates to scale turning functions for the camera
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float BaseTurnRate;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
	float BaseLookUpRate;


	//플레이어 성별
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerInfo")
	int Gender;

	//달리는 상태인지 확인
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	bool bRunning;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanMove = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		bool bFallenInDungeon = false; // 플레이어가 던전 범위 밖으로 추락했을 때 true

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int FallCount = 0;

	/**
	/*
	/* Player Stats
	/*
	*/

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player Stats")
	float MaxHP;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player Stats")
	float HP;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Stats")
	float MaxMP;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player Stats")
	float MP;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Stats")
	float MaxSP;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player Stats")
	float SP;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player Stats")
	int Level;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player Stats")
	float Exp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player Stats")
	float MaxExp;

	// 스탯 자동 회복
	FTimerHandle HPTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float HPDelay;

	FTimerHandle MPTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float MPDelay;

	FTimerHandle SPTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
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




	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int PotionNum;

public:
	// Sets default values for this character's properties
	AMain();

	AMainPlayerController* GetMainPlayerController() { return MainPlayerController; }

	FORCEINLINE void SetMovementStatus(EMovementStatus Status) { MovementStatus = Status; }
	FORCEINLINE EMovementStatus GetMovementStatus() { return MovementStatus; }

	void SetCanMove(bool value) { bCanMove = value; }

	float GetHP() { return HP; }
	void SetHP(float value) { HP = value; }
	float GetMaxHP() { return HP; }
	float GetMP() { return HP; }
	
	void AddHP(float value) { HP += value; }
	void AddMP(float value) { MP += value; }

	void RecoveryHP();
	void RecoveryMP();
	void RecoverySP();
	
	FORCEINLINE void SetActiveOverlappingItem(AItem* Item) { ActiveOverlappingItem = Item; }

	void SetEquippedWeapon(AWeapon* wp) { EquippedWeapon = wp; }
	AWeapon* GetEquippedWeapon() { return EquippedWeapon; }

	void SetItemInHand(AItem* item) { ItemInHand = item; }
	AItem* GetItemInHand() { return ItemInHand; }

	void SetFallCount(uint8 Count) { FallCount = Count; }
	int GetFallCount() { return FallCount; }

	void SetFallenInDungeon(bool Value) { bFallenInDungeon = Value; }
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

	void GetExp(float exp);

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

	void ShowManual();

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
