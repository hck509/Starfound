// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "StarfoundGameMode.h"
#include "StarfoundCharacter.h"
#include "UObject/ConstructorHelpers.h"

AStarfoundGameMode::AStarfoundGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/SideScrollerCPP/Blueprints/SideScrollerCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	PrimaryActorTick.bCanEverTick = true;
}

void AStarfoundGameMode::StartPlay()
{
	BlockActorScene = NewObject<UBlockActorScene>();
	GetWorld()->GetWorldSettings()->AddAssetUserData(BlockActorScene);
	BlockActorScene->InitializeGrid(100.0f, 100, 100);

	Super::StartPlay();

	BlockGenerator = NewObject<UBlockGenerator>();
	BlockGenerator->GenerateRandomBlockWorld(GetWorld(), BlockClasses);
}

void AStarfoundGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	BlockActorScene->DebugDraw();
}
