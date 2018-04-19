#include "StarfoundAIController.h"
#include "StarfoundGameMode.h"
#include "StarfoundMovementComponent.h"
#include "StarfoundPawn.h"
#include "DrawDebugHelpers.h"

const static float JobReachDistance = 150;

AStarfoundAIController::AStarfoundAIController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AStarfoundAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	AssignJobIfNeeded();
	MoveToJobLocation();
	ExecuteJobIfInRange();
}

bool AStarfoundAIController::MoveToLocation(const FVector& TargetLocation)
{
	AStarfoundGameMode* GameMode = Cast<AStarfoundGameMode>(GetWorld()->GetAuthGameMode());
	if (!GameMode)
	{
		return false;
	}

	const FVector PawnLocation = GetPawn()->GetActorLocation();

	TArray<FVector2D> PathPoints;
	const bool bPathFound = GameMode->GetNavigation()->FindPath(PawnLocation, TargetLocation, PathPoints);

	//DrawDebugPoint(GetWorld(), PawnLocation + FVector(10, 0, 0), 40.0f, FColor::Blue, false, 5.0f);
	DrawDebugPoint(GetWorld(), TargetLocation + FVector(10, 0, 0), 40.0f, FColor::Blue, false, 0.2f);

	if (bPathFound)
	{
		for (const FVector2D& Point : PathPoints)
		{
			//DrawDebugPoint(GetWorld(), FVector(0, Point.X, Point.Y), 30.0f, FColor::Red, false, 5.0f);
		}

		AStarfoundPawn* Pawn = Cast<AStarfoundPawn>(GetPawn());

		if (Pawn)
		{
			Pawn->GetStarfoundMovementController()->FollowPath(PathPoints);
		}
	}

	return bPathFound;
}

void AStarfoundAIController::AssignJobIfNeeded()
{
	AStarfoundPawn* Pawn = Cast<AStarfoundPawn>(GetPawn());

	if (!Pawn)
	{
		return;
	}

	AStarfoundGameMode* GameMode = Cast<AStarfoundGameMode>(GetWorld()->GetAuthGameMode());
	if (!GameMode)
	{
		return;
	}

	FStarfoundJob Job;
	const bool bJobAssigned = GameMode->GetJobQueue()->GetAssignedJob(Pawn, Job);
	
	if (!bJobAssigned)
	{
		GameMode->GetJobQueue()->AssignJob(Pawn);
	}
}

void AStarfoundAIController::MoveToJobLocation()
{
	AStarfoundPawn* Pawn = Cast<AStarfoundPawn>(GetPawn());

	if (!Pawn)
	{
		return;
	}

	AStarfoundGameMode* GameMode = Cast<AStarfoundGameMode>(GetWorld()->GetAuthGameMode());
	if (!GameMode)
	{
		return;
	}

	FStarfoundJob Job;
	const bool bJobAssigned = GameMode->GetJobQueue()->GetAssignedJob(Pawn, Job);

	if (!bJobAssigned)
	{
		return;
	}

	UBlockActorScene* BlockScene = GetBlockActorScene(GetWorld());

	if (!BlockScene)
	{
		return;
	}

	const FVector2D JobLocation2D = BlockScene->OriginSpaceGridToWorldSpace2D(Job.Location);
	const FVector JobLocation(0, JobLocation2D.X, JobLocation2D.Y);

	const float Distance = (JobLocation - Pawn->GetActorLocation()).Size();
	FVector TargetLocation;
	bool bFoundValidTargetLocation = false;

	if (Distance > JobReachDistance)
	{
		float TargetLocationMinDistance = 10E5;
		
		for (int32 X = -1; X <= 1; ++X)
		{
			bool bFoundValidNeighbor = false;

			for (int32 Y = -1; Y <= 1; ++Y)
			{
				if (X == 0 && Y == 0)
				{
					continue;
				}

				bFoundValidNeighbor = GameMode->GetNavigation()->IsValidGridLocation(Job.Location + FIntPoint(X, Y));

				if (bFoundValidNeighbor)
				{
					const FVector TargetLocationCandidate = BlockScene->OriginSpaceGridToWorldSpace(Job.Location + FIntPoint(X, Y));

					TArray<FVector2D> PathPoints;
					const bool bPathFound = GameMode->GetNavigation()->FindPath(Pawn->GetActorLocation(), TargetLocationCandidate, PathPoints);

					if (bPathFound)
					{
						const float Distance = (Pawn->GetActorLocation() - TargetLocationCandidate).Size();

						if (Distance < TargetLocationMinDistance)
						{
							TargetLocationMinDistance = Distance;
							TargetLocation = TargetLocationCandidate;
							bFoundValidTargetLocation = true;
						}
					}
				}
			}
		}

		if (!bFoundValidTargetLocation)
		{
			GameMode->GetJobQueue()->AssignAnotherJob(Pawn);
		}
		else
		{
			const bool bPathFound = MoveToLocation(TargetLocation);
			ensure(bPathFound);
		}
	}
}

void AStarfoundAIController::ExecuteJobIfInRange()
{
	AStarfoundPawn* Pawn = Cast<AStarfoundPawn>(GetPawn());

	if (!Pawn)
	{
		return;
	}

	AStarfoundGameMode* GameMode = Cast<AStarfoundGameMode>(GetWorld()->GetAuthGameMode());
	if (!GameMode)
	{
		return;
	}

	FStarfoundJob Job;
	const bool bJobAssigned = GameMode->GetJobQueue()->GetAssignedJob(Pawn, Job);

	if (!bJobAssigned)
	{
		return;
	}

	UBlockActorScene* BlockScene = GetBlockActorScene(GetWorld());

	if (!BlockScene)
	{
		return;
	}

	const FVector2D JobLocation2D = BlockScene->OriginSpaceGridToWorldSpace2D(Job.Location);
	const FVector JobLocation(0, JobLocation2D.X, JobLocation2D.Y);

	const float Distance = (JobLocation - Pawn->GetActorLocation()).Size();

	if (Distance <= JobReachDistance)
	{
		GameMode->GetJobExecutor()->ExecuteJob(Job);
		GameMode->GetJobQueue()->PopAssignedJob(Pawn);
	}
}
