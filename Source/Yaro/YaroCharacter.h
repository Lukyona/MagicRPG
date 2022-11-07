// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "YaroCharacter.generated.h"


UCLASS(config=Game)
class AYaroCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AYaroCharacter();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	class AAIController* AIController;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	class AMain* Player;

    UPROPERTY(BlueprintReadWrite)
	FTimerHandle MoveTimer; // move to player

	UFUNCTION(BlueprintCallable)
	void MoveToPlayer();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class UBoxComponent* Wand;

	// When character attck enemy, character look at enemy
	float InterpSpeed;
	bool bInterpToEnemy;
	void SetInterpToEnemy(bool Interp);

	FRotator GetLookAtRotationYaw(FVector Target);

	//About Combat System
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	class USphereComponent* CombatSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    class USphereComponent* AttackSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bOverlappingAttackSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	class AEnemy* CombatTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	FVector CombatTargetLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TArray<AEnemy*> AgroTargets;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TArray<AEnemy*> CombatTargets;

	FTimerHandle AttackTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackDelay;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
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

    UFUNCTION()
    virtual void AttackSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
    UFUNCTION()
    virtual void AttackSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);


    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
    class UArrowComponent* AttackArrow;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skills")
    TSubclassOf<class AMagicSkill> ToSpawn;

    int SkillNum;

    void Spawn();

    UPROPERTY(BlueprintReadWrite)
    FTimerHandle MagicSpawnTimer;


	bool bCanCastStrom = true;

	void CanCastStormMagic();

	void MoveToTarget(AEnemy* Target);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skills")
	class AMagicSkill* MagicAttack;

	UPROPERTY(EditAnywhere)
	TArray<FVector> Pos;

	UFUNCTION(BlueprintCallable)
	void MoveToLocation();

    UPROPERTY(VisibleAnywhere)
	int index = 0;

	bool canGo = false;

    UPROPERTY(BlueprintReadWrite)
	FTimerHandle TeamMoveTimer; // vivi, vivi, zizi
	
    UPROPERTY(EditAnywhere)
	bool bInterpToCharacter = false;

    class ACharacter* TargetCharacter; // Who the npc look at

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
    class UAnimMontage* NormalMontage;

    UAnimInstance* AnimInstance;

	void Teleport();

	FTimerHandle TeleportTimer;

	int TeleportCount = 0;

protected:

	virtual void BeginPlay() override;


protected:
	// APawn interface
	//virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface
public:

	virtual void Tick(float DeltaTime) override;




};

