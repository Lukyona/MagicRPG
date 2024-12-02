// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MagicSkill.generated.h"

UENUM(BlueprintType)
enum class EMagicSkillType : uint8
{
	Basic,
	Tornado,
	Laser,
	Healing,
	Hit,
	Storm
};

UENUM(BlueprintType)
enum class ECasterType : uint8
{
	Player,
	NPC,
	Enemy,
	Boss,
};

class USphereComponent;
class UParticleSystemComponent;
class UParticleSystem;
class UProjectileMovementComponent;
class USoundBase;
class ACharacter;
class AMain;

UCLASS()
class YARO_API AMagicSkill : public AActor
{
	GENERATED_BODY()
public:
	AMagicSkill();
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	virtual void OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USphereComponent* Sphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UParticleSystemComponent* Particle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ExplosionFX")
	UParticleSystem* ParticleFX;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UProjectileMovementComponent* MovementComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds")
	USoundBase* ExplosionSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds")
	USoundBase* MagicSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TSubclassOf<UDamageType> DamageTypeClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	AController* MagicInstigator;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Combat")
	ACharacter* Target;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	ECasterType Caster; // who cast this spell(magic)

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	EMagicSkillType SkillType;

	FTimerHandle DestroyTimerHandle, LocationTimerHandle;
	// To move Target
	FVector Direction, StartLocation;
	float TotalDistance, CurrentDistance;

	AMain* Main;

public: //Getters and setters
	AController* GetInstigator() 
	{
		return MagicInstigator; 
	}
	ECasterType GetCaster() 
	{
		return Caster; 
	}

	void SetLocation(); 
	void SetTarget(ACharacter* TargetCharacter) 
	{
		Target = TargetCharacter; 
	}
	void SetInstigator(AController* Inst) 
	{
		MagicInstigator = Inst; 
	}
	void SetMain();

private:
	void StartDestroyTimer(float Delay);
	void StartLocationUpdateTimer(float Delay);

	void PlaySound(USoundBase* Sound);

	void HandleStudentCasterOverlap(AActor* OtherActor);
	void HandleEnemyCasterOverlap(AActor* OtherActor);
};
