// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Main.generated.h"


UENUM(BlueprintType)
enum class EMovementStatus :uint8
{
	EMS_Normal			UMETA(DeplayName = "Normal"),
	EMS_Dead			UMETA(DeplayName = "Dead"),

	EMS_MAX				UMETA(DeplayName = "DefaultMAX")
};


UCLASS()
class YARO_API AMain : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMain();

	UPROPERTY(EditDefaultsOnly, Category = "SavedData")
	TSubclassOf<class AItemStorage> ObjectStorage;

	class AItemStorage* Storage;

	// CameraBoom positioning the camera behind the player, 카메라붐은 플레이어 뒤에 카메라를 위치시킴
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	// Base turn rates to scale turning functions for the camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseTurnRate;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseLookUpRate;

	//플레이어 성별
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerInfo")
	int Gender;

	//달리는 상태인지 확인
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	bool bRunning;

	/**
	/*
	/* Player Stats
	/*
	*/
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category  = "Player Stats")
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

	// When player attck enemy, player look at enemy
	float InterpSpeed;
	bool bInterpToEnemy;
	void SetInterpToEnemy(bool Interp);

	FRotator GetLookAtRotationYaw(FVector Target);

	//About Combat System
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	class USphereComponent* CombatSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bOverlappingCombatSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	class AEnemy* CombatTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	FVector CombatTargetLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TArray<AEnemy*> Targets;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bHasCombatTarget;

	int targetIndex;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Controller")
	class AMainPlayerController* MainPlayerController;


	FTimerHandle DeathTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float DeathDelay;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	EMovementStatus MovementStatus;

	FORCEINLINE void SetMovementStatus(EMovementStatus Status) { MovementStatus = Status; }
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool NpcGo = false;


	FTimerHandle HPTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float HPDelay;

	void RecoveryHP();

	FTimerHandle MPTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float MPDelay;

	void RecoveryMP();

	FTimerHandle SPTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float SPDelay;

	bool recoverySP = false;

	void RecoverySP();

	UPROPERTY(VisibleAnyWhere)
	int CurrentEnemyNum = 0;

	UPROPERTY(VisibleAnyWhere)
	TArray<AEnemy*> Enemies;


	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Interaction System")
	class UCapsuleComponent* InteractionRange;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Interaction System")
	class ACharacter* InteractTarget;


	UPROPERTY(VisibleAnywhere)
	class AYaroCharacter* test;

    UPROPERTY(VisibleAnywhere)
    TArray<AYaroCharacter*> NPCList;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// About CameraZoom
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraZoom")
	float MinZoomLength = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraZoom")
	float MaxZoomLength = 800.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraZoom")
	float ZoomStep = 30.f;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

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

	bool bLMBDown;
	void LMBDown();
	void LMBUp();


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items)
	class AItem* ActiveOverlappingItem;

	FORCEINLINE void SetActiveOverlappingItem(AItem* Item) { ActiveOverlappingItem = Item; }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items)
	class AWeapon* EquippedWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anims")
	bool bAttacking;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
	class UAnimMontage* CombatMontage;

	void Attack();

	UFUNCTION(BlueprintCallable)
	void AttackEnd();

	UFUNCTION()
	virtual void CombatSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	virtual void CombatSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void Targeting();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	class UArrowComponent* AttackArrow;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skills")
	TSubclassOf<class AMagicSkill> ToSpawn;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skills")
	class AMagicSkill* MagicAttack;

	int SkillNum;

	void Spawn();

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	void Die();

	UFUNCTION(BlueprintCallable)
	void DeathEnd();

	virtual void Jump() override;

	void Revive();

	UFUNCTION(BlueprintCallable)
	void RevivalEnd();

	UFUNCTION(BlueprintCallable)
	void SaveGame();

	UFUNCTION(BlueprintCallable)
	void LoadGame();

	bool bESCDown;
	void ESCDown();
	void ESCUp();

	UFUNCTION()
	virtual void InteractionRangeOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	virtual void InteractionRangeOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void StartDialogue();


};
