//////////////////////////////////////////////////////////////////////////////
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
#include "MemoryStack.h"
#include "MemoryFrame.h"
#include "MemoryManager.h"
#include <algorithm>

namespace shh {

	unsigned int MemoryStack::ourDefaultAllocatorBlocks = 64;
	unsigned int MemoryStack::ourGrowRate = 0;


	// MemoryStack ////////////////////////////////////////////////////////////

	// --------------------------------------------------------------------------						
	// Function:	MemoryStack
	// Description:	constructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	MemoryStack::MemoryStack() :
		MemoryLocator(true, NULL),
		myAllocatorBlocks(ourDefaultAllocatorBlocks),
		myIsDeleteableMemory(false),
		myMemory(NULL),
		myMinSize(0)
	{
	}


	// --------------------------------------------------------------------------						
	// Function:	MemoryStack
	// Description:	constructor
	// Arguments:	memory size, max number of items able to store in chunk
	// Returns:		none
	// --------------------------------------------------------------------------
	MemoryStack::MemoryStack(unsigned int size, unsigned int allocatorBlocks) :
		MemoryLocator(true, NULL),
		myAllocatorBlocks(allocatorBlocks == 0 ? ourDefaultAllocatorBlocks : allocatorBlocks),
		myIsDeleteableMemory(false),
		myMemory(NULL),
		myMinSize(size)
	{
		Resize(size);
	}


	// --------------------------------------------------------------------------						
	// Function:	SetMemory
	// Description:	reseize memory
	// Arguments:	memory size, max number of items able to store in chunk
	// Returns:		none
	// --------------------------------------------------------------------------
	void MemoryStack::SetMemory(unsigned int size, unsigned int allocatorBlocks)
	{
		myAllocatorBlocks = allocatorBlocks == 0 ? ourDefaultAllocatorBlocks : allocatorBlocks;
		Resize(size);
		myMinSize = size;
	}


	// --------------------------------------------------------------------------						
	// Function:	SetMemory
	// Description:	reseize memory
	// Arguments:	memory preallocated, memory size, 
	//				max number of items able to store in chunk
	// Returns:		none
	// --------------------------------------------------------------------------
	void MemoryStack::SetMemory(char* mem, unsigned int size, unsigned int allocatorBlocks)
	{
		myAllocatorBlocks = allocatorBlocks == 0 ? ourDefaultAllocatorBlocks : allocatorBlocks;
		myIsDeleteableMemory = false;
		myBase = mem;
		myTop = myBase;
		myEnd = myBase + size;
		myOwner = this;
		myMinSize = size;
	}


	// --------------------------------------------------------------------------						
	// Function:	~MemoryStack
	// Description:	destructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	MemoryStack::~MemoryStack()
	{
		Clean();
	}


	// --------------------------------------------------------------------------						
	// Function:	Clean
	// Description:	clear memory, invalidate pointers and delte memory
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void MemoryStack::Clean()
	{

	
		// fast unregistering of last added MemoryFrame
		while (!myStack.empty())
			delete GetTopFrame();
		
		while (!myStack.empty())
		{
			// invalidate other may also remove locator if it 
			// is a memory block so take copy (deleteing nulled iterator will do nothing)
			Stack::iterator it = myStack.begin();
			bool removed = (*it)->Invalidate();
			if (!removed)
				myStack.erase(it);
		}

		Delete(myBase);
		myIsDeleteableMemory = false;
		myMemory = NULL;
		myBase = 0;
		myTop = 0;
		myEnd = 0;

	}


	// --------------------------------------------------------------------------						
	// Function:	New
	// Description:	allocate memory for stack
	// Arguments:	memory size
	// Returns:		none
	// --------------------------------------------------------------------------
	char* MemoryStack::New(MemorySize size)
	{
		if (ourGrowRate > 0)
		{
			MemorySize aligned = ((size / ourGrowRate) * ourGrowRate);
			size = (aligned < size) ? aligned + ourGrowRate : aligned;
		}
		Allocator* allocator = MemoryManager::GetManager().GetAllocator(size + sizeof(MemoryStack*), myAllocatorBlocks);
		myMemory = (char*)allocator->Allocate();
		MemoryStack** header = (MemoryStack**)myMemory;
		*header = this;
		return myMemory + sizeof(MemoryStack*);
	}



	// --------------------------------------------------------------------------						
	// Function:	Delete
	// Description:	delete memory
	// Arguments:	memory base
	// Returns:		none
	// --------------------------------------------------------------------------
	void MemoryStack::Delete(char* oldBase)
	{
		if (oldBase == 0 || !myIsDeleteableMemory)
			return;

		char* memory = oldBase - sizeof(MemoryStack*);
		Allocator::Deallocate(memory);
	}


	// --------------------------------------------------------------------------						
	// Function:	Grow
	// Description:	grow memory stack size
	// Arguments:	min size
	// Returns:		none
	// --------------------------------------------------------------------------
	MemorySize MemoryStack::Grow(MemorySize min)
	{
		if (ourGrowRate > 0)
		{
			MemorySize aligned = ((min / ourGrowRate) * ourGrowRate);
			if (aligned < min || min == 0)
				aligned += ourGrowRate;
			min = aligned;

			return Resize((myEnd - myBase + 1) + min);
		}
		return 0;
	}


