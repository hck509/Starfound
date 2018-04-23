// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "StarfoundBTTaskNode.generated.h"

UCLASS()
class STARFOUND_API UPickupItemActorBTTaskNode : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UPickupItemActorBTTaskNode();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
