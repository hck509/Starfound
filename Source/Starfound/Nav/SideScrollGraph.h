#pragma once

#include "Micropather.h"

// void* state is defined as x + (y * Column Count)
class FSideScrollGraph : public MicroPanther::Graph
{
public:
	FSideScrollGraph();

	// Heights of each grid cell. index = X + (Y * GridCountX). -1 means blocked.
	void SetHeights(const TArray<int32>& NewHeights, int32 NewGridCountX, int32 NewGridCountY) { Heights = NewHeights; GridCountX = NewGridCountX; GridCountY = NewGridCountY; }

	virtual float LeastCostEstimate(void* StartState, void* EndState) override;
	virtual void AdjacentCost(void* State, MP_VECTOR<MicroPanther::StateCost>* AdjacentCosts) override;
	virtual void PrintStateInfo(void* State) override;

	FIntPoint StateToVec2(void* State) const;
	void* Vec2ToState(const FIntPoint& Position) const;

private:
	int GetHeight(int32 X, int32 Y) const;

	int32 GridCountX;
	int32 GridCountY;

	// Heights of each grid cell. index = X + (Y * GridCountX). -1 means blocked.
	TArray<int32> Heights;
};
