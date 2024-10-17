// Fill out your copyright notice in the Description page of Project Settings.


#include "System/UIManager.h"
#include "UIManager.h"

void UUIManager::Init()
{
    GameManager = Cast<UGameManager>(GetWorld()->GetGameInstance());
    if (GameManager)
    {

    }
    else return;

}
