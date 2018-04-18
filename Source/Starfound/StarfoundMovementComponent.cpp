// Fill out your copyright notice in the Description page of Project Settings.

#include "StarfoundMovementComponent.h"


UStarfoundMovementComponent::UStarfoundMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UStarfoundMovementComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UStarfoundMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (FollowingPath.Num() > 1)
	{
		const float MoveSpeed = 100;
		const float MoveDistance = DeltaTime * MoveSpeed;

		float MoveDistanceLeft = MoveDistance;

		while (FollowingPath.Num() > 1 && MoveDistanceLeft > 0)
		{
			const float StepDistance = (FollowingPath[1] - FollowingPath[0]).Size();

			if (StepDistance > MoveDistanceLeft)
			{
				FollowingPath[0] += (FollowingPath[1] - FollowingPath[0]).GetSafeNormal() * MoveDistanceLeft;
				MoveDistanceLeft = 0;
			}
			else
			{
				FollowingPath.RemoveAt(0);
				MoveDistanceLeft -= StepDistance;
			}
		}

		GetOwner()->SetActorLocation(FVector(0, FollowingPath[0].X, FollowingPath[0].Y));
	}
}

void UStarfoundMovementComponent::FollowPath(const TArray<FVector2D>& InFollowingPath)
{
	FollowingPath = InFollowingPath;
}

