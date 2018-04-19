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

	UPROPERTY()
	EStarfoundJobType JobType;

	UPROPERTY()
	FIntPoint Location;

	// Construct
	UPROPERTY()
	TSubclassOf<ABlockActor> ConstructBlockClass;

	void InitConstruct(const FIntPoint& InLocation, const TSubclassOf<ABlockActor>& InConstructBlockClass);
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

	void AssignJob(AStarfoundPawn* Pawn);

	UFUNCTION(BlueprintCallable)
	bool GetAssignedJob(const AStarfoundPawn* Pawn, FStarfoundJob& OutJob);

	UFUNCTION(BlueprintCallable)
	void PopAssignedJob(const AStarfoundPawn* Pawn);

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

	void ExecuteJob(const FStarfoundJob& Job);

private:
	void HandleConstruct(const FStarfoundJob& Job);
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

private:

	UPROPERTY(EditDefaultsOnly)
	TArray<TSubclassOf<ABlockActor>> BlockClasses;

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
