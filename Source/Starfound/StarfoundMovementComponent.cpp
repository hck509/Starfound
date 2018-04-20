#include "StarfoundMovementComponent.h"
#include "BlockActor.h"
#include "StarfoundGameMode.h"

UStarfoundMovementComponent::UStarfoundMovementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	MaxSpeed = 100;
	FallingSpeed = 0;
}

void UStarfoundMovementComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UStarfoundMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	PopupIfBuried();

	const bool bFalling = IsFreefalling();

	if (bFalling)
	{
		TickFreefall(DeltaTime);
	}
	else if (FollowingPath.Num() > 0)
	{
		FallingSpeed = 0;
		
		TickFollowPath(DeltaTime);
	}
	else
	{
		TickRotateToJobLocation(DeltaTime);
	}
}

void UStarfoundMovementComponent::FollowPath(const TArray<FVector2D>& InFollowingPath)
{
	FollowingPath = InFollowingPath;

	if (FollowingPath.Num() > 0)
	{
		FollowingPath[0] = FVector2D(GetOwner()->GetActorLocation().Y, GetOwner()->GetActorLocation().Z);
	}
}

float UStarfoundMovementComponent::GetSpeed() const
{
	if (FollowingPath.Num() > 0)
	{
		return MaxSpeed;
	}

	return 0;
}

void UStarfoundMovementComponent::TickFreefall(float DeltaTime)
{
	FallingSpeed += 980 * DeltaTime;

	GetOwner()->SetActorLocation(GetOwner()->GetActorLocation() + FVector(0, 0, -FallingSpeed * DeltaTime));
}

void UStarfoundMovementComponent::TickFollowPath(float DeltaTime)
{
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
		const FRotator TargetRotation = (Movement.Y == 0) ? OldRotation : 
			(Movement.Y > 0) ? FRotator(0, 0, 0) : FRotator(0, 180, 0);
		const FRotator NewRotation = FMath::Lerp(OldRotation, TargetRotation, FMath::Clamp(DeltaTime * 10, 0.01f, 0.1f));

		GetOwner()->SetActorLocation(NewLocation);
		GetOwner()->SetActorRotation(NewRotation);
	}

	if (FollowingPath.Num() == 1)
	{
		FollowingPath.Empty();
	}
}

void UStarfoundMovementComponent::TickRotateToJobLocation(float DeltaTime)
{
	UBlockActorScene* BlockScene = GetBlockActorScene(GetWorld());

	if (!BlockScene)
	{
		return;
	}

	AStarfoundGameMode* GameMode = Cast<AStarfoundGameMode>(GetWorld()->GetAuthGameMode());
	if (!GameMode)
	{
		return;
	}

	FStarfoundJob Job;
	const bool bHasJob = GameMode->GetJobQueue()->GetAssignedJob(Cast<AStarfoundPawn>(GetOwner()), Job);

	if (bHasJob)
	{
		const FRotator OldRotation = GetOwner()->GetActorRotation();
		const FVector JobLocation = BlockScene->OriginSpaceGridToWorldSpace(Job.Location);

		const FVector Direction = JobLocation - GetOwner()->GetActorLocation();

		const FRotator TargetRotation = (Direction.Y == 0) ? OldRotation :
			(Direction.Y > 0) ? FRotator(0, 0, 0) : FRotator(0, 180, 0);
		const FRotator NewRotation = FMath::Lerp(OldRotation, TargetRotation, FMath::Clamp(DeltaTime * 10, 0.01f, 0.1f));

		GetOwner()->SetActorRotation(NewRotation);
	}
}

bool UStarfoundMovementComponent::IsFreefalling() const
{
	UBlockActorScene* BlockScene = GetBlockActorScene(GetWorld());

	if (!BlockScene)
	{
		return false;
	}

	AStarfoundGameMode* GameMode = Cast<AStarfoundGameMode>(GetWorld()->GetAuthGameMode());
	if (!GameMode)
	{
		return false;
	}

	const FIntPoint PawnLocation = BlockScene->WorldSpaceToOriginSpaceGrid(GetOwner()->GetActorLocation() + FVector(0, 0, BlockScene->GetGridCellSize() * 0.5f));
	const FIntPoint FloorLocation = PawnLocation + FIntPoint(0, -1);

	ABlockActor* FloorBlock = BlockScene->GetBlock(FloorLocation.X, FloorLocation.Y);

	if (!FloorBlock)
	{
		// See if we are climbing
		if (FollowingPath.Num() > 0)
		{
			const FIntPoint SecondFloorLocation = PawnLocation + FIntPoint(0, -2);
			ABlockActor* SecondFloorBlock = BlockScene->GetBlock(SecondFloorLocation.X, SecondFloorLocation.Y);

			if (SecondFloorBlock)
			{
				return false;
			}
		}

		return true;
	}

	return false;
}

void UStarfoundMovementComponent::PopupIfBuried()
{
	UBlockActorScene* BlockScene = GetBlockActorScene(GetWorld());

	if (!BlockScene)
	{
		return;
	}

	const FIntPoint PawnLocation = BlockScene->WorldSpaceToOriginSpaceGrid(GetOwner()->GetActorLocation());

	const bool bIsBuried = (BlockScene->GetBlock(PawnLocation) != nullptr);

	if (bIsBuried)
	{
		bool bHasEnoughSpaceToPop = 
			(BlockScene->GetBlock(PawnLocation + FIntPoint(0, 1)) == nullptr)
			&& (BlockScene->GetBlock(PawnLocation + FIntPoint(0, 2)) == nullptr);

		if (bHasEnoughSpaceToPop)
		{
			const FVector NewLocation = BlockScene->OriginSpaceGridToWorldSpace(PawnLocation + FIntPoint(0, 1));

			GetOwner()->SetActorLocation(NewLocation);
		}
	}
}

