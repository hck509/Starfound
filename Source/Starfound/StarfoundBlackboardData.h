#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BlackboardData.h"
#include "ItemActor.h"
#include "StarfoundBlackboardData.generated.h"

UCLASS(BlueprintType)
class STARFOUND_API UStarfoundBlackboardData : public UBlackboardData
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	AItemActor* GatherTargetItemActor;
};
