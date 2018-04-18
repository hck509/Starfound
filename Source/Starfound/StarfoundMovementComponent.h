// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StarfoundMovementComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class STARFOUND_API UStarfoundMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	
	UStarfoundMovementComponent();

protected:
	
	virtual void BeginPlay() override;

public:	
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void FollowPath(const TArray<FVector2D>& InFollowingPath);

private:
	/**
	 * Path Following
	 */
	TArray<FVector2D> FollowingPath;
};
