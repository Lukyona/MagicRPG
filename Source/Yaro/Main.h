// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Main.generated.h"

UCLASS()
class YARO_API AMain : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMain();

	// CameraBoom positioning the camera behind the player, ī�޶���� �÷��̾� �ڿ� ī�޶� ��ġ��Ŵ
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	// Base turn rates to scale turning functions for the camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseTurnRate;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseLookUpRate;

	//�÷��̾� ����
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerInfo")
	int gender;

	//�޸��� �������� Ȯ��
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement)
	bool bRunning;

	/**
	/*
	/* Player Stats
	/*
	*/
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category  = "Player Stats")
	float MaxHP;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player Stats")
	float HP;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Stats")
	float MaxMP;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player Stats")
	float MP;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player Stats")
	float MaxSP;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player Stats")
	float SP;



protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Called for forwards/backwards input
	void MoveForward(float Value);

	// Called for side to side input
	void MoveRight(float Value);

	// Called for shift input
	void Run(float Value);

	// Called via(means ~�� ����) input to turn at a given rate
	// @param(�Ű����� ����) Rate This is a normalized rate, i.e.(means �ٽ� ����, �ٷ�) 1.0 means 100% of desired turn rate
	void TurnAtRate(float Rate);

	// Called via(means ~�� ����) input to look up/down at a given rate
	// @param(�Ű����� ����) Rate This is a normalized rate, i.e.(means �ٽ� ����, �ٷ�) 1.0 means 100% of desired look up/down rate
	void LookUpAtRate(float Rate);


	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

};
