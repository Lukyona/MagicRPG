// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Enemy.generated.h"

class USphereComponent;
class UBoxComponent;
class AAIController;
class UGameManager;
class UNPCManager;
class AMagicSkill;
class UAnimInstance;
class UAnimMontage;
class USoundCue;
class AMain;

UENUM(BlueprintType)
enum class EEnemyMovementStatus :uint8
{
	EMS_Idle			UMETA(DisplayName = "Idle"),
	EMS_MoveToTarget	UMETA(DisplayName = "MoveToTarget"),
	EMS_Attacking		UMETA(DisplayName = "Attacking"),
	EMS_Dead			UMETA(DisplayName = "Dead"),
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
	void BeginPlay() override;
	void Tick(float DeltaTime) override;
	float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
private:
	USphereComponent* CreateSphereComponent(FName Name, float Radius);
	void BindSphereComponentEvents();

	// ���� �ݸ���
	UBoxComponent* CreateCollision(FName Name, FName SocketName);
	void CreateWeaponCollisions();
	void BindWeaponCollisionEvents();

	//������ �̺�Ʈ
	UFUNCTION()
		virtual void AgroSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
		virtual void AgroSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
		virtual void CombatSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
		virtual void CombatSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
		void WeaponCollisionOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
		void WeaponCollisionOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

protected:
	void CreateSpheresAndCollisions();

protected:
	UPROPERTY()
	AAIController* AIController;
	UPROPERTY()
	EEnemyType EnemyType;

	//Managers
	UPROPERTY()
	UGameManager* GameManager;
	UPROPERTY()
	UNPCManager* NPCManager;

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

	bool bHasSecondCollision = false; // �ٰŸ� ���� �ݸ����� 2���� ��
	bool bIsRangedAttacker = false; // ���Ÿ� ���ݸ��� ����ϴ� ��

	float AgroSphereRadius = 0.f;
	float CombatSphereRadius = 0.f;

	// Enemy's back to their initial location
	FVector InitialLocation;
	FRotator InitialRotation;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	USphereComponent* AgroSphere; // �ν� ����
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	USphereComponent* CombatSphere; // ���� ����

	// �ν� ���� Ÿ�ٰ� ���� Ÿ��
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	AStudent* AgroTarget;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TArray<AStudent*> AgroTargets;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	AStudent* CombatTarget;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TArray<AStudent*> CombatTargets;

	// When enemy attck target, enemy look at target
	float InterpSpeed = 10.f;
	bool bInterpToTarget = false;

	// ���� ����
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	float AttackDelay; // ���� ��

	UPROPERTY(VisibleAnywhere, Category = "Combat")
	bool bAttackFromPlayer = false; //�÷��̾�κ����� �������� �Ǵ�

	AMagicSkill* MagicAttack;

	// ���� ����
	float DeathDelay = 3.f; // ��ü �Ҹ� ��

	// �ʱ� ��ġ ���� �� ���
	FTimerHandle CheckLocationTimer;
	int32 ReturnCounter = 0;

	// Ÿ�� ���� ���� Ȯ���� �� ���
	FTimerHandle CheckChaseStateTimer;
	int32 MoveFailCounter = 0;


	// �ִϸ��̼�
	UAnimInstance* AnimInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	UAnimMontage* CombatMontage;

	// ȿ����
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds")
	USoundCue* AgroSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds")
	USoundCue* DeathSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds")
	USoundCue* SkillSound;

	// ���� �ݸ���
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	UBoxComponent* CombatCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	UBoxComponent* CombatCollision2;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	AMain* Main; // �÷��̾�
	
public: // Getters and Setters
	EEnemyMovementStatus GetEnemyMovementStatus()
	{
		return EnemyMovementStatus;
	}

	void SetMain();

	FRotator GetLookAtRotationYaw(FVector Target);

	bool IsValidTarget(AActor* Target);

	// �ʱ� ��ġ�� �̵�
	void MoveToInitialLocation();
	void CheckCurrentLocation();


	//Components
	UFUNCTION(BlueprintCallable)
	void ActivateWeaponCollisions(); 
	UFUNCTION(BlueprintCallable)
	void DeactivateWeaponCollisions();

	void DisableWeaponCollisions();
	void DisableSphereCollisions();


	// ����
	UFUNCTION(BlueprintCallable)
	void Attack();
	UFUNCTION(BlueprintCallable)
	void AttackEnd();

	UFUNCTION(BlueprintCallable)
	void HitEnd();

	UFUNCTION(BlueprintCallable)
	void Die();
	UFUNCTION(BlueprintCallable)
	void DeathEnd();

	virtual void Disappear();

	UFUNCTION(BlueprintCallable)
	void MoveToTarget(AStudent* Target);

	// Ÿ�� ���� ���� Ȯ�� �� Ÿ�� ����
	void CheckChaseState();

	bool IsDead()
	{ 
		return EnemyMovementStatus == EEnemyMovementStatus::EMS_Dead;
	}
};
