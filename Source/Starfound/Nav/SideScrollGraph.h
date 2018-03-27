#pragma once

#include "Micropather.h"

// void* state is defined as x + (y * Column Count)
class FSideScrollGraph : public MicroPanther::FGraph
{
public:
	FSideScrollGraph();

	void InitializeGrid(int32 InGridCountX, int32 InGridCountY);

	void SetHeight(int32 X, int32 Y, int32 NewHeight);

	int32 GetGridCountX() const { return GridCountX; }
	int32 GetGridCountY() const { return GridCountY; }
	int32 GetHeight(int32 X, int32 Y) const;

	virtual float LeastCostEstimate(void* StartState, void* EndState) override;
	virtual void AdjacentCost(void* State, TArray<MicroPanther::FStateCost>* AdjacentCosts) override;
	virtual void PrintStateInfo(void* State) override;

	FIntPoint StateToVec2(void* State) const;
	void* Vec2ToState(const FIntPoint& Position) const;

private:

	int32 GridCountX;
	int32 GridCountY;

	// Heights of each grid cell. index = X + (Y * GridCountX). -1 means blocked.
	TArray<int32> Heights;
};
