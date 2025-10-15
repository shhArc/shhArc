///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2025 David K Bhowmik. All rights reserved.
// This file is part of shhArc.
//
// This Software is available under the MIT License with a No Modification clause.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this file and associated documentation files (the "Software"), to use the
// Software for any purpose, including commercial applications, provided that:
//
//   1. You do NOT modify, alter, or create derivative works of this file.
//   2. Redistributions must include this notice and may only distribute it 
//      unmodified.
//
// The full MIT License text, including this Custom No Modification clause,
// is available in the LICENSE file in the root of the project.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND.
//
// You may alternatively use this source under the terms of a specific 
// version of the shhArc Unrestricted License provided you have obtained 
// such a license from the copyright holder.
///////////////////////////////////////////////////////////////////////////////

#include "../Common/Debug.h"
#include "../Common/ThreadSafety.h"
#include "../Common/Mutex.h"
#include "Allocator.h"
#include "Chunk.h"



namespace shh {



	// --------------------------------------------------------------------------						
	// Function:	Allocator
	// Description:	constructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	Allocator::Allocator() :
		myDataSize(0),
		myMaxBlocks(0),
		myBlocksInUse(0),
		myHardLimitingRatio(0.0f),
		myLimitingRatio(0.0f),
		myPeak(-1),
		myTrough(-1),
		myAllocatingChunk(NULL)
	{
		myMutex = new Mutex;
	}


	// --------------------------------------------------------------------------						
	// Function:	Allocator
	// Description:	constructor
	// Arguments:	size of each data block, number of block, resize ratio metric
	// Returns:		none
	// --------------------------------------------------------------------------
	Allocator::Allocator(MemorySize dataSize, unsigned int maxBlocks, float limitingRatio) :
		myDataSize(dataSize),
		myMaxBlocks(maxBlocks),
		myBlocksInUse(0),
		myAllocatingChunk(NULL)
	{
		myMutex = new Mutex;
		SetDynamicDefrag(limitingRatio);
	}


	// --------------------------------------------------------------------------						
	// Function:	~Allocato
	// Description:	destructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	Allocator::~Allocator()
	{
		for (int c = 0; c != myChunks.size(); c++)
			delete myChunks[c];

		delete myMutex;
	}


	// --------------------------------------------------------------------------						
	// Function:	Allocate
	// Description:	allocate memory block in an available chunk
	// Arguments:	none
	// Returns:		pointer to memory
	// --------------------------------------------------------------------------
	void* Allocator::Allocate()
	{
		LOCK_MUTEX(myMutex);

		if (myAllocatingChunk == NULL || myAllocatingChunk->BlocksAvailable() == 0)
		{
			// find chunk with free blocks
			unsigned int minAvailable = myMaxBlocks;
			Chunks::iterator found = myChunks.end();
			Chunks::iterator it = myChunks.begin();
			while(it != myChunks.end())
			{
				unsigned int blocksAvailable = (*it)->BlocksAvailable();
				if (blocksAvailable != 0 && blocksAvailable < minAvailable)
				{
					minAvailable = blocksAvailable;
					found = it;
				}
				++it;
			}

			if (found == myChunks.end())
			{
				// create new chunk
				Chunk* newChunk = new Chunk(this, myDataSize, myMaxBlocks);
				myChunks.push_back(newChunk);
				myAllocatingChunk = myChunks.back();
				myAllocatingChunk->Init();
			}
			else
			{
				// set chunk found with free blocks as currently active
				myAllocatingChunk = *found;
			}
		}

		Chunk* allocatungChunk = myAllocatingChunk;
		UNLOCK_MUTEX(myMutex);

		return allocatungChunk->Allocate();
	}


	// --------------------------------------------------------------------------						
	// Function:	Deallocate
	// Description:	deallocate memory block within a chunk
	// Arguments:	pointer to deallocate
	// Returns:		none
	// --------------------------------------------------------------------------
	void Allocator::Deallocate(void* p)
	{
		MemoryLocator::Header& head = MemoryLocator::GetHeader(p);
		((Chunk*)head.myLocator)->Deallocate(p);
	}


