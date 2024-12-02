// Fill out your copyright notice in the Description page of Project Settings.


#include "Item.h"
#include "Components/SphereComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Sound/SoundCue.h"

#include "Yaro/Character/Main.h"

// Sets default values
AItem::AItem()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false; // Default value is true

	CollisionVolume = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionVolume"));
	RootComponent = CollisionVolume;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(GetRootComponent());

}

// Called when the game starts or when spawned
void AItem::BeginPlay()
{
	Super::BeginPlay();

	CollisionVolume->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnOverlapBegin);
	CollisionVolume->OnComponentEndOverlap.AddDynamic(this, &AItem::OnOverlapEnd);
}

bool AItem::IsValidTarget(AActor* OtherActor) const
{
    return OtherActor && GetName().Contains("Stone") && Cast<AMain>(OtherActor);
}

void AItem::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (IsValidTarget(OtherActor))
    {
        AMain* Main = Cast<AMain>(OtherActor);
        if (Main->GetItemInHand() == nullptr)
        {
            Main->SetActiveOverlappingItem(this);
        }
    }
}

void AItem::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    if (IsValidTarget(OtherActor))
    {
        AMain* Main = Cast<AMain>(OtherActor);
        Main->SetActiveOverlappingItem(nullptr);
    }
}

void AItem::PickUp(AMain* Char)
{
    Mesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
    Mesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

    Mesh->SetSimulatePhysics(false);
    Mesh->bHiddenInGame = false;

    const USkeletalMeshSocket* LeftHandSocket = Char->GetMesh()->GetSocketByName("LeftHandSocket");
    if (LeftHandSocket)
    {
        LeftHandSocket->AttachActor(this, Char->GetMesh());
        bRotate = false;
        Char->SetItemInHand(this);
        Char->SetActiveOverlappingItem(nullptr);
    }
}
