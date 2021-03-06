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

	virtual void Tick(float DeltaSeconds) override;

	bool FindPath(const FVector& StartLocation, const FVector& TargetLocation, TArray<FVector2D>& OutPath);

	bool IsValidLocation(const FVector& Location) const;
	bool IsValidGridLocation(const FIntPoint& GridLocation) const;

	void DebugDraw() const;

private:
	void UpdateGraph();

	TUniquePtr<FSideScrollGraph> Graph;
	TUniquePtr<MicroPanther::FMicroPather> MicroPather;
};
