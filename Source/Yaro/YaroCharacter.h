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

	FTimerHandle MoveTimer; // move to player

	void MoveToPlayer();


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items)
	class AItem* ActiveOverlappingItem;

	FORCEINLINE void SetActiveOverlappingItem(AItem* Item) { ActiveOverlappingItem = Item; }

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

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bOverlappingCombatSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	class AEnemy* CombatTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	FVector CombatTargetLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bHasCombatTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TArray<AEnemy*> Targets;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	int targetIndex;

	FTimerHandle AttackTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackDelay;

	void MoveToTarget(AEnemy* Target);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skills")
	class AMagicSkill* MagicAttack;

	UPROPERTY(EditAnywhere)
	TArray<FVector> Pos;

	UFUNCTION(BlueprintCallable)
	void MoveToLocation();

	int index = 0;

	bool canGo = false;

	FTimerHandle TeamMoveTimer;

protected:

	virtual void BeginPlay() override;

	

protected:
	// APawn interface
	//virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface
public:

	virtual void Tick(float DeltaTime) override;

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

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	class UArrowComponent* AttackArrow;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skills")
	TSubclassOf<class AMagicSkill> ToSpawn;

	int SkillNum;

	void Spawn();


};

