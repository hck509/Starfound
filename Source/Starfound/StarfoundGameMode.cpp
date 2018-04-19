#include "StarfoundGameMode.h"
#include "StarfoundCharacter.h"
#include "UObject/ConstructorHelpers.h"

AStarfoundGameMode::AStarfoundGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/SideScrollerCPP/Blueprints/SideScrollerCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	PrimaryActorTick.bCanEverTick = true;
}

void AStarfoundGameMode::StartPlay()
{
	BlockActorScene = NewObject<UBlockActorScene>();
	GetWorld()->GetWorldSettings()->AddAssetUserData(BlockActorScene);
	BlockActorScene->InitializeGrid(100.0f, 100, 100);

	Super::StartPlay();

	BlockGenerator = NewObject<UBlockGenerator>();
	BlockGenerator->GenerateRandomBlockWorld(GetWorld(), BlockClasses);

	Navigation = GetWorld()->SpawnActor<ANavigation>();

	JobQueue = NewObject<UStarfoundJobQueue>();
	JobExecutor = NewObject<UStarfoundJobExecutor>();
}

void AStarfoundGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	BlockActorScene->DebugDraw();
	Navigation->DebugDraw();
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

	FStarfoundJob Job = JobQueue.Pop();

	AssignedJobs.Add(Pawn, Job);
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

void FStarfoundJob::InitConstruct(const FIntPoint& InLocation, const TSubclassOf<ABlockActor>& InConstructBlockClass)
{
	JobType = EStarfoundJobType::Construct;
	Location = InLocation;
	ConstructBlockClass = InConstructBlockClass;
}

void UStarfoundJobExecutor::HandleConstruct(const FStarfoundJob& Job)
{
	UBlockActorScene* BlockScene = GetWorld() ? Cast<UBlockActorScene>(GetWorld()->GetWorldSettings()->GetAssetUserDataOfClass(UBlockActorScene::StaticClass())) : nullptr;

	if (!BlockScene)
	{
		return;
	}

	const FVector2D Location2D = BlockScene->OriginSpaceGridToWorldSpace(Job.Location);

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ABlockActor* NewBlockActor = GetWorld()->SpawnActor<ABlockActor>(
		Job.ConstructBlockClass, 
		FTransform(FVector(0, Location2D.X, Location2D.Y)), SpawnParameters);
}
