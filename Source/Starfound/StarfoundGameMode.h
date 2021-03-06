// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BlockActor.h"
#include "ItemActor.h"
#include "StarfoundPawn.h"
#include "Nav/Navigation.h"
#include "StarfoundGameMode.generated.h"

UENUM(BlueprintType)
enum class EStarfoundJobType : uint8
{
	None,
	Construct,
	Destruct,
	GatherItem
};

USTRUCT(BlueprintType)
struct FStarfoundJob
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 JobId;

	UPROPERTY(BlueprintReadOnly)
	EStarfoundJobType JobType;

	UPROPERTY(BlueprintReadOnly)
	FIntPoint Location;

	UPROPERTY(BlueprintReadOnly)
	float ProgressPercentage;

	// Construct
	UPROPERTY(BlueprintReadOnly)
	TSubclassOf<ABlockActor> ConstructBlockClass;

	// Destruct
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<ABlockActor> DestructBlockActor;

	// Gather
	UPROPERTY(BlueprintReadOnly)
	EItemType GatherItemType;	// None item type means everything

	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<ABlockActor> GatherTargetBlockActor;

	FStarfoundJob();

	void InitConstruct(const FIntPoint& InLocation, const TSubclassOf<ABlockActor>& InConstructBlockClass);
	void InitDestruct(TWeakObjectPtr<class ABlockActor> Actor);
	void InitGather(ABlockActor* Actor, EItemType ItemType);
};

UCLASS(BlueprintType)
class UStarfoundJobQueue : public UObject
{
public:
	GENERATED_BODY()

	UStarfoundJobQueue();

	UFUNCTION(BlueprintCallable)
	const TArray<FStarfoundJob>& GetJobQueue() const { return JobQueue; }

	UFUNCTION(BlueprintCallable)
	int32 AddJob(const FStarfoundJob& Job);

	UFUNCTION(BlueprintCallable)
	bool RemoveJob(int32 JobId);

	UFUNCTION(BlueprintCallable)
	bool AssignJob(AStarfoundPawn* Pawn);

	UFUNCTION(BlueprintCallable)
	void AssignAnotherJob(AStarfoundPawn* Pawn);

	UFUNCTION(BlueprintCallable)
	bool GetAssignedJob(const AStarfoundPawn* Pawn, FStarfoundJob& OutJob);

	float ProgressAssignedJob(const AStarfoundPawn* Pawn, float AddProgressPercentage);

	UFUNCTION(BlueprintCallable)
	void PopAssignedJob(const AStarfoundPawn* Pawn);

	void DebugDraw() const;

private:
	int32 NextJobId;

	TArray<FStarfoundJob> JobQueue;

	UPROPERTY()
	TMap<AStarfoundPawn*, FStarfoundJob> AssignedJobs;
};

UCLASS(BlueprintType)
class UStarfoundJobExecutor : public UObject
{
public:
	GENERATED_BODY()

	void FinishJob(AStarfoundPawn* Pawn, const FStarfoundJob& Job);

private:
	void HandleConstruct(AStarfoundPawn* Pawn, const FStarfoundJob& Job);
	void HandleDestruct(AStarfoundPawn* Pawn, const FStarfoundJob& Job);
	void HandleGather(AStarfoundPawn* Pawn, const FStarfoundJob& Job);
};

USTRUCT(BlueprintType)
struct FStarfoundConfiguration
{
	GENERATED_BODY()

	// Blocks that auto generate at beginning
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<TSubclassOf<ABlockActor>> ScenaryBlocks;

	// Blocks that player can construct
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<TSubclassOf<ABlockActor>> ConstructableBlocks;
};

UCLASS(minimalapi)
class AStarfoundGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AStarfoundGameMode();

	virtual void StartPlay() override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	ANavigation* GetNavigation() const { return Navigation; }

	UFUNCTION(BlueprintCallable)
	UStarfoundJobQueue* GetJobQueue() const { return JobQueue; }

	UFUNCTION(BlueprintCallable)
	UStarfoundJobExecutor* GetJobExecutor() const { return JobExecutor; }

	UFUNCTION(BlueprintCallable)
	const FStarfoundConfiguration& GetConfiguration() const { return Configuration; }

private:

	UPROPERTY(EditDefaultsOnly)
	FStarfoundConfiguration Configuration;

	UPROPERTY(Transient)
	UBlockGenerator* BlockGenerator;

	UPROPERTY(Transient)
	UBlockActorScene* BlockActorScene;

	UPROPERTY(Transient)
	ANavigation* Navigation;

	UPROPERTY(Transient)
	UStarfoundJobQueue* JobQueue;

	UPROPERTY(Transient)
	UStarfoundJobExecutor* JobExecutor;
};

AStarfoundGameMode* GetStarfoundGameMode(UWorld* World);
