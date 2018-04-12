#include "Micropather.h"

/*
Copyright (c) 2000-2009 Lee Thomason (www.grinninglizard.com)

Grinning Lizard Utilities.

This software is provided 'as-is', without any express or implied 
warranty. In no event will the authors be held liable for any 
damages arising from the use of this software.

Permission is granted to anyone to use this software for any 
purpose, including commercial applications, and to alter it and 
redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must 
not claim that you wrote the original software. If you use this 
software in a product, an acknowledgment in the product documentation 
would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and 
must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source 
distribution.
*/

/*
Modified to fit in Unreal Engine4 by hck509@gmail.com
*/


#ifdef _MSC_VER
#pragma warning( disable : 4786 )	// Debugger truncating names.
#pragma warning( disable : 4530 )	// Exception handler isn't used
#endif

#include <memory.h>
#include <stdio.h>

//#define DEBUG_PATH
//#define DEBUG_PATH_DEEP
//#define TRACK_COLLISION
//#define DEBUG_CACHING

#ifdef DEBUG_CACHING
#include "../grinliz/gldebug.h"
#endif


using namespace MicroPanther;

class FOpenQueue
{
public:
	FOpenQueue(FGraph* InGraph)
	{
		Graph = InGraph;
		SentinelPathNode = (FPathNode*)SentinelMem;
		SentinelPathNode->InitSentinel();
#ifdef DEBUG
		SentinelPathNode->CheckList();
#endif
	}
	~FOpenQueue() {}

	void Push(FPathNode* Node);
	FPathNode* Pop();

	void Update(FPathNode* Node);

	bool IsEmpty() const { return SentinelPathNode->Next == SentinelPathNode; }

	FOpenQueue(const FOpenQueue&) = delete;
	void operator=(const FOpenQueue&) = delete;

private:
	FPathNode* SentinelPathNode;
	int SentinelMem[(sizeof(FPathNode) + sizeof(int)) / sizeof(int)];

	// for debugging
	FGraph* Graph;
};

void FOpenQueue::Push(FPathNode* Node)
{
	MPASSERT(Node->bInOpen == 0);
	MPASSERT(Node->bInClosed == 0);

#ifdef DEBUG_PATH_DEEP
	printf("Open Push: ");
	Graph->PrintStateInfo(Node->State);
	printf(" total=%.1f\n", Node->TotalCost);
#endif

	// Add sorted. Lowest to highest cost path. Note that the sentinel has
	// a value of FLT_MAX, so it should always be sorted in.
	MPASSERT(Node->TotalCost < FLT_MAX);
	FPathNode* IterNode = SentinelPathNode->Next;
	while (true)
	{
		if (Node->TotalCost < IterNode->TotalCost)
		{
			IterNode->AddBefore(Node);
			Node->bInOpen = 1;
			break;
		}
		IterNode = IterNode->Next;
	}
	MPASSERT(Node->bInOpen);	// make sure this was actually added.
#ifdef DEBUG
	SentinelPathNode->CheckList();
#endif
}

FPathNode* FOpenQueue::Pop()
{
	MPASSERT(SentinelPathNode->Next != SentinelPathNode);
	FPathNode* Node = SentinelPathNode->Next;
	Node->Unlink();
#ifdef DEBUG
	SentinelPathNode->CheckList();
#endif

	MPASSERT(Node->bInClosed == 0);
	MPASSERT(Node->bInOpen == 1);
	Node->bInOpen = 0;

#ifdef DEBUG_PATH_DEEP
	printf("Open Pop: ");
	Graph->PrintStateInfo(Node->State);
	printf(" total=%.1f\n", Node->TotalCost);
#endif

	return Node;
}

void FOpenQueue::Update(FPathNode* Node)
{
#ifdef DEBUG_PATH_DEEP
	printf( "Open Update: " );		
	Graph->PrintStateInfo( Node->State );
	printf( " total=%.1f\n", Node->TotalCost );		
#endif

	MPASSERT(Node->bInOpen);

	// If the node now cost less than the one before it,
	// move it to the front of the list.
	if (Node->Prev != SentinelPathNode && Node->TotalCost < Node->Prev->TotalCost)
	{
		Node->Unlink();
		SentinelPathNode->Next->AddBefore(Node);
	}

	// If the node is too high, move to the right.
	if (Node->TotalCost > Node->Next->TotalCost)
	{
		FPathNode* IterNode = Node->Next;
		Node->Unlink();

		while (Node->TotalCost > IterNode->TotalCost)
		{
			IterNode = IterNode->Next;
		}

		IterNode->AddBefore(Node);
#ifdef DEBUG
		SentinelPathNode->CheckList();
#endif
	}
}


