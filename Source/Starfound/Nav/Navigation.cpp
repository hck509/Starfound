#include "Navigation.h"
#include "BlockActor.h"
#include "DrawDebugHelpers.h"

ANavigation::ANavigation()
{
	PrimaryActorTick.bCanEverTick = true;

	Graph.Reset(new FSideScrollGraph);
	MicroPather.Reset(new MicroPanther::MicroPather(Graph.Get(), 250, 6, false));
}

void ANavigation::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UBlockActorScene* BlockScene = GetWorld() ? Cast<UBlockActorScene>(GetWorld()->GetWorldSettings()->GetAssetUserDataOfClass(UBlockActorScene::StaticClass())) : nullptr;
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
				const int32 WorldGridX = BlockScene->OriginSpaceGridToWorldSpaceGridX(X);
				const int32 WorldGridY = BlockScene->OriginSpaceGridToWorldSpaceGridX(Y);
				ABlockActor* BlockActor = BlockScene->GetBlock(WorldGridX, WorldGridY);

				if (BlockActor)
				{
					Graph->SetHeight(X, Y, -1);
				}
				else
				{
					Graph->SetHeight(X, Y, 0);
				}
			}
		}
	}
}

bool ANavigation::FindPath(const FVector& StartLocation, const FVector& TargetLocation, TArray<FVector2D>& OutPath)
{
	UBlockActorScene* BlockScene = GetWorld() ? Cast<UBlockActorScene>(GetWorld()->GetWorldSettings()->GetAssetUserDataOfClass(UBlockActorScene::StaticClass())) : nullptr;
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

	MP_VECTOR<void*> Path;
	float TotalCost;

	const int32 Result = MicroPather->Solve(StartState, EndState, &Path, &TotalCost);

	if (Result == MicroPanther::MicroPather::SOLVED)
	{
		for (uint32 i = 0; i < Path.size(); ++i)
		{
			FIntPoint Point = Graph->StateToVec2(Path[i]);
			FVector2D WorldSpaceLocation = BlockScene->OriginSpaceGridToWorldSpace(Point);

			OutPath.Add(WorldSpaceLocation);
		}

		return true;
	}

	return false;
}

void ANavigation::DebugDraw() const
{
	UBlockActorScene* BlockScene = GetWorld() ? Cast<UBlockActorScene>(GetWorld()->GetWorldSettings()->GetAssetUserDataOfClass(UBlockActorScene::StaticClass())) : nullptr;
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

			const FVector WorldPosition(100, WorldGridX * BlockScene->GetGridCellSize(), WorldGridY * BlockScene->GetGridCellSize());

			if (Height == -1)
			{
				DrawDebugPoint(GetWorld(), WorldPosition, 20.0f, FColor::Green);
			}
		}
	}
}
