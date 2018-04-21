#include "StarfoundGameMode.h"
#include "StarfoundCharacter.h"
#include "StarfoundSpectatorPawn.h"
#include "UObject/ConstructorHelpers.h"

AStarfoundGameMode::AStarfoundGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/SideScrollerCPP/Blueprints/SideScrollerCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	SpectatorClass = AStarfoundSpectatorPawn::StaticClass();

	PrimaryActorTick.bCanEverTick = true;
}

void AStarfoundGameMode::StartPlay()
{
	BlockActorScene = NewObject<UBlockActorScene>(this);
	GetWorld()->GetWorldSettings()->AddAssetUserData(BlockActorScene);
	BlockActorScene->InitializeGrid(100.0f, 100, 100);

	Super::StartPlay();

	BlockGenerator = NewObject<UBlockGenerator>();
	BlockGenerator->GenerateRandomBlockWorld(GetWorld(), BlockClasses);

	Navigation = GetWorld()->SpawnActor<ANavigation>();

	JobQueue = NewObject<UStarfoundJobQueue>(this);
	JobExecutor = NewObject<UStarfoundJobExecutor>(this);
}

void AStarfoundGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	BlockActorScene->DebugDraw();
	Navigation->DebugDraw();
	JobQueue->DebugDraw();
}

void UStarfoundJobQueue::AddJob(const FStarfoundJob& Job)
{
	JobQueue.Add(Job);
}

void UStarfoundJobQueue::AssignJob(AStarfoundPawn* Pawn)
{
	if (AssignedJobs.Find(Pawn))
	{
		ensure(0);
		return;
	}

	if (JobQueue.Num() == 0)
	{
		return;
	}

	FStarfoundJob Job = JobQueue[0];
	JobQueue.RemoveAt(0);

	AssignedJobs.Add(Pawn, Job);
}

void UStarfoundJobQueue::AssignAnotherJob(AStarfoundPawn* Pawn)
{
	const FStarfoundJob* OldJob = AssignedJobs.Find(Pawn);
	
	if (!OldJob)
	{
		AssignJob(Pawn);
		return;
	}

	FStarfoundJob OldJobCopy = *OldJob;

	AssignedJobs.Remove(Pawn);

	AssignJob(Pawn);

	JobQueue.Add(OldJobCopy);
}

bool UStarfoundJobQueue::GetAssignedJob(const AStarfoundPawn* Pawn, FStarfoundJob& OutJob)
{
	if (!Pawn)
	{
		return false;
	}

	const FStarfoundJob* Job = AssignedJobs.Find(Pawn);

	if (Job)
	{
		OutJob = *Job;
		return true;
	}

	return false;
}

float UStarfoundJobQueue::ProgressAssignedJob(const AStarfoundPawn* Pawn, float AddProgressPercentage)
{
	if (!ensure(Pawn))
	{
		return 0;
	}

	FStarfoundJob* Job = AssignedJobs.Find(Pawn);

	if (ensure(Job))
	{
		Job->ProgressPercentage += AddProgressPercentage;

		return Job->ProgressPercentage;
	}

	return 0;
}

void UStarfoundJobQueue::PopAssignedJob(const AStarfoundPawn* Pawn)
{
	AssignedJobs.Remove(Pawn);
}

void UStarfoundJobQueue::DebugDraw() const
{
	UBlockActorScene* BlockScene = GetBlockActorScene(GetWorld());

	if (!BlockScene)
	{
		return;
	}

	for (const FStarfoundJob& Job : JobQueue)
	{
		BlockScene->DebugDrawBoxAt(Job.Location, FColor::White);
	}

	for (auto&& Iter : AssignedJobs)
	{
		AStarfoundPawn* Pawn = Iter.Key;
		const FStarfoundJob& Job = Iter.Value;

		BlockScene->DebugDrawBoxAt(Job.Location, FColor::Green);
	}
}

void FStarfoundJob::InitConstruct(const FIntPoint& InLocation, const TSubclassOf<ABlockActor>& InConstructBlockClass)
{
	JobType = EStarfoundJobType::Construct;
	Location = InLocation;
	ConstructBlockClass = InConstructBlockClass;
}

void FStarfoundJob::InitDestruct(TWeakObjectPtr<class ABlockActor> Actor)
{
	JobType = EStarfoundJobType::Destruct;
	DestructBlockActor = Actor;

	UBlockActorScene* BlockScene = GetBlockActorScene(Actor->GetWorld());

	if (!BlockScene)
	{
		return;
	}

	Location = BlockScene->WorldSpaceToOriginSpaceGrid(Actor->GetActorLocation());
}

void UStarfoundJobExecutor::FinishJob(const FStarfoundJob& Job)
{
	switch (Job.JobType)
	{
	case EStarfoundJobType::Construct:
		HandleConstruct(Job);
		break;

	case EStarfoundJobType::Destruct:
		HandleDestruct(Job);
		break;

	default:
		ensure(0);
	}
}

void UStarfoundJobExecutor::HandleConstruct(const FStarfoundJob& Job)
{
	UBlockActorScene* BlockScene = GetBlockActorScene(GetWorld());

	if (!BlockScene)
	{
		return;
	}

	const FVector2D Location2D = BlockScene->OriginSpaceGridToWorldSpace2D(Job.Location);

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ABlockActor* NewBlockActor = GetWorld()->SpawnActor<ABlockActor>(
		Job.ConstructBlockClass, 
		FTransform(FVector(0, Location2D.X, Location2D.Y)), SpawnParameters);
}

void UStarfoundJobExecutor::HandleDestruct(const FStarfoundJob& Job)
{
	if (Job.DestructBlockActor.IsValid())
	{
		Job.DestructBlockActor->Destroy();
	}
}
