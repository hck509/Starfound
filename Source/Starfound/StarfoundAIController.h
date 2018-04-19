// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Controller.h"
#include "StarfoundAIController.generated.h"

/**
 * 
 */
UCLASS()
class STARFOUND_API AStarfoundAIController : public AController
{
	GENERATED_BODY()
	
public:
	AStarfoundAIController();

	virtual void Tick(float DeltaSeconds);

	bool MoveToLocation(const FVector& TargetLocation);

private:
	void AssignJobIfNeeded();
	void MoveToJobLocation();
	void ExecuteJobIfInRange();
};
