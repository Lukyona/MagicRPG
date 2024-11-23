// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Engine/DataTable.h"
#include "Student.generated.h"

UCLASS()
class YARO_API AStudent : public ACharacter
{
	GENERATED_BODY()

public:
	AStudent();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	//Movements
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float WalkSpeed;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float RunSpeed;

	float InterpSpeed;

	UPROPERTY(EditAnywhere)
	bool bInterpToCharacter = false;
	UPROPERTY(EditAnywhere)
	bool bInterpToEnemy;


	//Combat
	int SkillNum;
	class ACharacter* TargetCharacter; // Who the character looks at

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	class USphereComponent* CombatSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bOverlappingCombatSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	class AEnemy* CombatTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	FVector CombatTargetLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	class UArrowComponent* AttackArrow;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bAttacking;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skills")
	UDataTable* AttackSkillData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skills")
	TWeakObjectPtr<class AMagicSkill> MagicAttack;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skills")
	TSubclassOf<class AMagicSkill> ToSpawn;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
		class UAnimMontage* NormalMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
		class UAnimMontage* CombatMontage;

public: //Getters and Setters
	float GetInterpSpeed() { return InterpSpeed; }
	void SetInterpSpeed(float value) { InterpSpeed = value; }

	FRotator GetLookAtRotationYaw(FVector Target);

	void SetInterpToEnemy(bool Interp) { bInterpToEnemy = Interp; };
	void SetInterpToCharacter(bool value) { bInterpToCharacter = value;};


	UAnimMontage* GetCombatMontage() { return CombatMontage; }
	UAnimMontage* GetNormalMontage() { return NormalMontage; }


	int GetSkillNum() { return SkillNum; }
	void SetSkillNum(int num) { SkillNum = num; }

	class ACharacter* GetTargetCharacter() { return TargetCharacter; }
	void SetTargetCharacter(class ACharacter* ch) { TargetCharacter = ch; }

	AEnemy* GetCombatTarget() { return CombatTarget; }
	void SetCombatTarget(AEnemy* target) { CombatTarget = target; }

	void SetAttackArrow();

	virtual void Attack() {};

	UFUNCTION(BlueprintCallable)
	virtual	void AttackEnd() {};

	// 마법 스폰
	virtual void Spawn() {};

	UFUNCTION()
		virtual void CombatSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {};
	UFUNCTION()
		virtual void CombatSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {};

};
