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

UENUM(BlueprintType)
enum class EEnemyType :uint8
{
	Goblin, Grux, Golem, LittleDino, Lizard, Archer, LizardShaman, Spider, LittleMonster, Boss
};

UCLASS()
class YARO_API AEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	AEnemy();
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	class USphereComponent* CreateSphereComponent(FName Name, float Radius);
	void BindComponentEvents();

protected:
	UPROPERTY()
	class AAIController* AIController;
	UPROPERTY()
	EEnemyType EnemyType;

	//Managers
	UPROPERTY()
	class UGameManager* GameManager;
	UPROPERTY()
	class UNPCManager* NPCManager;

	bool hasSecondCollision = false;

	EEnemyMovementStatus EnemyMovementStatus;

	//Stats
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Info")
	float Health;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Info")
	float MaxHealth;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Info")
	float Damage;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Info")
	float EnemyExp;

	// Enemy's back to their initial location
	FVector InitialLocation;
	FRotator InitialRotation;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	class USphereComponent* AgroSphere; // 인식 범위

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	USphereComponent* CombatSphere; // 공격 범위

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bOverlappingCombatSphere = false;

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
	float InterpSpeed = 10.f;
	bool bInterpToTarget = false;

	// 공격 관련
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	float AttackDelay; // 공격 텀

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bAttacking; // 공격 중 true

	UPROPERTY(VisibleAnywhere, Category = "Combat")
	bool bAttackFromPlayer = false; //플레이어로부터의 공격인지 판단

	class AMagicSkill* MagicAttack;

	// 죽음 관련
	float DeathDelay = 3.f; // 객체 소멸 텀


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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
		class AMain* Main; // 플레이어
	
public: // Getters and Setters
	EEnemyMovementStatus GetEnemyMovementStatus() { return EnemyMovementStatus; }
	void SetEnemyMovementStatus(EEnemyMovementStatus Status) { EnemyMovementStatus = Status; }

	void InitHealth(float value) { Health = value; MaxHealth = value;}

	void SetMain();

	void SetAgroSphere(float Radius);
	void SetCombatSphere(float Radius);

	void SetInterpToTarget(bool Interp) { bInterpToTarget = Interp; }

	FRotator GetLookAtRotationYaw(FVector Target);


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


	UFUNCTION(BlueprintCallable)
	void Die();

	UFUNCTION(BlueprintCallable)
	void DeathEnd();

	// 소멸
	virtual void Disappear();

	void DisableCombatCollisions();
	void DisableSphereCollisions();

	bool IsDead() { return EnemyMovementStatus != EEnemyMovementStatus::EMS_Dead;}

	// 공격받은 뒤
	UFUNCTION(BlueprintCallable)
	void HitEnd();
};
