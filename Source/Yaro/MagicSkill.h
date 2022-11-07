// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MagicSkill.generated.h"

UCLASS()
class YARO_API AMagicSkill : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMagicSkill();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class USphereComponent* Sphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UParticleSystemComponent* Particle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ExplosionFX")
	class UParticleSystem* ParticleFX;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UProjectileMovementComponent* MovementComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds")
	class USoundBase* ExplosionSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds")
	class USoundBase* MagicSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TSubclassOf<UDamageType> DamageTypeClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	AController* MagicInstigator;

	FORCEINLINE void SetInstigator(AController* Inst) { MagicInstigator = Inst; }

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "MoveActor")
	class ACharacter* Target;

	UPROPERTY(EditDefaultsOnly)
	int index;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Combat")
	class ACharacter* Caster; // who cast this spell(magic)

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// To move Target
	FVector Direction;
	float TotalDistance;
	float CurrentDistance;
	FVector StartLocation;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	virtual void OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void SetLocation(); //Move magic to the target
};