class FClosedSet
{
public:
	FClosedSet(FGraph* InGraph) { this->Graph = InGraph; }

	FClosedSet(const FClosedSet&) = delete;
	void operator=(const FClosedSet&) = delete;

	void Add(FPathNode* Node)
	{
#ifdef DEBUG_PATH_DEEP
		printf("Closed add: ");
		Graph->PrintStateInfo(Node->State);
		printf(" total=%.1f\n", Node->TotalCost);
#endif
#ifdef DEBUG
		MPASSERT(Node->bInClosed == 0);
		MPASSERT(Node->bInOpen == 0);
#endif
		Node->bInClosed = 1;
	}

	void Remove(FPathNode* Node)
	{
#ifdef DEBUG_PATH_DEEP
		printf("Closed remove: ");
		Graph->PrintStateInfo(Node->State);
		printf(" total=%.1f\n", Node->TotalCost);
#endif
		MPASSERT(Node->bInClosed == 1);
		MPASSERT(Node->bInOpen == 0);

		Node->bInClosed = 0;
	}

private:
	FGraph* Graph;
};


FPathNodePool::FPathNodePool(unsigned InNumNodesPerBlock, unsigned _typicalAdjacent)
	: FirstBlock(0)
	, Blocks(0)
#if defined( MICROPATHER_STRESS )
	, NumNodesPerBlock(32)
#else
	, NumNodesPerBlock(InNumNodesPerBlock)
#endif
	, NumTotalAllocatedNodes(0)
	, NumAvailableNodesOnLastBlock(0)
{
	FreeMemSentinel.InitSentinel();

	CacheCap = NumNodesPerBlock * _typicalAdjacent;
	CacheSize = 0;
	Cache = (FNodeCost*)malloc(CacheCap * sizeof(FNodeCost));

	// Want the behavior that if the actual number of states is specified, the cache 
	// will be at least that big.
	HashShift = 3;	// 8 (only useful for stress testing) 
#if !defined( MICROPATHER_STRESS )
	while (HashSize() < NumNodesPerBlock)
		++HashShift;
#endif
	HashTable = (FPathNode**)calloc(HashSize(), sizeof(FPathNode*));

	Blocks = FirstBlock = AllocBlock();
	NumHashCollision = 0;

	//printf( "HashSize=%d allocate=%d\n", HashSize(), allocate );
}


FPathNodePool::~FPathNodePool()
{
	Clear();
	free(FirstBlock);
	free(Cache);
	free(HashTable);
#ifdef TRACK_COLLISION
	printf("Total collide=%d HashSize=%d HashShift=%d\n", NumHashCollision, HashSize(), HashShift);
#endif
}


bool FPathNodePool::PushCache(const TArray<FNodeCost>& Nodes, int32* OutStartIndex)
{
	*OutStartIndex = -1;
	
	if (Nodes.Num() + CacheSize <= CacheCap)
	{
		for (int32 i = 0; i < Nodes.Num(); ++i)
		{
			Cache[i + CacheSize] = Nodes[i];
		}

		*OutStartIndex = CacheSize;
		CacheSize += Nodes.Num();

		return true;
	}

	return false;
}


void FPathNodePool::GetCache(int32 CacheIndex, int32 NumNodeCosts, TArray<FNodeCost>& OutNodeCosts)
{
	MPASSERT(CacheIndex >= 0 && CacheIndex < CacheCap);
	MPASSERT(NumNodeCosts > 0);
	MPASSERT(CacheIndex + NumNodeCosts <= CacheCap);

	OutNodeCosts.SetNum(NumNodeCosts);

	memcpy(OutNodeCosts.GetData(), &Cache[CacheIndex], sizeof(FNodeCost) * NumNodeCosts);
}


