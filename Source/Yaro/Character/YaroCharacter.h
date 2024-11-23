// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Student.h"
#include "Yaro/Character/Enemies/Enemy.h"
#include "YaroCharacter.generated.h"

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
	class AAIController* AIController;
	UPROPERTY()
	UAnimInstance* AnimInstance;
	UPROPERTY(EditDefaultsOnly)
	ENPCType NPCType;

	//Managers
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UGameManager* GameManager;
	UPROPERTY()
	class UDialogueManager* DialogueManager;
	UPROPERTY()
	class UNPCManager* NPCManager;


	UPROPERTY()
	class AMain* Player;

	UPROPERTY(BlueprintReadOnly)
	FTimerHandle PlayerFollowTimer;


	//Combat
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	class USphereComponent* AgroSphere;
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
	class USphereComponent* NotAllowSphere;

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
	TArray<FVector> TeamMovePosList;
	int32 TeamMoveIndex = 0;
	FTimerHandle TeamMoveTimer;

public: //Getters and Setters
	AAIController* GetAIController() { return AIController; }
	UAnimInstance* GetAnimInstance() { return AnimInstance; }
	ENPCType GetType() { return NPCType; }

	int32 GetTeamMoveIndex() { return TeamMoveIndex; }
	void SetTeamMoveIndex(int32 value) { TeamMoveIndex = value; }

	void SetSmileStatus(bool Value) { bIsSmiling = Value; }
	void SetSpeakingStatus(bool Value) { bIsSpeaking = Value; }

	TArray<AEnemy*> GetAgroTargets() { return AgroTargets; }

	void SetTeamMovePosList();

	void EnableStormCasting() { bCanCastStorm = true; }

public: // Core Methods
	//Movements
	void MoveTo(ACharacter* GoalActor, float AcceptanceRadius);

	UFUNCTION(BlueprintCallable)
	void MoveToPlayer();

	void MoveToTarget(AEnemy* Target);

	void MoveToTeamPos();

	void MoveToSafeLocation();

	void TeleportToPlayer();


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

