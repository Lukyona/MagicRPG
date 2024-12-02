// Fill out your copyright notice in the Description page of Project Settings.


#include "MagicSkill.h"
#include "Components/SphereComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

#include "Yaro/Character/Enemies/Enemy.h"
#include "Yaro/Character/Main.h"

constexpr float DEFAULT_DESTROY_DELAY = 1.5f;
constexpr float DEFAULT_LOCATION_UPDATE_DELAY = 0.5f;

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

}

// Called when the game starts or when spawned
void AMagicSkill::BeginPlay()
{
	Super::BeginPlay();
    Sphere->OnComponentBeginOverlap.AddDynamic(this, &AMagicSkill::OnComponentBeginOverlap);
    
    PlaySound(MagicSound);
    if(!MagicSound)
    {
        PlaySound(ExplosionSound);
    }

    if(SkillType == EMagicSkillType::Hit
        || SkillType == EMagicSkillType::Storm
        || SkillType == EMagicSkillType::Basic)
    {
        StartDestroyTimer(DEFAULT_DESTROY_DELAY);

        if (SkillType != EMagicSkillType::Basic)
        {
            StartLocationUpdateTimer(DEFAULT_LOCATION_UPDATE_DELAY);
        }
    }
}

void AMagicSkill::StartDestroyTimer(float Delay)
{
    GetWorld()->GetTimerManager().SetTimer(DestroyTimerHandle, FTimerDelegate::CreateLambda([this]()
    {
        Destroy();
    }), Delay, false);
}

void AMagicSkill::StartLocationUpdateTimer(float Delay)
{
    GetWorld()->GetTimerManager().SetTimer(LocationTimerHandle, this, &AMagicSkill::SetLocation, Delay, false);
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

void AMagicSkill::SetMain()
{
    Main = Cast<AMain>(UGameplayStatics::GetPlayerCharacter(this, 0));
}

void AMagicSkill::PlaySound(USoundBase* Sound)
{
    if (Sound)
    {
        UGameplayStatics::PlaySound2D(this, Sound);
    }
}

void AMagicSkill::HandleStudentCasterOverlap(AActor* OtherActor)
{
    AEnemy* Enemy = Cast<AEnemy>(OtherActor);
    if (Enemy)
    {
        if (Caster == ECasterType::Player && Main->GetCombatTarget() == nullptr
            && Main->GetTargets().Contains(Enemy))
        {
            Main->SetAutoTargeting(true);
            Main->SetCombatTarget(Enemy); // 자동으로 타겟 지정
            Main->Targeting();
            Main->SetAutoTargeting(false);
        }

        PlaySound(ExplosionSound);

        if (SkillType == EMagicSkillType::Hit)
        {
            UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ParticleFX, GetActorLocation());
            Destroy(true);
        }

        if (DamageTypeClass)
        {
            if (SkillType == EMagicSkillType::Laser)
            {
                return;
            }
            UGameplayStatics::ApplyDamage(Enemy, Damage, MagicInstigator, this, DamageTypeClass);
        }
    }
}

void AMagicSkill::HandleEnemyCasterOverlap(AActor* OtherActor)
{
    if (auto actor = Cast<AEnemy>(OtherActor))
    {
        return;
    }

    ACharacter* TargetCharacter = Cast<ACharacter>(OtherActor);
    if (TargetCharacter)
    {
        PlaySound(ExplosionSound);

        if (SkillType == EMagicSkillType::Hit)
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
        if (!Main)
        {
            SetMain();
        }

        if (SkillType == EMagicSkillType::Healing && Main == OtherActor)
        {
            Main->AddHP(150.f);
            return;
        }
        
        if (Caster == ECasterType::Player || Caster == ECasterType::NPC)
        {
            HandleStudentCasterOverlap(OtherActor);
        }
       
        if (Caster == ECasterType::Enemy || Caster == ECasterType::Boss)
        {
            HandleEnemyCasterOverlap(OtherActor);
        }
    }
}