	// --------------------------------------------------------------------------						
	// Function:	Shrink
	// Description:	shrink memory stack size
	// Arguments:	whether to trim free space at end
	// Returns:		if successful
	// --------------------------------------------------------------------------
	bool MemoryStack::Shrink(bool trimAll)
	{
		if (trimAll)
		{
			MemorySize size = (myEnd - myBase + 1);
			MemorySize unused = size - (myTop - myBase + 1);
			Resize(size - unused);
		}
		else if (ourGrowRate > 0)
		{
			MemorySize size = (myEnd - myBase + 1);
			if (size > myMinSize)
			{
				MemorySize used = myTop - myBase + 1;
				MemorySize aligned = ((used / ourGrowRate) * ourGrowRate);
				if (aligned < size || size == 0)
					aligned += ourGrowRate;

				if (aligned > 0 && aligned < size)
				{
					Resize(aligned);
					return true;
				}
			}
		}

		return false;
	}


	// --------------------------------------------------------------------------						
	// Function:	Resize
	// Description:	resize stack memory
	// Arguments:	new size
	// Returns:		none
	// --------------------------------------------------------------------------
	MemorySize MemoryStack::Resize(MemorySize size)
	{
		if (size == 0 || size == myEnd - myBase)
			return size;

		if (size < (MemorySize)(myTop - myBase + 1))
		{
			size = myTop - myBase + 1;
			return size;
		}
		char* oldBase = myBase;
		char* newBase = New(size);
		memset(newBase, 0, size);
		MemorySize baseOffset = newBase > myBase ? newBase - myBase : myBase - newBase;
		Direction baseDir = newBase > myBase ? Up : Down;

		// copy data and zap
		if (oldBase != 0)
		{
			MemorySize dataSize = myTop - myBase + 1;
			memcpy(newBase, myBase, dataSize);
			Delete(oldBase);
		}
		myIsDeleteableMemory = true;

		// move all my data refs to new locations
		Move(baseOffset, baseDir);
		myEnd = newBase + size - 1;


		return size;
	}


	// --------------------------------------------------------------------------						
	// Function:	Move
	// Description:	move stack memory (also readdresses all pointers)
	// Arguments:	delta to move, direction to move
	// Returns:		none
	// --------------------------------------------------------------------------
	void MemoryStack::Move(MemorySize baseDelta, MemoryLocator::Direction baseDir)
	{
		if (baseDelta == 0)
			return;

		if (baseDir == Up)
		{
			myTop = myTop + baseDelta;
			myEnd = myEnd + baseDelta;
			myBase = myBase + baseDelta;
		}
		else
		{
			myTop = myTop - baseDelta;
			myEnd = myEnd - baseDelta;
			myBase = myBase - baseDelta;
		}
		myInUseHead = (Header*)myBase;

		// point all pointers to new location
		for (Stack::iterator r = myStack.begin(); r != myStack.end(); r++)
			(*r)->RegisterMoveAll(baseDelta, baseDir);
		
		for (int c = 0; c != myMemoryMovedCallbacks.size(); c++)
			myMemoryMovedCallbacks[c]->MemoryMoved(this, baseDelta, baseDir);

	}


	// --------------------------------------------------------------------------						
	// Function:	TrimFrame
	// Description:	remove free space at end of a frame
	// Arguments:	frame
	// Returns:		if successful
	// --------------------------------------------------------------------------
	bool MemoryStack::TrimFrame(MemoryFrame* t)
	{
		MemorySize diff = t->myEnd - t->myTop;
		if (diff == 0)
			return false;

		DEBUG_ASSERT(t->myTop + t->myTrimReserve < t->myEnd);
		char* oldEnd = t->myEnd;
		t->myEnd = t->myTop + t->myTrimReserve;
		t->myTrimReserve = 0;
		char *base = t->myEnd + 1;

		Stack::iterator r = std::find(myStack.begin(), myStack.end(), t);
		r++;
		while(r != myStack.end())
		{
			MemoryFrame* mf = *r;

			// trim
			MemorySize dataSize = mf->myTop - mf->myBase + 1;
			
			// copy down in stack
			memcpy(base, mf->myBase, dataSize);		

			// point all pointers to new location
			(*r)->RegisterMoveAll(diff, Down); 

			base = mf->myEnd + 1;
			r++;
		}

		myTop = base-1;

		return true;
	}