void FPathNodePool::Clear()
{
#ifdef TRACK_COLLISION
	// Collision tracking code.
	int NumNewCollision = 0;
	for (unsigned i = 0; i < HashSize(); ++i)
	{
		if (HashTable[i] && (HashTable[i]->Child[0] || HashTable[i]->Child[1]))
		{
			++NumNewCollision;
		}
	}
	//printf( "PathNodePool %d/%d collision=%d %.1f%%\n", nAllocated, HashSize(), collide, 100.0f*(float)collide/(float)HashSize() );
	NumHashCollision += NumNewCollision;
#endif

	FBlock* IterBlock = Blocks;
	while (IterBlock)
	{
		FBlock* NextBlock = IterBlock->NextBlock;
		if (IterBlock != FirstBlock)
		{
			free(IterBlock);
		}
		IterBlock = NextBlock;
	}

	// Don't delete the first block (we always need at least that much memory.)
	Blocks = FirstBlock;	

	// Set up for new allocations (but don't do work we don't need to. Reset/Clear can be called frequently)
	if (NumTotalAllocatedNodes > 0)
	{
		FreeMemSentinel.Next = &FreeMemSentinel;
		FreeMemSentinel.Prev = &FreeMemSentinel;

		memset(HashTable, 0, sizeof(FPathNode*) * HashSize());

		for (uint32 i = 0; i < NumNodesPerBlock; ++i)
		{
			FreeMemSentinel.AddBefore(&FirstBlock->PathNode[i]);
		}
	}

	NumAvailableNodesOnLastBlock = NumNodesPerBlock;
	NumTotalAllocatedNodes = 0;
	CacheSize = 0;
}


FPathNodePool::FBlock* FPathNodePool::AllocBlock()
{
	FBlock* NewBlock = (FBlock*) calloc(1, sizeof(FBlock) + sizeof(FPathNode)*(NumNodesPerBlock - 1));
	NewBlock->NextBlock = 0;

	NumAvailableNodesOnLastBlock += NumNodesPerBlock;

	for (uint32 i = 0; i < NumNodesPerBlock; ++i)
	{
		FreeMemSentinel.AddBefore(&NewBlock->PathNode[i]);
	}

	return NewBlock;
}


uint32 FPathNodePool::Hash(void* voidval)
{
	/*
		Spent quite some time on this, and the result isn't quite satifactory. The
		input set is the size of a void*, and is generally (x,y) pairs or memory pointers.

		FNV resulting in about 45k collisions in a (large) test and some other approaches
		about the same.

		Simple folding reduces collisions to about 38k - big improvement. However, that may
		be an artifact of the (x,y) pairs being well distributed. And for either the x,y case 
		or the pointer case, there are probably very poor hash table sizes that cause "overlaps"
		and grouping. (An x,y encoding with a hashShift of 8 is begging for trouble.)

		The best tested results are simple folding, but that seems to beg for a pathelogical case.
		FNV-1a was the next best choice, without obvious pathelogical holes.

		Finally settled on h%HashMask(). Simple, but doesn't have the obvious collision cases of folding.
	*/
	/*
	// Time: 567
	// FNV-1a
	// http://isthe.com/chongo/tech/comp/fnv/
	// public domain.
	MP_UPTR val = (MP_UPTR)(voidval);
	const unsigned char *p = (unsigned char *)(&val);
	unsigned int h = 2166136261;

	for( size_t i=0; i<sizeof(MP_UPTR); ++i, ++p ) {
		h ^= *p;
		h *= 16777619;
	}
	// Fold the high bits to the low bits. Doesn't (generally) use all
	// the bits since the shift is usually < 16, but better than not
	// using the high bits at all.
	return ( h ^ (h>>hashShift) ^ (h>>(hashShift*2)) ^ (h>>(hashShift*3)) ) & HashMask();
	*/
	/*
	// Time: 526
	MP_UPTR h = (MP_UPTR)(voidval);
	return ( h ^ (h>>hashShift) ^ (h>>(hashShift*2)) ^ (h>>(hashShift*3)) ) & HashMask();
	*/

	// Time: 512
	// The HashMask() is used as the divisor. h%1024 has lots of common
	// repetitions, but h%1023 will move things out more.
	MP_UPTR h = (MP_UPTR)(voidval);
	return h % HashMask();	
}


FPathNode* FPathNodePool::Alloc()
{
	if (FreeMemSentinel.Next == &FreeMemSentinel)
	{
		MPASSERT(NumAvailableNodesOnLastBlock == 0);

		FBlock* NewBlock = AllocBlock();
		NewBlock->NextBlock = Blocks;
		Blocks = NewBlock;
		MPASSERT(FreeMemSentinel.Next != &FreeMemSentinel);
	}
	FPathNode* NewNode = FreeMemSentinel.Next;
	NewNode->Unlink();

	++NumTotalAllocatedNodes;
	MPASSERT(NumAvailableNodesOnLastBlock > 0);
	--NumAvailableNodesOnLastBlock;

	return NewNode;
}


