// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Classes/AIController.h"
#include "StarfoundAIController.generated.h"

UCLASS()
class STARFOUND_API AStarfoundAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	AStarfoundAIController();

	virtual void Tick(float DeltaSeconds);

	bool MoveToLocation(const FVector& TargetLocation);

	UFUNCTION(BlueprintCallable)
	bool IsWorking() const { return bWorking; }

	UFUNCTION(BlueprintCallable)
	bool MoveToPickupItem(const class AItemActor* ItemActor);

private:
	void AssignJobIfNeeded();

	UFUNCTION(BlueprintCallable)
	bool MoveToJobLocation();

	UFUNCTION(BlueprintCallable)
	void WorkOnJobIfInRange(float DeltaSeconds);

	float ThinkingCoolSeconds;
	bool bWorking;
};
