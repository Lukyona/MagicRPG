// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Yaro/Character/Main.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Yaro/Character/YaroCharacter.h"
//#include "Components/SphereComponent.h"


AWeapon::AWeapon()
{
    SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
    SkeletalMesh->SetupAttachment(GetRootComponent());

    WeaponState = EWeaponState::EWS_Pickup;

}


void AWeapon::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    //UE_LOG(LogTemp, Log, TEXT("%s"), *(this->GetName()));
    Super::OnOverlapBegin(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
    if ((WeaponState == EWeaponState::EWS_Pickup) && OtherActor)
    {
        AMain* Main = Cast<AMain>(OtherActor);
        if (Main)
        {
            Main->SetActiveOverlappingItem(this);
        }
        
    }
}

void AWeapon::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    Super::OnOverlapEnd(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);
    if (OtherActor)
    {
        AMain* Main = Cast<AMain>(OtherActor);
        if (Main)
        {
            Main->SetActiveOverlappingItem(nullptr);
        }
    }
}

void AWeapon::Equip(AMain* Char)
{
    if (Char)
    {
        SkeletalMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
        SkeletalMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

        SkeletalMesh->SetSimulatePhysics(false);

        const USkeletalMeshSocket* RightHandSocket = Char->GetMesh()->GetSocketByName("RightHandSocket");
        if (RightHandSocket)
        {
            RightHandSocket->AttachActor(this, Char->GetMesh());
            bRotate = false;
            Char->SetEquippedWeapon(this);
            SetWeaponState(EWeaponState::EWS_Equipped);
        }
    }
}