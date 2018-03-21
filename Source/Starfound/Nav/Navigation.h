#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SideScrollGraph.h"
#include "Micropather.h"
#include "Navigation.generated.h"

UCLASS()
class ANavigation : public AActor
{
	GENERATED_BODY()

public:
	ANavigation();

private:
	TUniquePtr<FSideScrollGraph> Graph;
	TUniquePtr<MicroPanther::MicroPather> MicroPather;
};
