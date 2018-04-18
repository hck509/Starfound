#include "StarfoundAIController.h"
#include "StarfoundGameMode.h"
#include "StarfoundMovementComponent.h"
#include "StarfoundPawn.h"
#include "DrawDebugHelpers.h"

AStarfoundAIController::AStarfoundAIController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AStarfoundAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
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

		AStarfoundPawn* Pawn = Cast<AStarfoundPawn>(GetPawn());

		if (Pawn)
		{
			Pawn->GetStarfoundMovementController()->FollowPath(PathPoints);
		}
	}
}
