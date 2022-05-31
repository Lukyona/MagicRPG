// Fill out your copyright notice in the Description page of Project Settings.


#include "MainPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

void AMainPlayerController::BeginPlay()
{
    Super::BeginPlay();
    
    this->PlayerCameraManager->ViewPitchMin = -50.f; // ����ȸ�� ��
    this->PlayerCameraManager->ViewPitchMax = 10.f; //�Ʒ�


}