// Fill out your copyright notice in the Description page of Project Settings.

#include "BlockActor.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"

// Sets default values
ABlockActor::ABlockActor()
{
	PrimaryActorTick.bCanEverTick = true;

	bTemporal = false;
}

// Called when the game starts or when spawned
void ABlockActor::BeginPlay()
{
	Super::BeginPlay();
	
	if (!bTemporal)
	{
		UBlockActorScene* BlockScene = Cast<UBlockActorScene>(GetWorld()->GetWorldSettings()->GetAssetUserDataOfClass(UBlockActorScene::StaticClass()));
		if (ensure(BlockScene))
		{
			BlockScene->RegisterBlockActor(this);
		}
	}
}

void ABlockActor::PostRegisterAllComponents()
{
	Super::PostRegisterAllComponents();

	GetRootComponent()->TransformUpdated.AddUObject(this, &ABlockActor::TransformUpdated);
}

void ABlockActor::PostUnregisterAllComponents()
{
	Super::PostUnregisterAllComponents();

	if (GetRootComponent())
	{
		GetRootComponent()->TransformUpdated.RemoveAll(this);
	}

	UBlockActorScene* BlockScene = GetWorld() ? Cast<UBlockActorScene>(GetWorld()->GetWorldSettings()->GetAssetUserDataOfClass(UBlockActorScene::StaticClass())) : nullptr;
	if (BlockScene)
	{
		BlockScene->UnRegisterBlockActor(this);
	}
}

void ABlockActor::TransformUpdated(USceneComponent* RootComponent, EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport)
{
	if (!bTemporal)
	{
		UBlockActorScene* BlockScene = Cast<UBlockActorScene>(GetWorld()->GetWorldSettings()->GetAssetUserDataOfClass(UBlockActorScene::StaticClass()));
		if (ensure(BlockScene))
		{
			BlockScene->RegisterBlockActor(this);
		}
	}
}

void ABlockActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void UBlockGenerator::GenerateRandomBlockWorld(UWorld* World, const TArray<TSubclassOf<ABlockActor>>& BlockClasses)
{
	if (!ensure(BlockClasses.Num() != 0))
	{
		return;
	}

	// Size
	const int32 SizeY = 20;
	const int32 SizeZ = 20;

	// Hole
	const FBox2D StartingArea(FVector2D(-5, 0), FVector2D(5, 4));
	const int32 HolePercentage = 30;

	FActorSpawnParameters ActorSpawnParam;
	ActorSpawnParam.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (int32 Y = -SizeY; Y <= SizeY; ++Y)
	{
		for (int32 Z = -SizeZ; Z <= SizeZ; ++Z)
		{
			if (StartingArea.IsInside(FVector2D(Y, Z)))
			{
				continue;
			}

			if (FMath::Rand() % 100 < HolePercentage)
			{
				continue;
			}

			const int32 ClassIndex = FMath::Rand() % BlockClasses.Num();

			TSubclassOf<ABlockActor> BlockClass = BlockClasses[ClassIndex];

			World->SpawnActor<ABlockActor>(BlockClass, FTransform(FVector(0, Y * 100, Z * 100)), ActorSpawnParam);
		}
	}
}

void UBlockActorScene::InitializeGrid(float InGridCellSize, int32 InGridX, int32 InGridY)
{
	GridCellSize = InGridCellSize;
	GridX = InGridX;
	GridY = InGridY;

	// *4 for negative area
	const int32 NumCols = (GridX * 2) + 1;
	const int32 NumRows = (GridY * 2) + 1;

	BlockActors.AddZeroed(NumCols * NumRows);
}

void UBlockActorScene::RegisterBlockActor(ABlockActor* BlockActor)
{
	BlockActors.Remove(BlockActor);

	int32 OldIndex = BlockActors.Find(BlockActor);
	if (OldIndex != INDEX_NONE)
	{
		BlockActors[OldIndex] = nullptr;
	}

	const int32 X = FMath::RoundToInt(BlockActor->GetActorLocation().Y / GridCellSize);
	const int32 Y = FMath::RoundToInt(BlockActor->GetActorLocation().Z / GridCellSize);

	if (X < -GridX || X > GridX)
	{
		ensure(0);
		return;
	}

	if (Y < -GridY || Y > GridY)
	{
		ensure(0);
		return;
	}

	const int32 OriginX = GetOriginX();
	const int32 OriginY = GetOriginY();
	const int32 XFromOrigin = X - OriginX;
	const int32 YFromOrigin = Y - OriginY;
	const int32 NumCols = (GridX * 2) + 1;
	const int32 Index = (XFromOrigin * NumCols) + YFromOrigin;

	BlockActors[Index] = BlockActor;
}

void UBlockActorScene::UnRegisterBlockActor(ABlockActor* BlockActor)
{
	int32 OldIndex = BlockActors.Find(BlockActor);
	if (OldIndex != INDEX_NONE)
	{
		BlockActors[OldIndex] = nullptr;
	}
}

ABlockActor* UBlockActorScene::GetBlock(int32 X, int32 Y) const
{
	if (X < -GridX || X > GridX)
	{
		ensure(0);
		return nullptr;
	}

	if (Y < -GridY || Y > GridY)
	{
		ensure(0);
		return nullptr;
	}

	const int32 OriginX = -GridX;
	const int32 OriginY = -GridY;
	const int32 XFromOrigin = X - OriginX;
	const int32 YFromOrigin = Y - OriginY;
	const int32 NumCols = (GridX * 2) + 1;
	const int32 Index = (XFromOrigin * NumCols) + YFromOrigin;

	return BlockActors[Index];
}

void UBlockActorScene::DebugDraw() const
{
	int32 NumActiveBlocks = 0;

	const int32 NumBlocks = BlockActors.Num();
	for (int32 Index = 0; Index < NumBlocks; ++Index)
	{
		if (BlockActors[Index])
		{
			++NumActiveBlocks;
		}
	}

	GEngine->AddOnScreenDebugMessage((uint64)(this + 0), 0, FColor::White,
		FString::Printf(TEXT("Active Blocks: %4d"), NumActiveBlocks));
}
