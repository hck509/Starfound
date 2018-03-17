// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BlockActor.h"
#include "StarfoundGameMode.generated.h"

UCLASS(minimalapi)
class AStarfoundGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AStarfoundGameMode();

	virtual void StartPlay() override;

private:

	UPROPERTY(EditDefaultsOnly)
	TArray<TSubclassOf<ABlockActor>> BlockClasses;

	UPROPERTY(Transient)
	UBlockGenerator* BlockGenerator;
};