	// --------------------------------------------------------------------------						
	// Function:	ExpandFrame
	// Description:	expand given frame to end of stack (whilst also trimming all 
	//				those below)
	// Arguments:	frame
	// Returns:		if successful
	// --------------------------------------------------------------------------
	bool MemoryStack::ExpandFrame(MemoryFrame* e)
	{
		MemorySize diff = myEnd - myTop;
		if (diff <= 1)
			return false;

		char* bottom = e->myTop;


		int s = (int)myStack.size() - 1;
		while (myStack[s] != e)
		{
			// trim frame
			MemoryFrame* mf = myStack[s];
			MemorySize dataSize = mf->myTop - mf->myBase + 1;
			char* tmp = new char[dataSize];
			//copy data down
			memcpy(tmp, mf->myBase, dataSize);
			memcpy(mf->myBase + diff, tmp, dataSize);
			delete tmp;

			// point all pointers to new location
			mf->RegisterMoveAll(diff, Up);
			s--;
		}

		myTop = myEnd;
		e->myEnd += diff;
		memset(e->myTop, 0, e->myEnd-e->myTop+1);
		return true;
	}


	// --------------------------------------------------------------------------						
	// Function:	ShiftAbove
	// Description:	trim all frames above and push them towards end of memory
	// Arguments:	frame
	// Returns:		none
	// --------------------------------------------------------------------------
	void MemoryStack::ShiftAbove(MemoryFrame* f)
	{
		Stack::iterator it = myStack.end();
		it--;
		myTop = myEnd;
		(*it)->myEnd = myTop;
		while(*it != f)
		{
			MemoryFrame *mf = *it;
			MemorySize diff = mf->myEnd - mf->myTop;
			if (diff == 0)
			{
				it--;
				continue;
			}

			MemorySize dataSize = mf->myTop - mf->myBase + 1;
			char* tmp = new char[dataSize];
			memcpy(tmp, mf->myBase, dataSize);
			memcpy(mf->myBase + diff, tmp, dataSize);
			delete tmp;

			// point all pointers to new location
			mf->RegisterMoveAll(diff, Up);

			it--;
			(*it)->myEnd = mf->myBase-1;
		}
	}


	// --------------------------------------------------------------------------						
	// Function:	Defrag
	// Description:	defragment all frames
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void MemoryStack::Defrag()
	{
		for (int s = 0; s != myStack.size(); s++)
			myStack[s]->Defrag();
	}


	// --------------------------------------------------------------------------						
	// Function:	Pack
	// Description:	defrag and trim free space in all frames, then remove
	//				free space at end of memory
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void MemoryStack::Pack()
	{
		for (int s =(int) myStack.size() - 1; s >= 0; s--)
		{
			myStack[s]->Defrag();
			myStack[s]->Trim();
		}
		Shrink();
	}


	// --------------------------------------------------------------------------						
	// Function:	AddFrame
	// Description:	creates new frame at end
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	MemoryFrame* MemoryStack::AddFrame()
	{
		return new MemoryFrame(this);
	}


	// --------------------------------------------------------------------------						
	// Function:	GetTopFrame
	// Description:	get frame at end of stack
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	MemoryFrame* MemoryStack::GetTopFrame() const
	{ 
		return myStack[myStack.size() - 1]; 
	}
	

	// --------------------------------------------------------------------------						
	// Function:	AddMemoryMoveCallback
	// Description:	add a function to call when memory is moved
	// Arguments:	callback object
	// Returns:		none
	// --------------------------------------------------------------------------
	void MemoryStack::AddMemoryMoveCallback(const GCPtr<MoveCallbackInterface>& c) 
	{ 
		myMemoryMovedCallbacks.push_back(c); 
	}


	// --------------------------------------------------------------------------						
	// Function:	Register
	// Description:	add frame to frame list
	// Arguments:	frame
	// Returns:		none
	// --------------------------------------------------------------------------
	void MemoryStack::Register(MemoryFrame& f)
	{
		myStack.push_back(&f);
	}


	// --------------------------------------------------------------------------						
	// Function:	Sorter
	// Description:	remove frame from stack (and invalidate pointers etc)
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void MemoryStack::Unregister(MemoryFrame& f)
	{
		MemorySize baseDelta = f.myEnd - f.myBase;
		Direction baseDir = Down;

		// fast unregistering of last added MemoryFrame
		if (GetTopFrame() == &f)
		{
			myStack.pop_back();
			myTop = myTop - baseDelta;
			return;
		}


		// slower unregistering 
		Stack::iterator unregistering = std::find(myStack.begin(), myStack.end(), &f);
		DEBUG_ASSERT(unregistering != myStack.end());

		Stack::iterator it = unregistering;
		it++;
		if (myTop > f.myEnd)
		{
			memcpy(f.myBase, f.myEnd + 1, myTop - f.myEnd);

			while (it != myStack.end())
			{
				// point all pointers to new location
				(*it)->RegisterMoveAll(baseDelta, baseDir);
				it++;
			}
		}
		myStack.erase(unregistering);
		myTop = myTop - baseDelta;

	}


	// --------------------------------------------------------------------------						
	// Function:	Malloc
	// Description:	alloc new frame
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void* MemoryStack::Malloc(MemorySize size)
	{
		DEBUG_ASSERT(!myStack.empty());
		MemoryFrame* mf = myStack[myStack.size() - 1];
		return mf->Malloc(size);
	}

}
