// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Enemy.generated.h"


UENUM(BlueprintType)
enum class EEnemyMovementStatus :uint8
{
	EMS_Idle			UMETA(DeplayName = "Idle"),
	EMS_MoveToTarget	UMETA(DeplayName = "MoveToTarget"),
	EMS_Attacking		UMETA(DeplayName = "Attacking"),
	EMS_Dead			UMETA(DeplayName = "Dead"),
};

UCLASS()
class YARO_API AEnemy : public ACharacter
{
	GENERATED_BODY()

private:
	UPROPERTY()
		class UGameManager* GameManager;

	UPROPERTY()
		class UNPCManager* NPCManager;

	bool hasSecondCollision = false;

protected:
	UPROPERTY(VisibleAnywhere, Category = "AI")
	class AAIController* AIController;

	UPROPERTY(VisibleAnywhere, Category = "Info")
	EEnemyMovementStatus EnemyMovementStatus;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Info")
	float Health;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Info")
	float MaxHealth;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info")
	float Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info")
	float EnemyExp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info")
	FString Name;

	// Enemy's back to their initial location
	FVector InitialLocation;
	FRotator InitialRotation;
	
protected:
	/*
	Combat System
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	class AMain* Main; // 플레이어

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	class USphereComponent* AgroSphere; // 인식 범위

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	USphereComponent* CombatSphere; // 공격 범위

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bOverlappingCombatSphere;

	// 인식 중인 타겟과 공격 타겟
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	AStudent* AgroTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	AStudent* CombatTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TArray<AStudent*> AgroTargets;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TArray<AStudent*> CombatTargets;


	// When enemy attck target, enemy look at target
	float InterpSpeed;
	bool bInterpToTarget;


	// 공격 관련
	FTimerHandle AttackTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackDelay; // 공격 텀

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bAttacking; // 공격 중 true

	UPROPERTY(VisibleAnywhere, Category = "Combat")
	bool bAttackFromPlayer = false; //플레이어로부터의 공격인지 판단

	class AMagicSkill* MagicAttack;

	// 죽음 관련
	FTimerHandle DeathTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float DeathDelay; // 객체 소멸 텀


	// 초기 위치 복귀 시 사용
	FTimerHandle CheckHandle;
	int Count = 0;

	// 우선순위 유저->npc로 변경하는 MovingNow()에서 사용
	int MovingCount = 0;
	FTimerHandle MovingTimer;


	// 애니메이션
	class UAnimInstance* AnimInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	class UAnimMontage* CombatMontage;

	// 효과음
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds")
	class USoundCue* AgroSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds")
	class USoundCue* DeathSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds")
	class USoundCue* SkillSound;

	// 무기 콜리전
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	class UBoxComponent* CombatCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	class UBoxComponent* CombatCollision2;
	
public: // Get, Set
	// Sets default values for this character's properties
	AEnemy();

	AAIController* GetAIController() { return AIController; }

	// Status
	FORCEINLINE void SetEnemyMovementStatus(EEnemyMovementStatus Status) { EnemyMovementStatus = Status; }
	FORCEINLINE EEnemyMovementStatus GetEnemyMovementStatus() { return EnemyMovementStatus; }

	void InitHealth(float value) { Health = value; MaxHealth = value;}
	//float GetMaxHealth() { return MaxHealth; }

	void SetMain();

	void SetAgroSphere(float value);
	void SetCombatSphere(float value);

	// 타겟을 향해 회전
	void SetInterpToTarget(bool Interp);

	FRotator GetLookAtRotationYaw(FVector Target);


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Info")
	TSubclassOf<UDamageType> DamageTypeClass;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


	// Combat
	UFUNCTION()
	virtual void AgroSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	virtual void AgroSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	virtual void CombatSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	virtual void CombatSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable)
	void MoveToTarget(AStudent* Target);

	// 우선순위 유저->npc
	void MovingNow();


	// 초기 위치로 이동
	void MoveToLocation();
	void CheckLocation();

	// 무기 콜리전 생성
	void CreateFirstWeaponCollision();
	void CreateSecondWeaponCollision();

	void EnableFirstWeaponCollision();
	void EnableSecondWeaponCollision();

	// 무기 콜리전 오버랩
	UFUNCTION()
	void CombatOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void CombatOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// 무기 콜리전 활성화, 비활성화
	UFUNCTION(BlueprintCallable)
	void ActivateCollision(); 

	UFUNCTION(BlueprintCallable)
	void DeactivateCollision();

	UFUNCTION(BlueprintCallable)
	void ActivateCollisions();

	UFUNCTION(BlueprintCallable)
	void DeactivateCollisions();


	// 공격
	UFUNCTION(BlueprintCallable)
	void Attack();

	UFUNCTION(BlueprintCallable)
	void AttackEnd();

	// 공격 받음
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable)
	void Die();

	UFUNCTION(BlueprintCallable)
	void DeathEnd();

	// 소멸
	virtual void Disappear();

	void CombatCollisionDisabled();
	void SphereCollisionDisabled();

    UFUNCTION(BlueprintCallable)
	bool Alive();

	// 공격받은 뒤
	UFUNCTION(BlueprintCallable)
	void HitEnd();
};
