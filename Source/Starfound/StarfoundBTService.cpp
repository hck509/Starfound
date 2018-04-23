#include "StarfoundBTService.h"
#include "AIController.h"
#include "StarfoundPawn.h"
#include "StarfoundGameMode.h"
#include "ItemActor.h"
#include "EngineUtils.h"
#include "BehaviorTree/BlackboardComponent.h"

UUpdateGatherTargetItemActorBTService::UUpdateGatherTargetItemActorBTService()
{
}

void UUpdateGatherTargetItemActorBTService::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* Controller = Cast<AAIController>(OwnerComp.GetOwner());

	if (!ensure(Controller))
	{
		return;
	}

	AStarfoundPawn* Pawn = Cast<AStarfoundPawn>(Controller->GetPawn());

	if (!ensure(Pawn))
	{
		return;
	}

	AStarfoundGameMode* GameMode = GetStarfoundGameMode(GetWorld());

	if (!ensure(GameMode))
	{
		return;
	}

	FStarfoundJob Job;

	const bool bHasJob = GameMode->GetJobQueue()->GetAssignedJob(Pawn, Job);

	if (!bHasJob || Job.JobType != EStarfoundJobType::GatherItem)
	{
		return;
	}

	for (TActorIterator<AItemActor> It(GetWorld()); It; ++It)
	{
		AItemActor* ItemActor = *It;

		if (ItemActor->GetItemType() == Job.GatherItemType)
		{
			Controller->GetBlackboardComponent()->SetValueAsObject(FName(TEXT("GatherTargetItemActor")), ItemActor);
			break;
		}
	}
}
