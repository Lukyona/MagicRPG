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

	// CameraBoom positioning the camera behind the player, 카메라붐은 플레이어 뒤에 카메라를 위치시킴
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	// Base turn rates to scale turning functions for the camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseTurnRate;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseLookUpRate;

	//플레이어 성별
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PlayerInfo")
	int gender;

	//달리는 상태인지 확인
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

	// About CameraZoom
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraZoom")
	float MinZoomLength = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraZoom")
	float MaxZoomLength = 800.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraZoom")
	float DefaultArmLength = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraZoom")
	float ZoomStep = 30.f;

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

	// Called via(means ~를 통해) input to turn at a given rate
	// @param(매개변수 설명) Rate This is a normalized rate, i.e.(means 다시 말해, 바로) 1.0 means 100% of desired turn rate
	void TurnAtRate(float Rate);

	// Called via(means ~를 통해) input to look up/down at a given rate
	// @param(매개변수 설명) Rate This is a normalized rate, i.e.(means 다시 말해, 바로) 1.0 means 100% of desired look up/down rate
	void LookUpAtRate(float Rate);

	void CameraZoom(const float Value);

	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	bool bLMBDown;
	void LMBDown();
	void LMBUp();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Items)
	class AWeapon* EquippedWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Items)
	class AItem* ActiveOverlappingItem;

	FORCEINLINE void SetEquippedWeapon(AWeapon* WeaponToSet) { EquippedWeapon = WeaponToSet; }
	FORCEINLINE void SetActiveOverlappingItem(AItem* Item) { ActiveOverlappingItem = Item; }
};
