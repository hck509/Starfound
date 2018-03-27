// Fill out your copyright notice in the Description page of Project Settings.

#include "StarfoundAIController.h"
#include "StarfoundGameMode.h"
#include "DrawDebugHelpers.h"

AStarfoundAIController::AStarfoundAIController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AStarfoundAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (FollowingPath.Num() > 1)
	{
		const float MoveSpeed = 100;
		const float MoveDistance = DeltaSeconds * MoveSpeed;

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

		GetPawn()->SetActorLocation(FVector(0, FollowingPath[0].X, FollowingPath[0].Y));
	}
}

void AStarfoundAIController::MoveToLocation(const FVector& TargetLocation)
{
	AStarfoundGameMode* GameMode = Cast<AStarfoundGameMode>(GetWorld()->GetAuthGameMode());
	if (!GameMode)
	{
		return;
	}

	const FVector PawnLocation = GetPawn()->GetActorLocation();

	TArray<FVector2D> PathPoints;
	const bool bPathFound = GameMode->GetNavigation()->FindPath(PawnLocation, TargetLocation, PathPoints);

	DrawDebugPoint(GetWorld(), PawnLocation + FVector(10, 0, 0), 40.0f, FColor::Blue, false, 5.0f);
	DrawDebugPoint(GetWorld(), TargetLocation + FVector(10, 0, 0), 40.0f, FColor::Blue, false, 5.0f);

	if (bPathFound)
	{
		for (const FVector2D& Point : PathPoints)
		{
			DrawDebugPoint(GetWorld(), FVector(0, Point.X, Point.Y), 30.0f, FColor::Red, false, 5.0f);
		}

		FollowingPath = PathPoints;
		PathFollowingDistance = 0;
	}
}