void FPathNodePool::AddPathNode(uint32 HashKey, FPathNode* RootNode)
{
	if (HashTable[HashKey])
	{
		FPathNode* Node = HashTable[HashKey];

		while (true)
		{
			int dir = (RootNode->State < Node->State) ? 0 : 1;
			if (Node->Child[dir])
			{
				Node = Node->Child[dir];
			}
			else
			{
				Node->Child[dir] = RootNode;
				break;
			}
		}
	}
	else
	{
		HashTable[HashKey] = RootNode;
	}
}


FPathNode* FPathNodePool::FetchPathNode(void* State)
{
	uint32 HashKey = Hash(State);

	FPathNode* RootNode = HashTable[HashKey];
	while (RootNode)
	{
		if (RootNode->State == State)
		{
			break;
		}
		RootNode = (State < RootNode->State) ? RootNode->Child[0] : RootNode->Child[1];
	}

	MPASSERT(RootNode);

	return RootNode;
}


FPathNode* FPathNodePool::GetPathNode(uint32 Frame, void* State, float CostFromStart, float EstToGoal, FPathNode* ParentNode)
{
	uint32 HashKey = Hash(State);

	FPathNode* RootNode = HashTable[HashKey];
	while (RootNode)
	{
		if (RootNode->State == State)
		{
			if (RootNode->Frame == Frame)
			{
				// This is the correct state and correct frame.
				break;
			}

			// Correct state, wrong frame.
			RootNode->Init(Frame, State, CostFromStart, EstToGoal, ParentNode);

			break;
		}

		RootNode = (State < RootNode->State) ? RootNode->Child[0] : RootNode->Child[1];
	}

	if (!RootNode)
	{
		// allocate new one
		RootNode = Alloc();
		RootNode->Clear();
		RootNode->Init(Frame, State, CostFromStart, EstToGoal, ParentNode);
		AddPathNode(HashKey, RootNode);
	}

	return RootNode;
}


void FPathNode::Init(
	uint32 InFrame,
	void* InState,
	float InCostFromStart,
	float InEstToGoal,
	FPathNode* InParent)
{
	State = InState;
	CostFromStart = InCostFromStart;
	EstToGoal = InEstToGoal;
	CalcTotalCost();
	Parent = InParent;
	Frame = InFrame;
	bInOpen = 0;
	bInClosed = 0;
}


void FPathNode::Clear()
{
	memset(this, 0, sizeof(FPathNode));
	NumAdjacent = -1;
	CacheIndex = -1;
}

void MicroPanther::FPathNode::InitSentinel()
{
	Clear();
	Init(0, 0, FLT_MAX, FLT_MAX, 0);
	Prev = Next = this;
}

void MicroPanther::FPathNode::Unlink()
{
	Next->Prev = Prev;
	Prev->Next = Next;
	Next = Prev = 0;
}

void MicroPanther::FPathNode::AddBefore(FPathNode* Node)
{
	Node->Next = this;
	Node->Prev = Prev;
	Prev->Next = Node;
	Prev = Node;
}

void MicroPanther::FPathNode::CalcTotalCost()
{
	if (CostFromStart < FLT_MAX && EstToGoal < FLT_MAX)
	{
		TotalCost = CostFromStart + EstToGoal;
	}
	else
	{
		TotalCost = FLT_MAX;
	}
}

FMicroPather::FMicroPather(FGraph* InGraph, uint32 allocate, unsigned typicalAdjacent, bool cache)
	: PathNodePool(allocate, typicalAdjacent),
	Graph(InGraph),
	Frame(0)
{
	MPASSERT(allocate);
	MPASSERT(typicalAdjacent);
	PathCache = 0;
	if (cache) {
		PathCache = new FPathCache(allocate * 4);	// untuned arbitrary constant	
	}
}


FMicroPather::~FMicroPather()
{
	delete PathCache;
}


void FMicroPather::Reset()
{
	PathNodePool.Clear();
	if ( PathCache ) {
		PathCache->Reset();
	}
	Frame = 0;
}


