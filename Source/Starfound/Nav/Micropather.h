/*
Copyright (c) 2000-2013 Lee Thomason (www.grinninglizard.com)
Micropather

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

#pragma once

#ifndef GRINNINGLIZARD_MICROPATHER_INCLUDED
#define GRINNINGLIZARD_MICROPATHER_INCLUDED


/** @mainpage MicroPather
	
	MicroPather is a path finder and A* solver (astar or a-star) written in platform independent 
	C++ that can be easily integrated into existing code. MicroPather focuses on being a path 
	finding engine for video games but is a generic A* solver. MicroPather is open source, with 
	a license suitable for open source or commercial use.
*/

#include <float.h>

#ifdef _DEBUG
	#ifndef DEBUG
		#define DEBUG
	#endif
#endif

#define MPASSERT ensure

#if defined(_MSC_VER) && (_MSC_VER >= 1400 )
	#include <stdlib.h>
	typedef uintptr_t		MP_UPTR;
#elif defined (__GNUC__) && (__GNUC__ >= 3 )
	#include <stdint.h>
	#include <stdlib.h>
	typedef uintptr_t		MP_UPTR;
#else
	// Assume not 64 bit pointers. Get a new compiler.
	typedef unsigned MP_UPTR;
#endif

namespace MicroPanther
{
	/**
		Used to pass the cost of states from the client application to MicroPather. This
		structure is copied in a vector.

		@sa AdjacentCost
	*/
	struct FStateCost
	{
		void* State;			///< The state as a void*
		float Cost;				///< The cost to the state. Use FLT_MAX for infinite cost.
	};


	/**
		A pure abstract class used to define a set of callbacks. 
		The client application inherits from 
		this class, and the methods will be called when MicroPather::Solve() is invoked.

		The notion of a "state" is very important. It must have the following properties:
		- Unique
		- Unchanging (unless MicroPather::Reset() is called)

		If the client application represents states as objects, then the state is usually
		just the object cast to a void*. If the client application sees states as numerical
		values, (x,y) for example, then state is an encoding of these values. MicroPather
		never interprets or modifies the value of state.
	*/
	class FGraph
	{
	  public:
		virtual ~FGraph() {}
	  
		/**
			Return the least possible cost between 2 states. For example, if your pathfinding 
			is based on distance, this is simply the straight distance between 2 points on the 
			map. If you pathfinding is based on minimum time, it is the minimal travel time 
			between 2 points given the best possible terrain.
		*/
		virtual float LeastCostEstimate(void* StartState, void* EndState) = 0;

		/** 
			Return the exact cost from the given state to all its neighboring states. This
			may be called multiple times, or cached by the solver. It *must* return the same
			exact values for every call to MicroPather::Solve(). It should generally be a simple,
			fast function with no callbacks into the pather.
		*/	
		virtual void AdjacentCost(void* State, TArray< MicroPanther::FStateCost >* Adjacent) = 0;

		/**
			This function is only used in DEBUG mode - it dumps output to stdout. Since void* 
			aren't really human readable, normally you print out some concise info (like "(1,2)") 
			without an ending newline.
		*/
		virtual void  PrintStateInfo(void* State) = 0;
	};


	class FPathNode;

	struct FNodeCost
	{
		FPathNode* Node;
		float Cost;
	};


	/*
		Every state (void*) is represented by a PathNode in MicroPather. There
		can only be one PathNode for a given state.
	*/
	class FPathNode
	{
	  public:
		  void Init(
			  uint32 InFrame,
			  void* InState,
			  float InCostFromStart,
			  float InEstToGoal,
			  FPathNode* InParent);

		void Clear();
		void InitSentinel();	

		void* State;			// the client state
		float CostFromStart;	// exact
		float EstToGoal;		// estimated
		float TotalCost;		// could be a function, but save some math.
		FPathNode* Parent;		// the parent is used to reconstruct the path
		unsigned Frame;			// unique id for this path, so the solver can distinguish
								// correct from stale values

		int NumAdjacent;		// -1  is unknown & needs to be queried
		int CacheIndex;			// position in cache
		uint32 CacheFrame;

		FPathNode* Child[2];		// Binary search in the hash table. [left, right]
		FPathNode* Next, *Prev;	// used by open queue

		bool bInOpen;
		bool bInClosed;

