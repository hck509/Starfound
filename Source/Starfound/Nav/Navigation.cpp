#include "Navigation.h"

ANavigation::ANavigation()
{
	Graph.Reset(new FSideScrollGraph);
	MicroPather.Reset(new MicroPanther::MicroPather(Graph.Get(), 250, 6, false));
}