void FMicroPather::GoalReached( FPathNode* node, void* start, void* end, TArray< void* > *_path )
{
	TArray< void* >& path = *_path;
	path.Empty();

	// We have reached the goal.
	// How long is the path? Used to allocate the vector which is returned.
	int count = 1;
	FPathNode* it = node;
	while( it->Parent )
	{
		++count;
		it = it->Parent;
	}

	// Now that the path has a known length, allocate
	// and fill the vector that will be returned.
	if ( count < 3 )
	{
		// Handle the short, special case.
		path.SetNum(2);
		path[0] = start;
		path[1] = end;
	}
	else
	{
		path.SetNum(count);

		path[0] = start;
		path[count-1] = end;
		count-=2;
		it = node->Parent;

		while ( it->Parent )
		{
			path[count] = it->State;
			it = it->Parent;
			--count;
		}
	}

	if ( PathCache ) {
		Costs.Empty();

		FPathNode* pn0 = PathNodePool.FetchPathNode( path[0] );
		FPathNode* pn1 = 0;
		for( int32 i=0; i<path.Num()-1; ++i ) {
			pn1 = PathNodePool.FetchPathNode( path[i+1] );
			NodeCosts.Empty();
			GetNodeNeighbors( pn0, &NodeCosts );
			for( int32 j=0; j<NodeCosts.Num(); ++j ) {
				if ( NodeCosts[j].Node == pn1 ) {
					Costs.Add( NodeCosts[j].Cost );
					break;
				}
			}
			MPASSERT( Costs.Num() == i+1 );
			pn0 = pn1;
		}
		PathCache->Add( path, Costs );
	}

	#ifdef DEBUG_PATH
	printf( "Path: " );
	int counter=0;
	#endif
	for ( int32 k=0; k<path.Num(); ++k )
	{
		#ifdef DEBUG_PATH
		Graph->PrintStateInfo( path[k] );
		printf( " " );
		++counter;
		if ( counter == 8 )
		{
			printf( "\n" );
			counter = 0;
		}
		#endif
	}
	#ifdef DEBUG_PATH
	printf( "Cost=%.1f Checksum %d\n", node->CostFromStart, checksum );
	#endif
}


void FMicroPather::GetNodeNeighbors( FPathNode* node, TArray< FNodeCost >* pNodeCost )
{
	if ( node->NumAdjacent == 0 ) {
		// it has no neighbors.
		pNodeCost->SetNum( 0 );
	}
	else if ( node->CacheIndex < 0 )
	{
		// Not in the cache. Either the first time or just didn't fit. We don't know
		// the number of neighbors and need to call back to the client.
		StateCosts.SetNum( 0 );
		Graph->AdjacentCost( node->State, &StateCosts );

		#ifdef DEBUG
		{
			// If this assert fires, you have passed a state
			// as its own neighbor state. This is impossible --
			// bad things will happen.
			for ( unsigned i=0; i<StateCosts.Num(); ++i )
				MPASSERT( StateCosts[i].State != node->State );
		}
		#endif

		pNodeCost->SetNum( StateCosts.Num() );
		node->NumAdjacent = StateCosts.Num();

		if ( node->NumAdjacent > 0 ) {
			// Now convert to pathNodes.
			// Note that the microsoft std library is actually pretty slow.
			// Move things to temp vars to help.
			const int32 stateCostVecSize = StateCosts.Num();
			const FStateCost* stateCostVecPtr = &StateCosts[0];
			FNodeCost* pNodeCostPtr = &(*pNodeCost)[0];

			for( int32 i=0; i<stateCostVecSize; ++i ) {
				void* state = stateCostVecPtr[i].State;
				pNodeCostPtr[i].Cost = stateCostVecPtr[i].Cost;
				pNodeCostPtr[i].Node = PathNodePool.GetPathNode( Frame, state, FLT_MAX, FLT_MAX, 0 );
			}

			// Can this be cached?
			int start = 0;
			if (pNodeCost->Num() > 0 && PathNodePool.PushCache(*pNodeCost, &start))
			{
				node->CacheIndex = start;
			}
		}
	}
	else {
		// In the cache!
		pNodeCost->SetNum( node->NumAdjacent );
		FNodeCost* pNodeCostPtr = &(*pNodeCost)[0];
		PathNodePool.GetCache(node->CacheIndex, node->NumAdjacent, *pNodeCost);

		// A node is uninitialized (even if memory is allocated) if it is from a previous frame.
		// Check for that, and Init() as necessary.
		for( int i=0; i<node->NumAdjacent; ++i ) {
			FPathNode* pNode = pNodeCostPtr[i].Node;
			if ( pNode->Frame != Frame ) {
				pNode->Init( Frame, pNode->State, FLT_MAX, FLT_MAX, 0 );
			}
		}
	}
}


