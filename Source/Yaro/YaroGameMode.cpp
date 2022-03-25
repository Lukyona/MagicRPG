// Copyright Epic Games, Inc. All Rights Reserved.

#include "YaroGameMode.h"
#include "YaroCharacter.h"
#include "UObject/ConstructorHelpers.h"

AYaroGameMode::AYaroGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
