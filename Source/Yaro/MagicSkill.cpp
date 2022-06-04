// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicSkill.h"
#include "Enemy.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Sound/SoundCue.h"
#include "GameFramework/Controller.h"

// Sets default values
AMagicSkill::AMagicSkill()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
    Sphere->SetupAttachment(GetRootComponent());

    Particle = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Particle"));
    Particle->SetupAttachment(Sphere);

    MovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("MovementComponent"));

    MovementComponent->InitialSpeed = 400.f;
    MovementComponent->MaxSpeed = 1000.f;

    MovementComponent->ProjectileGravityScale = 0.03f;


}

// Called when the game starts or when spawned
void AMagicSkill::BeginPlay()
{
	Super::BeginPlay();
    Sphere->OnComponentBeginOverlap.AddDynamic(this, &AMagicSkill::OnComponentBeginOverlap);

    if (MagicSound)
    {
        UGameplayStatics::PlaySound2D(this, MagicSound);
    }
    else if (ExplosionSound) // 폭발 사운드만 존재하는 경우
    {
        UGameplayStatics::PlaySound2D(this, ExplosionSound);
    }

    FTimerHandle WaitHandle;
    GetWorld()->GetTimerManager().SetTimer(WaitHandle, FTimerDelegate::CreateLambda([&]()
        {
            if (this->IsValidLowLevel())
            {
                Destroy(true);
            }

        }), 3.f, false);
}

// Called every frame
//void AMagicSkill::Tick(float DeltaTime)
//{
//	Super::Tick(DeltaTime);
//
//}

void AMagicSkill::OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor)
    {
        AEnemy* Enemy = Cast<AEnemy>(OtherActor);
        if (Enemy)
        {
            UGameplayStatics::PlaySound2D(this, ExplosionSound);
            if (this->GetName().Contains("Wind"))
            {
                UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ParticleFX, GetActorLocation());
                Destroy(true);
            }     
            if (DamageTypeClass)
            {
                Enemy->TakeDam(Damage);
                //UGameplayStatics::ApplyDamage(Enemy, Damage, MagicInstigator, this, DamageTypeClass);
            }
        }
        
    }
}

