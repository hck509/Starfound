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

	UFUNCTION(BlueprintCallable)
	float GetSpeed() const;

	UFUNCTION(BlueprintCallable)
	float GetMaxSpeed() const { return MaxSpeed; }

private:
	bool IsFreefalling() const;

	void PopupIfBuried();
	void TickFreefall(float DeltaTime);
	void TickFollowPath(float DeltaTime);
	void TickRotateToJobLocation(float DeltaTime);

	/**
	 * Path Following
	 */
	TArray<FVector2D> FollowingPath;

	UPROPERTY(EditDefaultsOnly)
	float MaxSpeed;

	UPROPERTY()
	float FallingSpeed;
};