#ifdef DEBUG
/*
void MicroPather::DumpStats()
{
	int hashTableEntries = 0;
	for( int i=0; i<HASH_SIZE; ++i )
		if ( hashTable[i] )
			++hashTableEntries;
	
	int pathNodeBlocks = 0;
	for( PathNode* node = pathNodeMem; node; node = node[ALLOCATE-1].left )
		++pathNodeBlocks;
	printf( "HashTableEntries=%d/%d PathNodeBlocks=%d [%dk] PathNodes=%d SolverCalled=%d\n",
			  hashTableEntries, HASH_SIZE, pathNodeBlocks, 
			  pathNodeBlocks*ALLOCATE*sizeof(PathNode)/1024,
			  pathNodeCount,
			  frame );
}
*/
#endif


void FMicroPather::StatesInPool( TArray< void* >* stateVec )
{
 	stateVec->Empty();
	PathNodePool.AllStates( Frame, stateVec );
}


void FPathNodePool::AllStates( unsigned frame, TArray< void* >* stateVec )
{	
    for ( FBlock* b=Blocks; b; b=b->NextBlock )
    {
    	for( unsigned i=0; i<NumNodesPerBlock; ++i )
    	{
    	    if ( b->PathNode[i].Frame == frame )
	    	    stateVec->Add( b->PathNode[i].State );
    	}    
	}           
}   


FPathCache::FPathCache( int _allocated )
{
	Items = new Item[_allocated];
	memset( Items, 0, sizeof(*Items)*_allocated );
	NumItemsAllocated = _allocated;
	NumItems = 0;
	NumHit = 0;
	NumMiss = 0;
}


FPathCache::~FPathCache()
{
	delete [] Items;
}


void FPathCache::Reset()
{
	if ( NumItems ) {
		memset( Items, 0, sizeof(*Items)*NumItemsAllocated );
		NumItems = 0;
		NumHit = 0;
		NumMiss = 0;
	}
}


void FPathCache::Add( const TArray< void* >& path, const TArray< float >& cost )
{
	if ( NumItems + path.Num() > NumItemsAllocated*3/4 ) {
		return;
	}

	for( int32 i=0; i<path.Num()-1; ++i ) {
		// example: a->b->c->d
		// Huge memory saving to only store 3 paths to 'd'
		// Can put more in cache with also adding path to b, c, & d
		// But uses much more memory. Experiment with this commented
		// in and out and how to set.

		void* end   = path[path.Num()-1];
		Item item = { path[i], end, path[i+1], cost[i] };
		AddItem( item );
	}
}


void FPathCache::AddNoSolution( void* end, void* states[], int count )
{
	if ( count + NumItems > NumItemsAllocated*3/4 ) {
		return;
	}

	for( int i=0; i<count; ++i ) {
		Item item = { states[i], end, 0, FLT_MAX };
		AddItem( item );
	}
}


int FPathCache::Solve( void* start, void* end, TArray< void* >* path, float* totalCost )
{
	const Item* item = Find( start, end );
	if ( item ) {
		if ( item->Cost == FLT_MAX ) {
			++NumHit;
			return FMicroPather::NO_SOLUTION;
		}

		path->Empty();
		path->Add( start );
		*totalCost = 0;

		for ( ;start != end; start=item->Next, item=Find(start, end) ) {
			MPASSERT( item );
			*totalCost += item->Cost;
			path->Add( item->Next );
		}
		++NumHit;
		return FMicroPather::SOLVED;
	}
	++NumMiss;
	return FMicroPather::NOT_CACHED;
}


void FPathCache::AddItem( const Item& item )
{
	MPASSERT( NumItemsAllocated );
	unsigned index = item.Hash() % NumItemsAllocated;
	while( true ) {
		if ( Items[index].Empty() ) {
			Items[index] = item;
			++NumItems;
#ifdef DEBUG_CACHING
			GLOUTPUT(( "Add: start=%x next=%x end=%x\n", item.Start, item.Next, item.End ));
#endif
			break;
		}
		else if ( Items[index].KeyEqual( item ) ) {
			MPASSERT( (Items[index].Next && item.Next) || (Items[index].Next==0 && item.Next == 0) );
			// do nothing; in cache
			break;
		}
		++index;
		if ( index == NumItemsAllocated )
			index = 0;
	}
}


