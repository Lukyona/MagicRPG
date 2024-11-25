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
	void BeginPlay() override;
	void Tick(float DeltaTime) override;
	float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
private:
	class USphereComponent* CreateSphereComponent(FName Name, float Radius);
	void SetAgroSphere(float Radius);
	void SetCombatSphere(float Radius);
	void BindSphereComponentEvents();

	// ���� �ݸ���
	class UBoxComponent* CreateCollision(FName Name, FName SocketName);
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


	void SetMain();

	FRotator GetLookAtRotationYaw(FVector Target);

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

	bool hasSecondCollision = false;

	float AgroSphereRadius = 0.f;
	float CombatSphereRadius = 0.f;

	// Enemy's back to their initial location
	FVector InitialLocation;
	FRotator InitialRotation;

	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	class USphereComponent* AgroSphere; // �ν� ����

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	USphereComponent* CombatSphere; // ���� ����

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bOverlappingCombatSphere = false;

	// �ν� ���� Ÿ�ٰ� ���� Ÿ��
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

	// ���� ����
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	float AttackDelay; // ���� ��

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bAttacking; // ���� �� true

	UPROPERTY(VisibleAnywhere, Category = "Combat")
	bool bAttackFromPlayer = false; //�÷��̾�κ����� �������� �Ǵ�

	class AMagicSkill* MagicAttack;

	// ���� ����
	float DeathDelay = 3.f; // ��ü �Ҹ� ��


	// �ʱ� ��ġ ���� �� ���
	FTimerHandle CheckHandle;
	int Count = 0;

	// �켱���� ����->npc�� �����ϴ� MovingNow()���� ���
	int MovingCount = 0;
	FTimerHandle MovingTimer;


	// �ִϸ��̼�
	class UAnimInstance* AnimInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	class UAnimMontage* CombatMontage;

	// ȿ����
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds")
	class USoundCue* AgroSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds")
	class USoundCue* DeathSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds")
	class USoundCue* SkillSound;

	// ���� �ݸ���
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	class UBoxComponent* CombatCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	class UBoxComponent* CombatCollision2;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
		class AMain* Main; // �÷��̾�
	
public: // Getters and Setters
	EEnemyMovementStatus GetEnemyMovementStatus() { return EnemyMovementStatus; }
	void SetEnemyMovementStatus(EEnemyMovementStatus Status) { EnemyMovementStatus = Status; }

	void SetInterpToTarget(bool Interp) { bInterpToTarget = Interp; }



	UFUNCTION(BlueprintCallable)
	void MoveToTarget(AStudent* Target);

	// �켱���� ����->npc
	void MovingNow();


	// �ʱ� ��ġ�� �̵�
	void MoveToLocation();
	void CheckLocation();



	
	UFUNCTION(BlueprintCallable)
	void ActivateCollision(); 
	UFUNCTION(BlueprintCallable)
	void DeactivateCollision();


	// ����
	UFUNCTION(BlueprintCallable)
	void Attack();
	UFUNCTION(BlueprintCallable)
	void AttackEnd();


	UFUNCTION(BlueprintCallable)
	void Die();
	UFUNCTION(BlueprintCallable)
	void DeathEnd();

	// �Ҹ�
	virtual void Disappear();

	void DisableWeaponCollisions();
	void DisableSphereCollisions();

	bool IsDead() { return EnemyMovementStatus != EEnemyMovementStatus::EMS_Dead;}

	// ���ݹ��� ��
	UFUNCTION(BlueprintCallable)
	void HitEnd();
};
