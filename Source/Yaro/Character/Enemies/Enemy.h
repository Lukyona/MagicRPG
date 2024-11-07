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
	class AMain* Main; // �÷��̾�

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	class USphereComponent* AgroSphere; // �ν� ����

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	USphereComponent* CombatSphere; // ���� ����

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bOverlappingCombatSphere;

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
	float InterpSpeed;
	bool bInterpToTarget;


	// ���� ����
	FTimerHandle AttackTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackDelay; // ���� ��

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bAttacking; // ���� �� true

	UPROPERTY(VisibleAnywhere, Category = "Combat")
	bool bAttackFromPlayer = false; //�÷��̾�κ����� �������� �Ǵ�

	class AMagicSkill* MagicAttack;

	// ���� ����
	FTimerHandle DeathTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float DeathDelay; // ��ü �Ҹ� ��


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

	// Ÿ���� ���� ȸ��
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

	// �켱���� ����->npc
	void MovingNow();


	// �ʱ� ��ġ�� �̵�
	void MoveToLocation();
	void CheckLocation();

	// ���� �ݸ��� ����
	void CreateFirstWeaponCollision();
	void CreateSecondWeaponCollision();

	void EnableFirstWeaponCollision();
	void EnableSecondWeaponCollision();

	// ���� �ݸ��� ������
	UFUNCTION()
	void CombatOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void CombatOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// ���� �ݸ��� Ȱ��ȭ, ��Ȱ��ȭ
	UFUNCTION(BlueprintCallable)
	void ActivateCollision(); 

	UFUNCTION(BlueprintCallable)
	void DeactivateCollision();

	UFUNCTION(BlueprintCallable)
	void ActivateCollisions();

	UFUNCTION(BlueprintCallable)
	void DeactivateCollisions();


	// ����
	UFUNCTION(BlueprintCallable)
	void Attack();

	UFUNCTION(BlueprintCallable)
	void AttackEnd();

	// ���� ����
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable)
	void Die();

	UFUNCTION(BlueprintCallable)
	void DeathEnd();

	// �Ҹ�
	virtual void Disappear();

	void CombatCollisionDisabled();
	void SphereCollisionDisabled();

    UFUNCTION(BlueprintCallable)
	bool Alive();

	// ���ݹ��� ��
	UFUNCTION(BlueprintCallable)
	void HitEnd();
};