		void Unlink();
		void AddBefore(FPathNode* Node);

#ifdef DEBUG
		void CheckList()
		{
			MPASSERT( TotalCost == FLT_MAX );
			for( FPathNode* it = Next; it != this; it=it->Next ) {
				MPASSERT( it->Prev == this || it->TotalCost >= it->Prev->TotalCost );
				MPASSERT( it->TotalCost <= it->Next->TotalCost );
			}
		}
#endif

		void CalcTotalCost();

	private:
		void operator=(const FPathNode&);
	};


	/* Memory manager for the PathNodes. */
	class FPathNodePool
	{
	public:
		FPathNodePool(unsigned InNumNodesPerBlock, unsigned _typicalAdjacent);
		~FPathNodePool();

		// Free all the memory except the first block. Resets all memory.
		void Clear();

		void ClearNeighborCache();

		// Essentially:
		// pNode = Find();
		// if ( !pNode )
		//		pNode = New();
		//
		// Get the PathNode associated with this state. If the PathNode already
		// exists (allocated and is on the current frame), it will be returned. 
		// Else a new PathNode is allocated and returned. The returned object
		// is always fully initialized.
		//
		// NOTE: if the pathNode exists (and is current) all the initialization
		//       parameters are ignored.
		FPathNode* GetPathNode(
			unsigned frame,
			void* _state,
			float _costFromStart,
			float _estToGoal,
			FPathNode* _parent);

		// Get a pathnode that is already in the pool.
		FPathNode* FetchPathNode(void* state);

		// Store stuff in cache
		bool PushCache(const TArray<FNodeCost>& Nodes, int32* OutStartIndex);

		// Get neighbors from the cache
		// Note - always access this with an offset. Can get re-allocated.
		void GetCache(int32 CacheIndex, int32 NumNodeCosts, TArray<FNodeCost>& OutNodeCosts);

		// Return all the allocated states. Useful for visuallizing what
		// the pather is doing.
		void AllStates(uint32 frame, TArray<void*>* stateVec);

	private:
		struct FBlock
		{
			FBlock* NextBlock;
			FPathNode PathNode[1];
		};

		unsigned Hash(void* voidval);
		unsigned HashSize() const { return 1 << HashShift; }
		unsigned HashMask()	const { return ((1 << HashShift) - 1); }
		void AddPathNode(uint32 HashKey, FPathNode* RootNode);
		FBlock* AllocBlock();
		FPathNode* Alloc();

		FPathNode** HashTable;
		FBlock* FirstBlock;
		FBlock* Blocks;

		FNodeCost* NeighborCostsCache;
		int32 NeighborCostsCapacity;
		int32 NeighborCostsCacheSize;

		FPathNode FreeMemSentinel;

		// how big a block of path nodes to allocate at once
		uint32 NumNodesPerBlock;

		// number of pathnodes allocated (from Alloc())
		uint32 NumTotalAllocatedNodes;

		// number available for allocation
		uint32 NumAvailableNodesOnLastBlock;
				 
		uint32 HashShift;

		// Debug purpose
		int32 NumHashCollision;
	};


	/* Used to cache results of paths. Much, much faster
	   to return an existing solution than to calculate
	   a new one. A post on this is here: http://grinninglizard.com/altera/programming/a-path-caching-2/
	*/
	class FPathCache
	{
	public:
		struct FItem
		{
			// The key:
			void* Start;
			void* End;

			bool KeyEqual(const FItem& item) const { return Start == item.Start && End == item.End; }
			bool Empty() const { return Start == 0 && End == 0; }

			// Data:
			void* Next;
			float Cost;	// from 'start' to 'next'. FLT_MAX if unsolveable.

			unsigned Hash() const
			{
				const unsigned char *p = (const unsigned char *)(&Start);
				unsigned int h = 2166136261U;

				for (unsigned i = 0; i < sizeof(void*) * 2; ++i, ++p)
				{
					h ^= *p;
					h *= 16777619;
				}
				return h;
			}
		};

		explicit FPathCache(int NumItemsToAllocate);
		FPathCache(const FPathCache&) = delete;
		~FPathCache();
		
		FPathCache& operator = (const FPathCache&) = delete;
		
		void Reset();
		void Add(const TArray<void*>& path, const TArray<float>& cost);
		void AddNoSolution(void* end, void* states[], int count);
		int Solve(void* startState, void* endState, TArray<void*>* path, float* totalCost);

		int AllocatedBytes() const { return NumItemsAllocated * sizeof(FItem); }
		int UsedBytes() const { return NumItems * sizeof(FItem); }

