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
	//DrawDebugPoint(GetWorld(), TargetLocation + FVector(10, 0, 0), 40.0f, FColor::Blue, false, 5.0f);

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

	if (Distance > 100)
	{
		const bool bPathFound = MoveToLocation(JobLocation);

		if (!bPathFound)
		{
			GameMode->GetJobQueue()->AssignAnotherJob(Pawn);
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

	if (Distance <= 100)
	{
		GameMode->GetJobExecutor()->ExecuteJob(Job);
		GameMode->GetJobQueue()->PopAssignedJob(Pawn);
	}
}
