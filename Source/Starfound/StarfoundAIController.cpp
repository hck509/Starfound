#include "StarfoundAIController.h"
#include "StarfoundGameMode.h"
#include "StarfoundMovementComponent.h"
#include "StarfoundPawn.h"
#include "DrawDebugHelpers.h"

const static float JobReachDistance = 350;

static bool _IsJobInReach(const AStarfoundPawn& Pawn, const FIntPoint& JobLocation)
{
	UBlockActorScene* BlockScene = GetBlockActorScene(Pawn.GetWorld());

	if (!BlockScene)
	{
		return false;
	}

	const FIntPoint PawnLocation = BlockScene->WorldSpaceToOriginSpaceGrid(Pawn.GetActorLocation());

	const FIntPoint LocationDiff = PawnLocation - JobLocation;

	if (FMath::Abs(LocationDiff.X) <= 1 && FMath::Abs(LocationDiff.Y) <= 3)
	{
		return true;
	}

	return false;
}


AStarfoundAIController::AStarfoundAIController()
{
	PrimaryActorTick.bCanEverTick = true;

	bWorking = false;
}

void AStarfoundAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	ThinkingCoolSeconds -= DeltaSeconds;

	if (ThinkingCoolSeconds < 0)
	{
		ThinkingCoolSeconds = 1.0f;
		AssignJobIfNeeded();
		MoveToJobLocation();
	}

	WorkOnJobIfInRange(DeltaSeconds);
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
		DrawDebugString(GetWorld(), FVector(0, 0, 150), "Idle", Pawn, FColor::White, 0, true);
		return;
	}

	UBlockActorScene* BlockScene = GetBlockActorScene(GetWorld());

	if (!BlockScene)
	{
		return;
	}

	const FVector2D JobLocation2D = BlockScene->OriginSpaceGridToWorldSpace2D(Job.Location);
	const FVector JobLocation(0, JobLocation2D.X, JobLocation2D.Y);

	DrawDebugLine(GetWorld(), Pawn->GetActorLocation(), JobLocation, FColor::Green);

	FVector TargetLocation;
	bool bFoundValidTargetLocation = false;

	const bool bJobInReach = _IsJobInReach(*Pawn, Job.Location);

	if (!bJobInReach)
	{
		float TargetLocationMinDistance = 10E5;
		
		for (int32 X = -1; X <= 1; ++X)
		{
			for (int32 Y = -3; Y <= 1; ++Y)
			{
				if (X == 0 && Y == 0)
				{
					continue;
				}

				const FIntPoint Location = Job.Location + FIntPoint(X, Y);

				const bool bFoundValidNeighbor = GameMode->GetNavigation()->IsValidGridLocation(Location);
				const bool bHasFloor = BlockScene->GetBlock(Location.X, Location.Y - 1);

				if (bFoundValidNeighbor && bHasFloor)
				{
					const FVector TargetLocationCandidate = BlockScene->OriginSpaceGridToWorldSpace(Location);

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
			DrawDebugString(GetWorld(), FVector(0, 0, 150), "Noway", Pawn, FColor::White, 0, true);

			GameMode->GetJobQueue()->AssignAnotherJob(Pawn);
		}
		else
		{
			const bool bPathFound = MoveToLocation(TargetLocation);
			ensure(bPathFound);
		}
	}
}

void AStarfoundAIController::WorkOnJobIfInRange(float DeltaSeconds)
{
	bWorking = false;

	AStarfoundPawn* Pawn = Cast<AStarfoundPawn>(GetPawn());

	if (!Pawn)
	{
		return;
	}

	if (Pawn->GetStarfoundMovementController()->GetSpeed() != 0)
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

	const bool bJobInReach = _IsJobInReach(*Pawn, Job.Location);

	if (bJobInReach)
	{
		const float ProgressPercentage = GameMode->GetJobQueue()->ProgressAssignedJob(Pawn, DeltaSeconds * 30.0f);
		
		if (ProgressPercentage > 100.0f)
		{
			GameMode->GetJobExecutor()->FinishJob(Job);
			GameMode->GetJobQueue()->PopAssignedJob(Pawn);

			bWorking = false;
		}
		else
		{
			bWorking = true;
		}
	}
}
