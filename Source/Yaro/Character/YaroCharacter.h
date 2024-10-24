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



	UPROPERTY()
	class UDialogueManager* DialogueManager;

	UPROPERTY()
	class UNPCManager* NPCManager;

protected:
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UGameManager* GameManager;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	class AAIController* AIController;

	UAnimInstance* AnimInstance;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	ENPCType NPCType;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	class AMain* Player;

	UPROPERTY(BlueprintReadWrite)
	FTimerHandle MoveTimer; // move to player


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	class USphereComponent* AgroSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TArray<AEnemy*> AgroTargets;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TArray<AEnemy*> CombatTargets;

	FTimerHandle AttackTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackDelay;

	UPROPERTY(BlueprintReadWrite)
	FTimerHandle MagicSpawnTimer;

	bool bCanCastStrom = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bIsHealTime;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	class USphereComponent* NotAllowSphere;

	FTimerHandle SafeDistanceTimer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	TArray<AEnemy*> DangerousTargets;



	// 비비, 지지, 보보 팀 이동 위치
	UPROPERTY(EditAnywhere)
	TArray<FVector> Pos;

	UPROPERTY(VisibleAnywhere)
	int index = 0;

	UPROPERTY(BlueprintReadWrite)
	FTimerHandle TeamMoveTimer; // vovo, vivi, zizi



	FTimerHandle TeleportTimer;

	int TeleportCount = 0;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AActor> Boss;

	UPROPERTY(BlueprintReadWrite)
	bool bSmile;

	UPROPERTY(BlueprintReadWrite)
	bool bSpeaking;


	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	AYaroCharacter();

	AAIController* GetAIController() { return AIController; }

	ENPCType GetType() { return NPCType; }

	UFUNCTION(BlueprintCallable)
	void MoveToPlayer();

	void SetSpeakingStatus(bool value) { bSpeaking = value; }

	UAnimInstance* GetAnimInstance() { return AnimInstance; }

	void SetPosList();

	//About Combat System
	void Attack() override;
	void AttackEnd() override;


	void SetAgroSphere();
	void SetCombatSphere();
	void SetNotAllowSphere();
    
    UFUNCTION()
    virtual void AgroSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
    UFUNCTION()
    virtual void AgroSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void CombatSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	void CombatSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;

	void Spawn() override;

	void CanCastStormMagic();

	void MoveToTarget(AEnemy* Target);


	UFUNCTION(BlueprintCallable)
	void MoveToLocation();
  
	void Teleport();

	UFUNCTION()
	virtual void NotAllowSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	virtual void NotAllowSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void MoveToSafeLocation();

	
	int GetTeleportCount() { return TeleportCount; }

	FTimerHandle& GetMoveTimer() { return MoveTimer; }

	TArray<AEnemy*> GetAgroTargets() { return AgroTargets; }

	void ClearTeamMoveTimer();

	UFUNCTION(BlueprintCallable)
	void ClearAllTimer();

	void SetIndex(int value) { index = value; }
	int GetIndex() { return index; }

	TSubclassOf<class AActor> GetBoss() { return Boss; }
	
	void Smile();
	void UsualFace();
};