		int NumHit;
		int NumMiss;

	private:
		void AddItem(const FItem& item);
		const FItem* Find(void* start, void* end);

		FItem* Items;
		int32 NumItemsAllocated;
		int32 NumItems;
	};

	struct FCacheData
	{
		FCacheData() : BytesAllocated(0), BytesUsed(0), MemoryFraction(0), NumHit(0), NumMiss(0), HitFraction(0) {}

		int32 BytesAllocated;
		int32 BytesUsed;
		float MemoryFraction;

		int32 NumHit;
		int32 NumMiss;
		float HitFraction;
	};

	/**
		Create a MicroPather object to solve for a best path. Detailed usage notes are
		on the main page.
	*/
	class FMicroPather
	{
		friend class MicroPanther::FPathNode;

	  public:
		enum
		{
			SOLVED,
			NO_SOLUTION,
			START_END_SAME,

			// internal
			NOT_CACHED
		};

		/**
			Construct the pather, passing a pointer to the object that implements
			the Graph callbacks.

			@param InGraph			The "map" that implements the Graph callbacks.
			@param NumStatesAlloc	How many states should be internally allocated at a time. This
									can be hard to get correct. The higher the value, the more memory
									MicroPather will use.
									- If you have a small map (a few thousand states?) it may make sense
									  to pass in the maximum value. This will cache everything, and MicroPather
									  will only need one main memory allocation. For a chess board, allocate
									  would be set to 8x8 (64)
									- If your map is large, something like 1/4 the number of possible
									  states is good.
									- If your state space is huge, use a multiple (5-10x) of the normal
									  path. "Occasionally" call Reset() to free unused memory.
			@param NumTypicalAdjacent	Used to determine cache size. The typical number of adjacent states
										to a given state. (On a chessboard, 8.) Higher values use a little
										more memory.
			@param bUseCache	Turn on path caching. Uses more memory (yet again) but at a huge speed
								advantage if you may call the pather with the same path or sub-path, which
								is common for pathing over maps in games.
		*/
		FMicroPather(FGraph* InGraph, uint32 NumStatesAlloc = 250, uint32 NumTypicalAdjacent = 6, bool bUseCache = true);
		~FMicroPather();

		/**
			Solve for the path from start to end.

			@param startState	Input, the starting state for the path.
			@param endState		Input, the ending state for the path.
			@param path			Output, a vector of states that define the path. Empty if not found.
			@param totalCost	Output, the cost of the path, if found.
			@return				Success or failure, expressed as SOLVED, NO_SOLUTION, or START_END_SAME.
		*/
		int Solve(void* StartState, void* EndState, TArray<void*>* Path, float* TotalCost);

		/**
			Find all the states within a given cost from startState.

			@param startState	Input, the starting state for the path.
			@param near			All the states within 'maxCost' of 'startState', and cost to that state.
			@param maxCost		Input, the maximum cost that will be returned. (Higher values return
								larger 'near' sets and take more time to compute.)
			@return				Success or failure, expressed as SOLVED or NO_SOLUTION.
		*/
		int SolveForNearStates(void* startState, TArray<FStateCost>* near, float maxCost);

		/** Should be called whenever the cost between states or the connection between states changes.
			Also frees overhead memory used by MicroPather, and calling will free excess memory.
		*/
		void Reset();

		// Debugging function to return all states that were used by the last "solve" 
		void Debug_StatesInPool(TArray<void*>* stateVec);
		void Debug_GetCacheData(FCacheData* data);

	  private:
		  FMicroPather(const FMicroPather&);	// undefined and unsupported
		  void operator=(const FMicroPather); // undefined and unsupported

		  void GoalReached(FPathNode* node, void* start, void* end, TArray<void*> *path);
		  void GetNodeNeighbors(FPathNode* node, TArray<FNodeCost>* neighborNode);

		  void IncreaseFrame();

#ifdef DEBUG
		  //void DumpStats();
#endif

		  FPathNodePool PathNodePool;

		  // local to Solve, but put here to reduce memory allocation
		  TArray<FStateCost> TempStateCosts;

		  // local to Solve, but put here to reduce memory allocation
		  TArray<FNodeCost> TempNodeCosts;	
		  TArray<float>	TempCosts;

		  FGraph* Graph;

		  // incremented with every solve, used to determine if cached data needs to be refreshed
		  uint32 Frame;
		  FPathCache* PathCache;
	};

};	// MicroPanther

#endif