const FPathCache::Item* FPathCache::Find( void* start, void* end )
{
	MPASSERT( NumItemsAllocated );
	Item fake = { start, end, 0, 0 };
	unsigned index = fake.Hash() % NumItemsAllocated;
	while( true ) {
		if ( Items[index].Empty() ) {
			return 0;
		}
		if ( Items[index].KeyEqual( fake )) {
			return Items + index;
		}
		++index;
		if ( index == NumItemsAllocated )
			index = 0;
	}
}


void FMicroPather::GetCacheData( FCacheData* data )
{
	memset( data, 0, sizeof(*data) );

	if ( PathCache ) {
		data->BytesAllocated = PathCache->AllocatedBytes();
		data->BytesUsed = PathCache->UsedBytes();
		data->MemoryFraction = (float)( (double)data->BytesUsed / (double)data->BytesAllocated );

		data->NumHit = PathCache->NumHit;
		data->NumMiss = PathCache->NumMiss;
		if ( data->NumHit + data->NumMiss ) {
		data->HitFraction = (float)( (double)(data->NumHit) / (double)(data->NumHit + data->NumMiss) );
	}
		else {
			data->HitFraction = 0;
		}
	}
}



int FMicroPather::Solve( void* startNode, void* endNode, TArray< void* >* path, float* cost )
{
	// Important to clear() in case the caller doesn't check the return code. There
	// can easily be a left over path  from a previous call.
	path->Empty();

	#ifdef DEBUG_PATH
	printf( "Path: " );
	Graph->PrintStateInfo( startNode );
	printf( " --> " );
	Graph->PrintStateInfo( endNode );
	printf( " min cost=%f\n", Graph->LeastCostEstimate( startNode, endNode ) );
	#endif

	*cost = 0.0f;

	if ( startNode == endNode )
		return START_END_SAME;

	if ( PathCache ) {
		int cacheResult = PathCache->Solve( startNode, endNode, path, cost );
		if ( cacheResult == SOLVED || cacheResult == NO_SOLUTION ) {
		#ifdef DEBUG_CACHING
			GLOUTPUT(( "PathCache hit. result=%s\n", cacheResult == SOLVED ? "solved" : "no_solution" ));
		#endif
			return cacheResult;
		}
		#ifdef DEBUG_CACHING
		GLOUTPUT(( "PathCache miss\n" ));
		#endif
	}

	++Frame;

	FOpenQueue open( Graph );
	FClosedSet closed( Graph );
	
	FPathNode* newPathNode = PathNodePool.GetPathNode(	Frame, 
														startNode, 
														0, 
														Graph->LeastCostEstimate( startNode, endNode ), 
														0 );

	open.Push( newPathNode );	
	StateCosts.Empty();
	NodeCosts.Empty(0);

	while ( !open.IsEmpty() )
	{
		FPathNode* node = open.Pop();
		
		if ( node->State == endNode )
		{
			GoalReached( node, startNode, endNode, path );
			*cost = node->CostFromStart;
			#ifdef DEBUG_PATH
			DumpStats();
			#endif
			return SOLVED;
		}
		else
		{
			closed.Add( node );

			// We have not reached the goal - add the neighbors.
			GetNodeNeighbors( node, &NodeCosts );

			for( int i=0; i<node->NumAdjacent; ++i )
			{
				// Not actually a neighbor, but useful. Filter out infinite cost.
				if ( NodeCosts[i].Cost == FLT_MAX ) {
					continue;
				}
				FPathNode* child = NodeCosts[i].Node;
				float newCost = node->CostFromStart + NodeCosts[i].Cost;

				FPathNode* inOpen   = child->bInOpen ? child : 0;
				FPathNode* inClosed = child->bInClosed ? child : 0;
				FPathNode* inEither = (FPathNode*)( ((MP_UPTR)inOpen) | ((MP_UPTR)inClosed) );

				MPASSERT( inEither != node );
				MPASSERT( !( inOpen && inClosed ) );

				if ( inEither ) {
					if ( newCost < child->CostFromStart ) {
						child->Parent = node;
						child->CostFromStart = newCost;
						child->EstToGoal = Graph->LeastCostEstimate( child->State, endNode );
						child->CalcTotalCost();
						if ( inOpen ) {
							open.Update( child );
						}
					}
				}
				else {
					child->Parent = node;
					child->CostFromStart = newCost;
					child->EstToGoal = Graph->LeastCostEstimate( child->State, endNode ),
					child->CalcTotalCost();
					
					MPASSERT( !child->bInOpen && !child->bInClosed );
					open.Push( child );
				}
			}
		}					
	}
	#ifdef DEBUG_PATH
	DumpStats();
	#endif
	if ( PathCache ) {
		// Could add a bunch more with a little tracking.
		PathCache->AddNoSolution( endNode, &startNode, 1 );
	}
	return NO_SOLUTION;		
}	