	// --------------------------------------------------------------------------						
	// Function:	SetDynamicDefrag
	// Description:	sets the resizing tio metric
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void Allocator::SetDynamicDefrag(float limitingRatio)
	{
		myPeak = -1;
		myTrough = -1;
		myHardLimitingRatio = limitingRatio;
		myLimitingRatio = limitingRatio;
		RELEASE_ASSERT(myHardLimitingRatio <= 1.0f);
	}


	// --------------------------------------------------------------------------						
	// Function:	SetDynamicDefrag
	// Description:	log number of blocks in use in all chunks
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void Allocator::RecordUsage()
	{
		myBlocksInUse = 0;
		Chunks::iterator it = myChunks.begin();
		while (it != myChunks.end())
			myBlocksInUse += (*(it++))->BlocksUsed();

		if (myTrough == -1 || myBlocksInUse < (unsigned int)myTrough)
			myTrough = myBlocksInUse;

		if (myPeak == -1 || myBlocksInUse > (unsigned int)myPeak)
			myPeak = myBlocksInUse;

	}


	// --------------------------------------------------------------------------						
	// Function:	Defrag
	// Description:	removes gaps in all chunks
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void Allocator::Defrag()
	{
		LOCK_MUTEX(myMutex);

		// calculate limiting ratio
		if (myPeak == 0)
			myLimitingRatio = 0.0f;
		else if (myTrough == 0)
			myLimitingRatio = myHardLimitingRatio;
		else
			myLimitingRatio = (float)myTrough / (float)myPeak; // dynamic calculatiomn


		myPeak = -1;
		myTrough = -1;

		// bount blocks available in all my chunks
		unsigned int blocksAvailable = 0;
		Chunks::iterator it = myChunks.begin();
		while (it != myChunks.end())
			blocksAvailable += (*(it++))->BlocksAvailable();


		float totalBlocks = (float)myChunks.size() * myMaxBlocks;

		it = myChunks.begin();
		while (it != myChunks.end())
		{
			unsigned int chunkBlocksUsed = (*it)->BlocksUsed();
			float chunkRatio = (float)chunkBlocksUsed / (float)myMaxBlocks;

			if (!(myLimitingRatio == 0.0f && myChunks.size() == 1))
			{
				// if usage in this chunk is less than average usage ratio or hard defined ratio then relocate
				if (chunkRatio <= myLimitingRatio)
				{
					while ((*it)->BlocksUsed() && chunkBlocksUsed <= (blocksAvailable - (*it)->BlocksAvailable()))
					{
						// find chunk with smallest free space
						unsigned int smallestFree = myMaxBlocks;
						Chunks::iterator relocateTo = myChunks.end();
						Chunks::iterator checkIt = myChunks.begin();
						while (checkIt != myChunks.end())
						{
							if (checkIt != it)
							{
								unsigned int available = (*(checkIt))->BlocksAvailable();
								if (available > 0 && available < smallestFree)
								{
									smallestFree = available;
									relocateTo = checkIt;
								}
							}
							++checkIt;
						}

						// relocate my block in other chunks
						if (relocateTo != myChunks.end())
						{
							Chunk* r = *relocateTo;
							(*it)->Relocate(**relocateTo);
							chunkBlocksUsed = (*it)->BlocksUsed();
						}
					}


					if((*it)->BlocksAvailable() == myMaxBlocks && blocksAvailable > myMaxBlocks)
					{
						// delete chunk
						MemorySize dist = std::distance(myChunks.begin(), it);
						delete* it;
						myChunks.erase(it);
						it = myChunks.begin();
						std::advance(it, dist);
						myAllocatingChunk = NULL;
						blocksAvailable -= myMaxBlocks;
						continue;
					}
				}
			}
			it++;
		}
		UNLOCK_MUTEX(myMutex);
	}


	// --------------------------------------------------------------------------						
	// Function:	SetMaxBlocks
	// Description:	set maximum number of blocks in a chunks
	// Arguments:	num blocks
	// Returns:		none
	// --------------------------------------------------------------------------
	void Allocator::SetMaxBlocks(unsigned int blocks)
	{
		if (myChunks.empty())
		{
			myMaxBlocks = blocks;
			myPeak = 0;
			myTrough = blocks;
		}
	}


} 
