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
#include "Chunk.h"
#include "Allocator.h"

namespace shh {


	// --------------------------------------------------------------------------						
	// Function:	Chunk
	// Description:	constructor
	// Arguments:	owning allocator, size of data block, num blocks
	// Returns:		none
	// --------------------------------------------------------------------------
	Chunk::Chunk(Allocator* owner, MemorySize dataSize, int blocks) :
		MemoryLocator(false),
		myAllocator(owner),
		myDataSize(dataSize),
		myBlockSize(dataSize + MemoryLocator::fixedHeaderSize),
		myTotalBlocks(blocks),
		myBlocksAvailable(blocks),
		myFreeSpace(NULL),
		myRecycledSpace(NULL)
	{
		myMutex = new Mutex;
	}



	// --------------------------------------------------------------------------						
	// Function:	~Chunk
	// Description:	destructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	Chunk::~Chunk()
	{
		if (myBase != NULL)
		{
			while (myInUseHead != NULL)
			{
				if (myInUseHead->myMemoryInfo && myInUseHead->myMemoryInfo->myGCObject)
				{
					GCObject::DestroyObjectVirtuallyIfPossible(myInUseHead->myMemoryInfo->myGCObject, myInUseHead->myMemoryInfo->myGCObject);
				}
				else
				{
					if (myInUseHead->myDestructor != NULL)
						myInUseHead->myDestructor(((FixedHeader*)myInUseHead) + 1);
					Deallocate(((FixedHeader*)myInUseHead) + 1);
				}
			}
			delete[]myBase;
		}
		delete myMutex;
	}



	// --------------------------------------------------------------------------						
	// Function:	Init
	// Description:	allocate chunk memory and initialzes chunk
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void Chunk::Init()
	{
		MemorySize size = myBlockSize * myBlocksAvailable;
		myBase = new char[size];
		myEnd = myBase + size;
		myFreeSpace = myBase;
	}


	// --------------------------------------------------------------------------						
	// Function:	Allocate
	// Description:	allocates a dataa block in chunk
	// Arguments:	none
	// Returns:		pointer to block
	// --------------------------------------------------------------------------
	void* Chunk::Allocate()
	{
		LOCK_MUTEX(myMutex);

		DEBUG_ASSERT(myBlocksAvailable != 0);

		FixedHeader* head;
		if (myRecycledSpace)
		{
			// use previously freed space
			head = myRecycledSpace;
			myRecycledSpace = (FixedHeader*)myRecycledSpace->myNextHeader;
			if(myRecycledSpace != NULL)
				myRecycledSpace->myPreviousHeader = NULL;
		}
		else
		{
			// use space at end of chunk
			memset(myFreeSpace, 0, myBlockSize);
			head = new (myFreeSpace) FixedHeader(this);
			myFreeSpace = myFreeSpace + myBlockSize;
		}

		// chain headers
		FixedHeader* inUseHead = (FixedHeader*)myInUseHead;
		if (myInUseHead != NULL)
			inUseHead->myPreviousHeader = head;
		head->myNextHeader = inUseHead;
		myInUseHead = head;

		--myBlocksAvailable;

		UNLOCK_MUTEX(myMutex);

		return (char*)head + fixedHeaderSize;
	}



	// --------------------------------------------------------------------------						
	// Function:	Deallocate
	// Description:	deallocate block in chunk
	// Arguments:	block data ptr, whether to invalidate pointer
	// Returns:		pointer to header of deallocated block
	// --------------------------------------------------------------------------
	MemoryLocator::Header* Chunk::Deallocate(void* p, bool invalidateInfo)
	{
		LOCK_MUTEX(myMutex);

		Header* next = MemoryLocator::Deallocate(p, invalidateInfo);

		FixedHeader* head = &MemoryLocator::GetFixedHeader(p);

		// add me to the recycled space
		head->myNextHeader = myRecycledSpace;
		head->myPreviousHeader = NULL;
		if(myRecycledSpace != NULL)
			myRecycledSpace->myPreviousHeader = head;
		myRecycledSpace = head;
		++myBlocksAvailable;

		UNLOCK_MUTEX(myMutex);

		return next;
	}



	// --------------------------------------------------------------------------						
	// Function:	Relocate
	// Description:	moves data blocks from me to other chunk
	// Arguments:	other chunk
	// Returns:		none
	// --------------------------------------------------------------------------
	void Chunk::Relocate(Chunk& relocateTo)
	{
		FixedHeader* head = NULL;
		while (relocateTo.BlocksAvailable() && BlocksUsed())
		{
			head = (FixedHeader*)myInUseHead;
			char* source = (char*)(head + 1);
			char* target = (char*)relocateTo.Allocate();
			memcpy(target, source, myDataSize);
			FixedHeader* targetHead = &GetFixedHeader(target);
			targetHead->myMemoryInfo = head->myMemoryInfo;

			if (targetHead->myMemoryInfo)
			{
				// update any smarp pointer to point to new memory
				MemoryOffset offset = relocateTo.AddressOffset(target);
				GCInfo* gci = dynamic_cast<GCInfo*>(targetHead->myMemoryInfo);
				if (gci)
					gci->RegisterMoveSingle(&relocateTo, offset);
				else
					targetHead->myMemoryInfo->RegisterMoveSingle(&relocateTo, offset);

			}

			Deallocate(source, false);
		}
	}

}
