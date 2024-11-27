// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Student.h"
#include "Yaro/Character/Enemies/Enemy.h"
#include "YaroCharacter.generated.h"

class AAIController;
class UGameManager;
class UDialogueManager;
class UNPCManager;
class AMain;
class USphereComponent;

UENUM(BlueprintType)
enum class ENPCType : uint8
{
	Momo,
	Luko,
	Vovo,
	Vivi,
	Zizi
};

UCLASS(config=Game)
class AYaroCharacter : public AStudent
{
	GENERATED_BODY()

protected:
	AYaroCharacter();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	void CombatSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	void CombatSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;
	void Attack() override;
	void AttackEnd() override;
	void Spawn() override;

	USphereComponent* CreateSphereComponent(FName Name, float Radius, FVector RelativeLocation);
	void BindComponentEvents();

protected:
	UPROPERTY()
	AAIController* AIController;
	UPROPERTY()
	UAnimInstance* AnimInstance;
	UPROPERTY(EditDefaultsOnly)
	ENPCType NPCType;

	//Managers
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UGameManager* GameManager;
	UPROPERTY()
	UDialogueManager* DialogueManager;
	UPROPERTY()
	UNPCManager* NPCManager;


	UPROPERTY()
	AMain* Player;

	UPROPERTY(BlueprintReadOnly)
	FTimerHandle PlayerFollowTimer;


	//Combat
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	USphereComponent* AgroSphere;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TArray<AEnemy*> AgroTargets;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TArray<AEnemy*> CombatTargets;

	FTimerHandle AttackTimer;
	UPROPERTY(EditAnywhere, Category = "Combat")
	float AttackDelay;

	FTimerHandle MagicSpawnTimer;
	bool bCanCastStorm = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bIsHealTime;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	USphereComponent* NotAllowSphere;

	FTimerHandle SafeDistanceTimer;

	UPROPERTY()
	TArray<AEnemy*> DangerousTargets;


	FVector LastPosition;
	int32 MoveFailCounter = 0;

	UPROPERTY(BlueprintReadWrite)
	bool bIsSmiling;
	UPROPERTY(BlueprintReadWrite)
	bool bIsSpeaking;


	// 비비, 지지, 보보 팀 이동 위치
	TArray<FVector> TeamMovePosList =
	{ 
		FVector(2517.f, 5585.f, 3351.f),
		FVector(2345.f, 4223.f, 2833.f),
		FVector(2080.f, 283.f, 2838.f),
		FVector(1550.f, -1761.f, 2843.f),
		FVector(1026.f, -1791.f, 2576.f)
	};
	int32 TeamMoveIndex = 0;
	FTimerHandle TeamMoveTimer;

public: //Getters and Setters
	AAIController* GetAIController() 
	{
		return AIController; 
	}
	UAnimInstance* GetAnimInstance() 
	{
		return AnimInstance; 
	}
	ENPCType GetType() 
	{
		return NPCType; 
	}

	int32 GetTeamMoveIndex() 
	{
		return TeamMoveIndex; 
	}
	void SetTeamMoveIndex(int32 value) 
	{
		TeamMoveIndex = value; 
	}

	void SetSmileStatus(bool Value) 
	{
		bIsSmiling = Value; 
	}
	void SetSpeakingStatus(bool Value) 
	{
		bIsSpeaking = Value; 
	}

	void SetMovementSpeed(bool bEnableRunning);

	TArray<AEnemy*> GetAgroTargets() 
	{
		return AgroTargets; 
	}

	void EnableStormCasting() 
	{
		bCanCastStorm = true; 
	}

public: // Core Methods
	//Movements
	void MoveTo(ACharacter* GoalActor, float AcceptanceRadius);

	UFUNCTION(BlueprintCallable)
	void MoveToPlayer();

	void MoveToTarget(AEnemy* Target);

	void MoveToTeamPos();

	void MoveToSafeLocation();

	void TeleportToPlayer();

	void CheckGolem();

	// Clear Timers
	void ClearTeamMoveTimer();

	UFUNCTION(BlueprintCallable)
	void ClearPlayerFollowTimer();

	UFUNCTION(BlueprintCallable)
	void ClearAllTimer();
    
    UFUNCTION()
    virtual void AgroSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
    UFUNCTION()
    virtual void AgroSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	virtual void NotAllowSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	virtual void NotAllowSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);	
};