int FMicroPather::SolveForNearStates( void* startState, TArray< FStateCost >* near, float maxCost )
{
	/*	 http://en.wikipedia.org/wiki/Dijkstra%27s_algorithm

		 1  function Dijkstra(Graph, source):
		 2      for each vertex v in Graph:           // Initializations
		 3          dist[v] := infinity               // Unknown distance function from source to v
		 4          previous[v] := undefined          // Previous node in optimal path from source
		 5      dist[source] := 0                     // Distance from source to source
		 6      Q := the set of all nodes in Graph
				// All nodes in the graph are unoptimized - thus are in Q
		 7      while Q is not empty:                 // The main loop
		 8          u := vertex in Q with smallest dist[]
		 9          if dist[u] = infinity:
		10              break                         // all remaining vertices are inaccessible from source
		11          remove u from Q
		12          for each neighbor v of u:         // where v has not yet been removed from Q.
		13              alt := dist[u] + dist_between(u, v) 
		14              if alt < dist[v]:             // Relax (u,v,a)
		15                  dist[v] := alt
		16                  previous[v] := u
		17      return dist[]
	*/

	++Frame;

	FOpenQueue open( Graph );			// nodes to look at
	FClosedSet closed( Graph );

	NodeCosts.Empty();
	StateCosts.Empty();

	FPathNode closedSentinel;
	closedSentinel.Clear();
	closedSentinel.Init( Frame, 0, FLT_MAX, FLT_MAX, 0 );
	closedSentinel.Next = closedSentinel.Prev = &closedSentinel;

	FPathNode* newPathNode = PathNodePool.GetPathNode( Frame, startState, 0, 0, 0 );
	open.Push( newPathNode );
	
	while ( !open.IsEmpty() )
	{
		FPathNode* node = open.Pop();	// smallest dist
		closed.Add( node );				// add to the things we've looked at
		closedSentinel.AddBefore( node );
			
		if ( node->TotalCost > maxCost )
			continue;		// Too far away to ever get here.

		GetNodeNeighbors( node, &NodeCosts );

		for( int i=0; i<node->NumAdjacent; ++i )
		{
			MPASSERT( node->CostFromStart < FLT_MAX );
			float newCost = node->CostFromStart + NodeCosts[i].Cost;

			FPathNode* inOpen   = NodeCosts[i].Node->bInOpen ? NodeCosts[i].Node : 0;
			FPathNode* inClosed = NodeCosts[i].Node->bInClosed ? NodeCosts[i].Node : 0;
			MPASSERT( !( inOpen && inClosed ) );
			FPathNode* inEither = inOpen ? inOpen : inClosed;
			MPASSERT( inEither != node );

			if ( inEither && inEither->CostFromStart <= newCost ) {
				continue;	// Do nothing. This path is not better than existing.
			}
			// Groovy. We have new information or improved information.
			FPathNode* child = NodeCosts[i].Node;
			MPASSERT( child->State != newPathNode->State );	// should never re-process the parent.

			child->Parent = node;
			child->CostFromStart = newCost;
			child->EstToGoal = 0;
			child->TotalCost = child->CostFromStart;

			if ( inOpen ) {
				open.Update( inOpen );
			}
			else if ( !inClosed ) {
				open.Push( child );
			}
		}
	}	
	near->Empty();

	for( FPathNode* pNode=closedSentinel.Next; pNode != &closedSentinel; pNode=pNode->Next ) {
		if ( pNode->TotalCost <= maxCost ) {
			FStateCost sc;
			sc.Cost = pNode->TotalCost;
			sc.State = pNode->State;

			near->Add( sc );
		}
	}
#ifdef DEBUG
	for( unsigned i=0; i<near->Num(); ++i ) {
		for( unsigned k=i+1; k<near->Num(); ++k ) {
			MPASSERT( (*near)[i].state != (*near)[k].state );
		}
	}
#endif

	return SOLVED;
}




