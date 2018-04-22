// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/AssetUserData.h"
#include "BlockActor.generated.h"

UCLASS()
class STARFOUND_API ABlockActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ABlockActor();

public:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	virtual void PostRegisterAllComponents() override;
	virtual void PostUnregisterAllComponents() override;



	void SetTemporal(bool bInTemporal) { bTemporal = bInTemporal; }
	bool IsTemporal() const { return bTemporal; }

private:
	void TransformUpdated(USceneComponent* RootComponent, EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport);

	bool bTemporal;
};


UCLASS()
class UBlockGenerator : public UObject
{
	GENERATED_BODY()

public:

	void GenerateRandomBlockWorld(UWorld* World, const TArray<TSubclassOf<ABlockActor>>& BlockClasses);
};


UCLASS()
class UBlockActorScene : public UAssetUserData
{
	GENERATED_BODY()

public:

	void InitializeGrid(float InGridCellSize, int32 InGridX, int32 InGridY);

	void RegisterBlockActor(ABlockActor* BlockActor);
	void UnRegisterBlockActor(ABlockActor* BlockActor);

	float GetGridCellSize() const { return GridCellSize; }
	int32 GetGridX() const { return GridX; }
	int32 GetGridY() const { return GridY; }
	int32 GetNumGridX() const { return (GridX * 2) + 1; }
	int32 GetNumGridY() const { return (GridY * 2) + 1; }
	int32 GetOriginX() const { return -GridX; }
	int32 GetOriginY() const { return -GridY; }

	int32 OriginSpaceGridToWorldSpaceGridX(int32 X) const { return X + GetOriginX(); }
	int32 OriginSpaceGridToWorldSpaceGridY(int32 Y) const { return Y + GetOriginY(); }

	int32 WorldSpaceToOriginSpaceGridX(float X) const { return FMath::RoundToInt(X / GridCellSize) - GetOriginX(); }
	int32 WorldSpaceToOriginSpaceGridY(float Y) const { return FMath::RoundToInt(Y / GridCellSize) - GetOriginY(); }

	FIntPoint WorldSpaceToWorldSpaceGrid(const FVector& Location) const
	{
		const int32 X = FMath::RoundToInt(Location.Y / GridCellSize);
		const int32 Y = FMath::RoundToInt(Location.Z / GridCellSize);

		return FIntPoint(X, Y);
	}

	FIntPoint WorldSpaceToOriginSpaceGrid(const FVector& Location) const
	{
		return FIntPoint(WorldSpaceToOriginSpaceGridX(Location.Y), WorldSpaceToOriginSpaceGridY(Location.Z));
	}

	FVector2D OriginSpaceGridToWorldSpace2D(const FIntPoint& Point) const
	{
		return FVector2D((Point.X + GetOriginX()) * GridCellSize, (Point.Y + GetOriginY()) * GridCellSize);
	}

	UFUNCTION(BlueprintCallable)
	FVector OriginSpaceGridToWorldSpace(const FIntPoint& Point) const
	{
		return FVector(0, (Point.X + GetOriginX()) * GridCellSize, (Point.Y + GetOriginY()) * GridCellSize);
	}

	ABlockActor* GetBlock(int32 X, int32 Y) const;

	UFUNCTION(BlueprintCallable)
	ABlockActor* GetBlock(const FIntPoint& Location) const;

	void DebugDrawBoxAt(const FIntPoint& OriginSpaceGridLocation, const FColor& Color) const;

	void DebugDraw() const;

private:

	float GridCellSize;

	// Grid count. Inclusive
	int32 GridX;
	int32 GridY;

	UPROPERTY()
	TArray<ABlockActor*> BlockActors;
};

UBlockActorScene* GetBlockActorScene(UWorld* World);


UCLASS()
class UStarfoundHelper : public UObject
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
	static UBlockActorScene* GetBlockActorScene(const UObject* WorldContextObject);
};
