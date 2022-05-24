// Copyright Epic Games, Inc. All Rights Reserved.

#include "YaroCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h" //GetPlayerCharacter
#include "AIController.h"
#include "Main.h"

//////////////////////////////////////////////////////////////////////////
// AYaroCharacter

AYaroCharacter::AYaroCharacter()
{

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;


	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

void AYaroCharacter::BeginPlay()
{
	Super::BeginPlay();

	AIController = Cast<AAIController>(GetController());
	MoveToPlayer();

}

void AYaroCharacter::MoveToPlayer()
{
	FTimerHandle WaitHandle;
	float WaitTime = 1.5f; // ������ Ÿ�� ����
	GetWorld()->GetTimerManager().SetTimer(WaitHandle, FTimerDelegate::CreateLambda([&]()
		{
			ACharacter* p = UGameplayStatics::GetPlayerCharacter(this, 0);
			AMain* player = Cast<AMain>(p);
			float distance = GetDistanceTo(player);

			if (distance >= 500.f) //���� �Ÿ� �̻� �������ִٸ� �ӵ� ���� �޸���
			{
				//UE_LOG(LogTemp, Log, TEXT("%s"), *(this->GetName()));
				if ((this->GetName()).Contains("Momo"))
				{
					GetCharacterMovement()->MaxWalkSpeed = 600.f;
				}
				else if ((this->GetName()).Contains("Zizi") || (this->GetName()).Contains("Vivi"))
				{
					GetCharacterMovement()->MaxWalkSpeed = 500.f;
				}
				else
				{
					GetCharacterMovement()->MaxWalkSpeed = 450.f;
				}
			}
			else //�����ٸ� �ӵ� ���� �ȱ�
			{
				if ((this->GetName()).Contains("Momo"))
				{
					GetCharacterMovement()->MaxWalkSpeed = 300.f;
				}
				else if ((this->GetName()).Contains("Zizi") || (this->GetName()).Contains("Vivi"))
				{
					GetCharacterMovement()->MaxWalkSpeed = 250.f;
				}
				else
				{
					GetCharacterMovement()->MaxWalkSpeed = 225.f;
				}
			}

			FAIMoveRequest MoveRequest;
			MoveRequest.SetGoalActor(player);
			MoveRequest.SetAcceptanceRadius(80.f);

			FNavPathSharedPtr NavPath;
			AIController->MoveTo(MoveRequest, &NavPath);

		}), WaitTime, true);	//������ �ð� �����Ͽ� ��� �ݺ�
}
