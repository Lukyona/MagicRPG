// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicSkill.h"
#include "Enemy.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Sound/SoundCue.h"
#include "GameFramework/Controller.h"
#include "Main.h"

// Sets default values
AMagicSkill::AMagicSkill()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
    RootComponent = Sphere;

    Sphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
    Sphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Ignore);

    Particle = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Particle"));
    Particle->SetupAttachment(Sphere);

    MovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("MovementComponent"));

    MovementComponent->InitialSpeed = 0.f;
    MovementComponent->MaxSpeed = 500.f;

    MovementComponent->ProjectileGravityScale = 0.03f;

    PrimaryActorTick.bCanEverTick = true;

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
    GetWorld()->GetTimerManager().SetTimer(WaitHandle, FTimerDelegate::CreateLambda([&]() {
        if (this->IsValidLowLevel())
        {
            if (this->GetName().Contains("Tornado") || this->GetName().Contains("Laser")) return;
            Destroy(true);
        }

    }), 1.5f, false);

    FTimerHandle WaitHandle2;
    GetWorld()->GetTimerManager().SetTimer(WaitHandle2, FTimerDelegate::CreateLambda([&]() {
        if (this->GetName().Contains("Hit") || this->GetName().Contains("Storm"))
        {
            SetLocation();
        }
    }), 0.1f, false);


}

void AMagicSkill::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (Target != nullptr)
    {
        if (CurrentDistance < TotalDistance)
        {
            FVector Location = GetActorLocation();

            Location += Direction * MovementComponent->InitialSpeed * DeltaTime;

            SetActorLocation(Location);

            CurrentDistance = (Location - StartLocation).Size();

        }
    }
}

void AMagicSkill::SetLocation()
{
    if (Target != nullptr)
    {
        StartLocation = GetActorLocation();
        Direction = Target->GetActorLocation() - StartLocation;
        TotalDistance = Direction.Size();

        Direction = Direction.GetSafeNormal();
        CurrentDistance = 0.0f;
    }
}

void AMagicSkill::OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor)
    {
        if (this->index != 10)
        {
            AEnemy* Enemy = Cast<AEnemy>(OtherActor);
            if (Enemy)
            {
                if (index == 0)
                {
                    Enemy->bAttackFromPlayer = true;
                    if (Enemy->Main->CombatTarget == nullptr)
                    {
                        if (Enemy->Main->Targets.Contains(Enemy))
                        {
                            Enemy->Main->bAutoTargeting = true;
                            Enemy->Main->CombatTarget = Enemy; // 자동으로 타겟 지정
                            Enemy->Main->Targeting();
                            Enemy->Main->bAutoTargeting = false;
                        }

                    }
                }

                UGameplayStatics::PlaySound2D(this, ExplosionSound);
                if (this->GetName().Contains("Hit"))
                {
                    UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ParticleFX, GetActorLocation());
                    Destroy(true);
                }
                if (DamageTypeClass)
                {
                    if (this->GetName().Contains("Laser")) return;
                    UGameplayStatics::ApplyDamage(Enemy, Damage, MagicInstigator, this, DamageTypeClass);
                }
            }
        }
       
        if (index == 10) // enemy's magic skill
        {
            if (auto actor = Cast<AEnemy>(OtherActor)) return; // 오버랩된 게 Enemy라면 코드 실행X

            ACharacter* TargetCharacter = Cast<ACharacter>(OtherActor);
            if (TargetCharacter)
            {
                UGameplayStatics::PlaySound2D(this, ExplosionSound);
                if (this->GetName().Contains("Hit"))
                {
                    UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ParticleFX, GetActorLocation());

                    Destroy(true);
                }
                if (DamageTypeClass)
                {
                    UGameplayStatics::ApplyDamage(TargetCharacter, Damage, MagicInstigator, this, DamageTypeClass);

                }
            }
        }
    }
}

