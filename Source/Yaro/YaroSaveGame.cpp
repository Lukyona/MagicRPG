// Fill out your copyright notice in the Description page of Project Settings.


#include "YaroSaveGame.h"

UYaroSaveGame::UYaroSaveGame()
{
    SaveName = TEXT("Default");
    UserIndex = 0;

    CharacterStats.WeaponName = TEXT("");

    //for (int i = 0; i < EnemyInfo.Num(); i++)
    //{
    //    EnemyInfo[i].EnemyName = TEXT("");
    //}

}
