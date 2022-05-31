// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicSkill.h"
#include "Enemy.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"


// Sets default values
AMagicSkill::AMagicSkill()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
    Sphere->SetupAttachment(GetRootComponent());

   /* static ConstructorHelpers::FObjectFinder<UParticleSystem> Particle(TEXT("/Game/Luos8Elements/Particles/Wind/Par_LMagic_Wind_Explosion.Par_LMagic_Wind_Explosion"));
    ParticleFX = Particle.Object;*/
}

// Called when the game starts or when spawned
void AMagicSkill::BeginPlay()
{
	Super::BeginPlay();
    Sphere->OnComponentBeginOverlap.AddDynamic(this, &AMagicSkill::OnComponentBeginOverlap);
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
            UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ParticleFX, GetActorLocation());
            Destroy(true);
        }
        
    }
}