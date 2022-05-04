// Copyright Epic Games, Inc. All Rights Reserved.

#include "Project_DeepBrownGameMode.h"
#include "Project_DeepBrownCharacter.h"
#include "UObject/ConstructorHelpers.h"

AProject_DeepBrownGameMode::AProject_DeepBrownGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
