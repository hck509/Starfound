#include "Navigation.h"
#include "BlockActor.h"
#include "DrawDebugHelpers.h"

ANavigation::ANavigation()
{
	PrimaryActorTick.bCanEverTick = true;

	Graph.Reset(new FSideScrollGraph);
	MicroPather.Reset(new MicroPanther::FMicroPather(Graph.Get(), 250, 6, false));
}

void ANavigation::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateGraph();

}

void ANavigation::UpdateGraph()
{
	UBlockActorScene* BlockScene = GetBlockActorScene(GetWorld());
	if (BlockScene)
	{
		if (Graph->GetGridCountX() != BlockScene->GetNumGridX() ||
			Graph->GetGridCountY() != BlockScene->GetNumGridY())
		{
			Graph->InitializeGrid(BlockScene->GetNumGridX(), BlockScene->GetNumGridY());
		}

		const int32 NumX = Graph->GetGridCountX();
		const int32 NumY = Graph->GetGridCountY();

		for (int32 X = 0; X < NumX; ++X)
		{
			for (int32 Y = 0; Y < NumY; ++Y)
			{
				ABlockActor* BlockActor = BlockScene->GetBlock(X, Y);

				// Have to have floor to move
				const ABlockActor* FloorBlockActor1 = BlockScene->GetBlock(X, Y - 1);
				const ABlockActor* FloorBlockActor2 = BlockScene->GetBlock(X, Y - 2);
				const ABlockActor* LeftWallBlockActor = BlockScene->GetBlock(X - 1, Y - 1);
				const ABlockActor* RightWallBlockActor = BlockScene->GetBlock(X + 1, Y - 1);

				const bool bHasFloor = (FloorBlockActor1) 
					|| (FloorBlockActor2 && (LeftWallBlockActor || RightWallBlockActor));

				// A pawn is 2 block tall
				const ABlockActor* UpperBlockActor = BlockScene->GetBlock(X, Y + 1);

				if (BlockActor || !bHasFloor || UpperBlockActor)
				{
					Graph->SetHeight(X, Y, -1);
				}
				else
				{
					int32 Cost = 0;
					
					if (!FloorBlockActor1)
					{
						Cost = 1;
					}

					Graph->SetHeight(X, Y, Cost);
				}
			}
		}
	}
}

bool ANavigation::FindPath(const FVector& StartLocation, const FVector& TargetLocation, TArray<FVector2D>& OutPath)
{
	UBlockActorScene* BlockScene = GetBlockActorScene(GetWorld());

	if (!ensure(BlockScene))
	{
		return false;
	}

	const int32 StartX = BlockScene->WorldSpaceToOriginSpaceGridX(StartLocation.Y);
	const int32 StartY = BlockScene->WorldSpaceToOriginSpaceGridX(StartLocation.Z);
	const int32 TargetX = BlockScene->WorldSpaceToOriginSpaceGridX(TargetLocation.Y);
	const int32 TargetY = BlockScene->WorldSpaceToOriginSpaceGridX(TargetLocation.Z);


	void* StartState = Graph->Vec2ToState(FIntPoint(StartX, StartY));
	void* EndState = Graph->Vec2ToState(FIntPoint(TargetX, TargetY));

	TArray<void*> Path;
	float TotalCost;

	const int32 Result = MicroPather->Solve(StartState, EndState, &Path, &TotalCost);

	if (Result == MicroPanther::FMicroPather::SOLVED)
	{
		for (int32 i = 0; i < Path.Num(); ++i)
		{
			FIntPoint Point = Graph->StateToVec2(Path[i]);
			FVector2D WorldSpaceLocation = BlockScene->OriginSpaceGridToWorldSpace2D(Point);

			OutPath.Add(WorldSpaceLocation);
		}

		return true;
	}

	return false;
}

bool ANavigation::IsValidLocation(const FVector& Location) const
{
	UBlockActorScene* BlockScene = GetBlockActorScene(GetWorld());

	if (!ensure(BlockScene))
	{
		return false;
	}

	const FIntPoint GridLocation = BlockScene->WorldSpaceToOriginSpaceGrid(Location);

	return IsValidGridLocation(GridLocation);
}

bool ANavigation::IsValidGridLocation(const FIntPoint& GridLocation) const
{
	const int32 GraphValue = Graph->GetHeight(GridLocation.X, GridLocation.Y);

	return (GraphValue == 0);
}

void ANavigation::DebugDraw() const
{
	UBlockActorScene* BlockScene = GetBlockActorScene(GetWorld());
	if (!BlockScene)
	{
		return;
	}

	const int32 NumX = Graph->GetGridCountX();
	const int32 NumY = Graph->GetGridCountY();

	for (int32 X = 0; X < NumX; ++X)
	{
		for (int32 Y = 0; Y < NumY; ++Y)
		{
			const int32 Height = Graph->GetHeight(X, Y);

			const int32 WorldGridX = BlockScene->OriginSpaceGridToWorldSpaceGridX(X);
			const int32 WorldGridY = BlockScene->OriginSpaceGridToWorldSpaceGridX(Y);

			const FVector WorldPosition(55, WorldGridX * BlockScene->GetGridCellSize(), WorldGridY * BlockScene->GetGridCellSize());

			if (Height != -1)
			{
				DrawDebugPoint(GetWorld(), WorldPosition, 5.0f, FColor::Green);
			}
		}
	}
}
