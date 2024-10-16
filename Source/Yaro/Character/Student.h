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

private:
	// 회전 속도
	float InterpSpeed;

	int SkillNum;

	class ACharacter* TargetCharacter; // Who the character looks at

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	bool bInterpToCharacter = false;

	UPROPERTY(EditAnywhere)
	bool bInterpToEnemy;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
	class UAnimMontage* NormalMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
	class UAnimMontage* CombatMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
		float WalkSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
		float RunSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	class USphereComponent* CombatSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bOverlappingCombatSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	class AEnemy* CombatTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	FVector CombatTargetLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bAttacking;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	class UArrowComponent* AttackArrow;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skills")
	UDataTable* AttackSkillData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skills")
	TWeakObjectPtr<class AMagicSkill> MagicAttack;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skills")
	TSubclassOf<class AMagicSkill> ToSpawn;



public:	
	// Sets default values for this character's properties
	AStudent();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void SetInterpSpeed(float value) { InterpSpeed = value; }
	float GetInterpSpeed() { return InterpSpeed; }

	FRotator GetLookAtRotationYaw(FVector Target);

	void SetSkillNum(int num) { SkillNum = num; }
	int GetSkillNum() { return SkillNum; }
	
	void SetInterpToEnemy(bool Interp) { bInterpToEnemy = Interp; };

	void SetInterpToCharacter(bool value) { bInterpToCharacter = value;};

	UAnimMontage* GetCombatMontage() { return CombatMontage; }
	UAnimMontage* GetNormalMontage() { return NormalMontage; }

	void SetTargetCharacter(class ACharacter* ch) { TargetCharacter = ch; }
	class ACharacter* GetTargetCharacter() { return TargetCharacter; }

	void SetCombatTarget(AEnemy* target) { CombatTarget = target; }
	AEnemy* GetCombatTarget() { return CombatTarget; }

	void SetAttackArrow();

	virtual void Attack() {};

	UFUNCTION(BlueprintCallable)
	virtual	void AttackEnd() {};

	UFUNCTION()
	virtual void CombatSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {};
	UFUNCTION()
	virtual void CombatSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {};

	// 마법 스폰
	virtual void Spawn() {};

};
