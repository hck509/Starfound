#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "StarfoundBTService.generated.h"

/**
 * Update Gather target item actor
 */
UCLASS()
class STARFOUND_API UUpdateGatherTargetItemActorBTService : public UBTService
{
	GENERATED_BODY()
	
public:
	UUpdateGatherTargetItemActorBTService();

	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};

