#include "StarfoundBTDecorator.h"
#include "StarfoundAIController.h"
#include "StarfoundPawn.h"
#include "StarfoundGameMode.h"


bool UPawnHasGatherItemBTDecorator::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	AStarfoundAIController* Controller = Cast<AStarfoundAIController>(OwnerComp.GetAIOwner());

	if (!Controller)
	{
		return false;
	}

	AStarfoundPawn* Pawn = Cast<AStarfoundPawn>(Controller->GetPawn());

	if (!Pawn)
	{
		return false;
	}

	AStarfoundGameMode* GameMode = GetStarfoundGameMode(GetWorld());

	FStarfoundJob Job;
	const bool bHasJob = GameMode->GetJobQueue()->GetAssignedJob(Pawn, Job);

	if (!bHasJob || Job.JobType != EStarfoundJobType::GatherItem)
	{
		ensure(0);
		return false;
	}

	if (Job.GatherItemType == EItemType::None)
	{
		if (Pawn->GetInventory().Items.Num() > 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		bool bHasItem = Pawn->GetInventory().Items.Contains(Job.GatherItemType);

		if (bHasItem)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}
