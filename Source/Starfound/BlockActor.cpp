// Fill out your copyright notice in the Description page of Project Settings.

#include "BlockActor.h"


// Sets default values
ABlockActor::ABlockActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABlockActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
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
