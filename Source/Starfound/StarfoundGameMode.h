// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BlockActor.h"
#include "StarfoundPawn.h"
#include "Nav/Navigation.h"
#include "StarfoundGameMode.generated.h"

UENUM(BlueprintType)
enum class EStarfoundJobType : uint8
{
	Construct,
	Destruct
};

USTRUCT(BlueprintType)
struct FStarfoundJob
{
	GENERATED_BODY()

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
	TWeakObjectPtr<class ABlockActor> DestructBlockActor;

	FStarfoundJob();

	void InitConstruct(const FIntPoint& InLocation, const TSubclassOf<ABlockActor>& InConstructBlockClass);
	void InitDestruct(TWeakObjectPtr<class ABlockActor> Actor);
};

UCLASS(BlueprintType)
class UStarfoundJobQueue : public UObject
{
public:
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable)
	const TArray<FStarfoundJob>& GetJobQueue() const { return JobQueue; }

	UFUNCTION(BlueprintCallable)
	void AddJob(const FStarfoundJob& Job);

	UFUNCTION(BlueprintCallable)
	bool AssignJob(AStarfoundPawn* Pawn);

	void AssignAnotherJob(AStarfoundPawn* Pawn);

	UFUNCTION(BlueprintCallable)
	bool GetAssignedJob(const AStarfoundPawn* Pawn, FStarfoundJob& OutJob);

	float ProgressAssignedJob(const AStarfoundPawn* Pawn, float AddProgressPercentage);

	UFUNCTION(BlueprintCallable)
	void PopAssignedJob(const AStarfoundPawn* Pawn);

	void DebugDraw() const;

private:
	TArray<FStarfoundJob> JobQueue;

	UPROPERTY()
	TMap<AStarfoundPawn*, FStarfoundJob> AssignedJobs;
};

UCLASS(BlueprintType)
class UStarfoundJobExecutor : public UObject
{
public:
	GENERATED_BODY()

	void FinishJob(const FStarfoundJob& Job);

private:
	void HandleConstruct(const FStarfoundJob& Job);
	void HandleDestruct(const FStarfoundJob& Job);
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
