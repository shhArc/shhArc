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


#include "../Config/GCPtr.h"
#include "../Common/Debug.h"
#include "../Common/ThreadSafety.h"
#include "../Common/Mutex.h"
#include "MemoryManager.h"

namespace shh {


	unsigned int MemoryManager::StdAllocatorSize::ourSingle = 256;
	unsigned int MemoryManager::StdAllocatorSize::ourMultiple = 32;


	////////////////////////////////////////////////////////////////////////////////////////////

	MemoryManager* MemoryManager::ourManager = NULL;
	int MemoryManager::ourGranularity = sizeof(MemorySize);



	// --------------------------------------------------------------------------						
	// Function:	GetManager
	// Description:	gets memory manager singleton
	// Arguments:	none
	// Returns:		memory manager
	// --------------------------------------------------------------------------
	MemoryManager& MemoryManager::GetManager()
	{
		if (ourManager == NULL)
			ourManager = new MemoryManager();

		return *ourManager;
	}


	// --------------------------------------------------------------------------						
	// Function:	CloseManager
	// Description:	closes memory manager removing all allocators and memory
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void MemoryManager::CloseManager()
	{
		if (ourManager != NULL)
		{
			delete ourManager;
			ourManager = NULL;
		}
	}


	// --------------------------------------------------------------------------						
	// Function:	MemoryManager
	// Description:	constructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	MemoryManager::MemoryManager() :
		myMaxBlocks(256)
	{
		myMutex = new Mutex;

		SetDynamicDefrag(0.75);
	}


	// --------------------------------------------------------------------------						
	// Function:	~MemoryManager
	// Description:	destructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	MemoryManager::~MemoryManager()
	{
		delete myMutex;
		Allocators::iterator it = myAllocators.begin();
		while (it != myAllocators.end())
		{
			delete it->second;
			it++;
		}

	}


	// --------------------------------------------------------------------------						
	// Function:	GetAllocator
	// Description:	get allocator of that handles block size
	// Arguments:	data block size, max blocks in a chunk
	// Returns:		allocator
	// --------------------------------------------------------------------------
	Allocator* MemoryManager::GetAllocator(MemorySize dataSize, unsigned int maxBlocks)
	{
		MemorySize  blockSize = dataSize + MemoryLocator::fixedHeaderSize;
		MemorySize aligned = ((blockSize / ourGranularity) * ourGranularity) + ourGranularity;
		blockSize = (aligned == blockSize + ourGranularity ? blockSize : aligned);
		LOCK_MUTEX(myMutex);
		MemorySize newDataSize = blockSize - MemoryLocator::fixedHeaderSize;
		Allocators::iterator it = myAllocators.find(newDataSize);
		if (it == myAllocators.end())
		{
			myAllocators[dataSize] = new Allocator(newDataSize, (maxBlocks == 0 ? myMaxBlocks : maxBlocks), myLimitingRatio);
			it = myAllocators.find(dataSize);
		}
		UNLOCK_MUTEX(myMutex);
		return it->second;
	}


	// --------------------------------------------------------------------------						
	// Function:	SetMaxBlocks
	// Description:	sets global max blocks per chunk default
	// Arguments:	max blocks
	// Returns:		none
	// --------------------------------------------------------------------------
	void MemoryManager::SetMaxBlocks(unsigned int maxBlocks)
	{
		myMaxBlocks = maxBlocks;
		LOCK_MUTEX(myMutex);
		Allocators::iterator it = myAllocators.begin();
		if (it == myAllocators.end())
		{
			it->second->SetMaxBlocks(myMaxBlocks);
			it++;
		}
		UNLOCK_MUTEX(myMutex);
		return;
	}


	// --------------------------------------------------------------------------						
	// Function:	SetDynamicDefrag
	// Description:	sets the auto resize limiting ratio for chunks
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void MemoryManager::SetDynamicDefrag(float limitingRatio)
	{
		myLimitingRatio = limitingRatio;
		LOCK_MUTEX(myMutex);
		Allocators::iterator it = myAllocators.begin();
		while (it != myAllocators.end())
		{
			it->second->SetDynamicDefrag(myLimitingRatio);
			it++;
		}
		UNLOCK_MUTEX(myMutex);
	}


	// --------------------------------------------------------------------------						
	// Function:	RecordUsage
	// Description:	reecord blocks used in each allocator/chunk
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void MemoryManager::RecordUsage()
	{
		LOCK_MUTEX(myMutex);
		Allocators::iterator it = myAllocators.begin();
		while (it != myAllocators.end())
		{
			it->second->RecordUsage();
			it++;
		}
		UNLOCK_MUTEX(myMutex);
		return;
	}


	// --------------------------------------------------------------------------						
	// Function:	Defrag
	// Description:	remove all gaps in chunks in all allocaors
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void MemoryManager::Defrag()
	{
		LOCK_MUTEX(myMutex);
		Allocators::iterator it = myAllocators.begin();
		while (it != myAllocators.end())
		{
			it->second->Defrag();
			it++;
		}
		UNLOCK_MUTEX(myMutex);
		return;
	}




}
