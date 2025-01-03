// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Engine/DataTable.h"
#include "Student.generated.h"

class ACharacter;
class USphereComponent;
class AEnemy;
class UArrowComponent;
class AMagicSkill;
class UAnimMontage;

UCLASS()
class YARO_API AStudent : public ACharacter
{
	GENERATED_BODY()

public:
	AStudent();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION()
	virtual void CombatSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {};
	UFUNCTION()
	virtual void CombatSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {};

protected:
	//Movements
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float WalkSpeed;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float RunSpeed;

	float InterpSpeed = 15.f;

	UPROPERTY(EditAnywhere)
	bool bInterpToActor = false;
	UPROPERTY(EditAnywhere)
	bool bInterpToEnemy;


	//Combat
	int32 SkillNum;
	AActor* TargetActor; // what or who the character looks at

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	USphereComponent* CombatSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bOverlappingCombatSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	AEnemy* CombatTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	FVector CombatTargetLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	UArrowComponent* AttackArrow;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bAttacking;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skills")
	UDataTable* AttackSkillData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skills")
	TWeakObjectPtr<AMagicSkill> MagicAttack;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skills")
	TSubclassOf<AMagicSkill> ToSpawn;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
	UAnimMontage* NormalMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
	UAnimMontage* CombatMontage;

public: //Getters and Setters
	FRotator GetLookAtRotationYaw(FVector Target);

	void SetInterpToEnemy(bool Interp) 
	{
		bInterpToEnemy = Interp; 
	}

	UAnimMontage* GetCombatMontage() 
	{
		return CombatMontage; 
	}
	UAnimMontage* GetNormalMontage() 
	{
		return NormalMontage; 
	}


	int GetSkillNum() 
	{
		return SkillNum; 
	}
	void SetSkillNum(int num) 
	{
		SkillNum = num; 
	}

	UFUNCTION(BlueprintCallable)
	void SetTargetActor(AActor* Actor) 
	{
		TargetActor = Actor;
		bInterpToActor = Actor != nullptr? true : false;
	}

	AEnemy* GetCombatTarget() 
	{
		return CombatTarget; 
	}
	void SetCombatTarget(AEnemy* target) 
	{
		CombatTarget = target; 
	}

	void SetAttackArrow();

	virtual void Attack() {};

	UFUNCTION(BlueprintCallable)
	virtual	void AttackEnd() {};

	// ���� ����
	virtual void Spawn() {};

};
