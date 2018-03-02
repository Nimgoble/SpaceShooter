// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "SpaceShooterGameMode.h"
#include "SpaceShooterPawn.h"

ASpaceShooterGameMode::ASpaceShooterGameMode()
{
	// set default pawn class to our flying pawn
	DefaultPawnClass = ASpaceShooterPawn::StaticClass();
}
