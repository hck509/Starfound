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
	BlockGenerator->GenerateRandomBlockWorld(GetWorld(), Configuration.ScenaryBlocks);

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

UStarfoundJobQueue::UStarfoundJobQueue()
	: NextJobId(0)
{

}

int32 UStarfoundJobQueue::AddJob(const FStarfoundJob& Job)
{
	JobQueue.Add(Job);

	JobQueue.Last().JobId = NextJobId;

	++NextJobId;

	return JobQueue.Last().JobId;
}

bool UStarfoundJobQueue::RemoveJob(int32 JobId)
{
	for (int i = 0; i < JobQueue.Num(); ++i)
	{
		if (JobQueue[i].JobId == JobId)
		{
			JobQueue.RemoveAt(i);
			return true;
		}
	}

	for (auto&& Iter : AssignedJobs)
	{
		if (Iter.Value.JobId == JobId)
		{
			AssignedJobs.Remove(Iter.Key);
			return true;
		}
	}

	return false;
}

bool UStarfoundJobQueue::AssignJob(AStarfoundPawn* Pawn)
{
	if (!ensure(Pawn))
	{
		return false;
	}

	if (AssignedJobs.Find(Pawn))
	{
		ensure(0);
		return false;
	}

	if (JobQueue.Num() == 0)
	{
		return false;
	}

	FStarfoundJob Job = JobQueue[0];
	JobQueue.RemoveAt(0);

	AssignedJobs.Add(Pawn, Job);

	return true;
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
	if (!ensure(Pawn))
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

FStarfoundJob::FStarfoundJob() 
	: ProgressPercentage(0)
{

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

void FStarfoundJob::InitGather(ABlockActor* Actor, EItemType ItemType)
{
	JobType = EStarfoundJobType::GatherItem;
	GatherItemType = ItemType;
	GatherTargetBlockActor = Actor;

	UBlockActorScene* BlockScene = GetBlockActorScene(Actor->GetWorld());

	if (!BlockScene)
	{
		return;
	}

	Location = BlockScene->WorldSpaceToOriginSpaceGrid(Actor->GetActorLocation());
}

void UStarfoundJobExecutor::FinishJob(AStarfoundPawn* Pawn, const FStarfoundJob& Job)
{
	switch (Job.JobType)
	{
	case EStarfoundJobType::Construct:
		HandleConstruct(Pawn, Job);
		break;

	case EStarfoundJobType::Destruct:
		HandleDestruct(Pawn, Job);
		break;

	case EStarfoundJobType::GatherItem:
		HandleGather(Pawn, Job);
		break;

	default:
		ensure(0);
	}
}

void UStarfoundJobExecutor::HandleConstruct(AStarfoundPawn* Pawn, const FStarfoundJob& Job)
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

void UStarfoundJobExecutor::HandleDestruct(AStarfoundPawn* Pawn, const FStarfoundJob& Job)
{
	if (Job.DestructBlockActor.IsValid() && !Job.DestructBlockActor->IsPendingKillPending())
	{
		Job.DestructBlockActor->OnBlockDestructed();
		Job.DestructBlockActor->Destroy();
	}
}

void UStarfoundJobExecutor::HandleGather(AStarfoundPawn* Pawn, const FStarfoundJob& Job)
{
	if (!Job.GatherTargetBlockActor.IsValid())
	{
		return;
	}

	int32* NumItems = Pawn->GetInventory().Items.Find(Job.GatherItemType);

	if (!NumItems || *NumItems <= 0)
	{
		ensure(0);
		return;
	}

	--(*NumItems);

	UStorageComponent* Storage = Job.GatherTargetBlockActor->FindComponentByClass<UStorageComponent>();
	if (ensure(Storage))
	{
		Storage->AddItem(Job.GatherItemType);
	}
}

AStarfoundGameMode* GetStarfoundGameMode(UWorld* World)
{
	if (!World)
	{
		return nullptr;
	}

	return Cast<AStarfoundGameMode>(World->GetAuthGameMode());
}
