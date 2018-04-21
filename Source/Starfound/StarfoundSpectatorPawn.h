#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SpectatorPawn.h"
#include "StarfoundSpectatorPawn.generated.h"

/**
 * 
 */
UCLASS()
class STARFOUND_API AStarfoundSpectatorPawn : public ASpectatorPawn
{
	GENERATED_BODY()
	
public:
	AStarfoundSpectatorPawn();

	virtual void SetupPlayerInputComponent(UInputComponent* InInputComponent) override;
};
