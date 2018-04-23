#include "StarfoundBTTaskNode.h"
#include "StarfoundAIController.h"
#include "StarfoundPawn.h"
#include "StarfoundGameMode.h"
#include "BehaviorTree/BlackboardComponent.h"

UPickupItemActorBTTaskNode::UPickupItemActorBTTaskNode()
{
	bNotifyTick = true;
}

EBTNodeResult::Type UPickupItemActorBTTaskNode::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	return EBTNodeResult::InProgress;
}

void UPickupItemActorBTTaskNode::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AStarfoundAIController* Controller = Cast<AStarfoundAIController>(OwnerComp.GetOwner());

	if (!ensure(Controller))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	AItemActor* ItemActor = Cast<AItemActor>(Controller->GetBlackboardComponent()->GetValueAsObject(FName(TEXT("GatherTargetItemActor"))));

	if (!ensure(ItemActor))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	AStarfoundPawn* Pawn = Cast<AStarfoundPawn>(Controller->GetPawn());

	if (!ensure(Pawn))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	AStarfoundGameMode* GameMode = GetStarfoundGameMode(GetWorld());

	if (!ensure(GameMode))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	FStarfoundJob Job;

	const bool bHasJob = GameMode->GetJobQueue()->GetAssignedJob(Pawn, Job);

	if (!ensure(bHasJob))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	const bool bItemInPickupRange = Pawn->IsItemActorInRangeToPickup(ItemActor);

	if (!bItemInPickupRange)
	{
		const bool bCanMoveTo = Controller->MoveToPickupItem(ItemActor);

		if (!bCanMoveTo)
		{
			FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
			return;
		}
	}
	else
	{
		const bool bPickupSuccess = Pawn->PickupItemActor(ItemActor);

		if (bPickupSuccess)
		{
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		}
		else
		{
			FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		}
	}
}
