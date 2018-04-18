// Fill out your copyright notice in the Description page of Project Settings.

#include "StarfoundMovementComponent.h"


UStarfoundMovementComponent::UStarfoundMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	MaxSpeed = 100;
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
		const float MoveSpeed = MaxSpeed;
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

		const FVector OldLocation = GetOwner()->GetActorLocation();
		const FRotator OldRotation = GetOwner()->GetActorRotation();
		const FVector NewLocation = FVector(0, FollowingPath[0].X, FollowingPath[0].Y);
		const FVector Movement = NewLocation - OldLocation;
		const FRotator TargetRotation = (Movement.Y > 0) ? FRotator(0, 0, 0) : FRotator(0, 180, 0);
		const FRotator NewRotation = FMath::Lerp(OldRotation, TargetRotation, FMath::Clamp(DeltaTime * 10, 0.01f, 0.1f));

		GetOwner()->SetActorLocation(NewLocation);
		GetOwner()->SetActorRotation(NewRotation);

		if (FollowingPath.Num() == 1)
		{
			FollowingPath.Empty();
		}
	}
}

void UStarfoundMovementComponent::FollowPath(const TArray<FVector2D>& InFollowingPath)
{
	FollowingPath = InFollowingPath;
}

float UStarfoundMovementComponent::GetSpeed() const
{
	if (FollowingPath.Num() > 0)
	{
		return MaxSpeed;
	}

	return 0;
}

