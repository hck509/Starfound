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
	int32 OriginSpaceToWorldSpaceX(int32 InX) { return InX + GetOriginX(); }
	int32 OriginSpaceToWorldSpaceY(int32 InY) { return InY + GetOriginY(); }

	ABlockActor* GetBlock(int32 X, int32 Y) const;

	void DebugDraw() const;

private:

	float GridCellSize;

	// Grid count. Inclusive
	int32 GridX;
	int32 GridY;

	UPROPERTY()
	TArray<ABlockActor*> BlockActors;
};
