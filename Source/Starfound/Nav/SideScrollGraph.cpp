#include "SideScrollGraph.h"

FSideScrollGraph::FSideScrollGraph()
{
	GridCountX = 0;
	GridCountY = 0;
}

void FSideScrollGraph::InitializeGrid(int32 InGridCountX, int32 InGridCountY)
{
	GridCountX = InGridCountX;
	GridCountY = InGridCountY;

	Heights.Init(0, GridCountX * GridCountY);
}

void FSideScrollGraph::SetHeight(int32 X, int32 Y, int32 NewHeight)
{
	if (X < 0 || X > GridCountX || Y < 0 || Y > GridCountY)
	{
		ensure(0);
		return;
	}

	Heights[X + (Y * GridCountX)] = NewHeight;
}

float FSideScrollGraph::LeastCostEstimate(void* StartState, void* EndState)
{
	FIntPoint StartPosition = StateToVec2(StartState);
	FIntPoint EndPosition = StateToVec2(EndState);

	// TODO : need square root here, but we are trying not to use floating point. so...
	// WARN : without square root, comparison can be wrong. (a + b)^2 != a^2 + b^2
	int32 Cost = (StartPosition - EndPosition).SizeSquared();

	return Cost;
}

void FSideScrollGraph::AdjacentCost(void* State, TArray<MicroPanther::FStateCost>* AdjacentCosts)
{
	//const int32 AdjacentX[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };
	//const int32 AdjacentY[8] = { 0, 1, 1, 1, 0, -1, -1, -1 };
	//const int32 Costs[8] = { 1, 2, 1, 2, 1, 2, 1, 2 };

	const int32 AdjacentX[]		= { 1, 0, -1, 0 };
	const int32 AdjacentY[]		= { 0, 1, 0, -1 };
	const int32 Costs[]			= { 1, 1, 1, 1 };

	FIntPoint Position = StateToVec2(State);

	for (int i = 0; i < ARRAY_COUNT(Costs); ++i)
	{
		FIntPoint AdjacentPosition(Position.X + AdjacentX[i], Position.Y + AdjacentY[i]);

		int Height = GetHeight(AdjacentPosition.X, AdjacentPosition.Y);

		if (Height != -1)
		{
			// Not Blocked
			void* AdjacentState = Vec2ToState(AdjacentPosition);

			MicroPanther::FStateCost Cost = { AdjacentState, Costs[i] };

			AdjacentCosts->Add(Cost);
		}
	}
}

void FSideScrollGraph::PrintStateInfo(void* /*State*/)
{
	// TODO : ...
}

FIntPoint FSideScrollGraph::StateToVec2(void* State) const
{
	if (GridCountX == 0)
	{
		ensure(0);
		return FIntPoint(0, 0);
	}

	intptr_t Index = (intptr_t)State;

	return FIntPoint(Index % GridCountX, Index / GridCountX);
}

void* FSideScrollGraph::Vec2ToState(const FIntPoint& Position) const
{
	intptr_t Index = Position.X + (Position.Y * GridCountX);

	return (void*)Index;
}

int32 FSideScrollGraph::GetHeight(int32 X, int32 Y) const
{
	if (X < 0 || X >= GridCountX)
	{
		return -1;
	}

	if (Y < 0 || Y >= GridCountY)
	{
		return -1;
	}

	return Heights[X + (Y * GridCountX)];
}
