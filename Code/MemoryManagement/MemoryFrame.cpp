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

#ifdef _MSC_VER
#pragma warning( disable : 4786 4503 )
#endif

#include "MemoryFrame.h"
#include "MemoryStack.h"
#include "MemoryManager.h"

namespace shh {

	Allocator* MemoryFrame::ourAllocator = MemoryManager::GetManager().GetAllocator(sizeof(MemoryFrame));



	// --------------------------------------------------------------------------						
	// Function:	MemoryFrame
	// Description:	constructor
	// Arguments:	owning stack
	// Returns:		none
	// --------------------------------------------------------------------------
	MemoryFrame::MemoryFrame(MemoryStack* owner) : 
		MemoryLocator(true), 
		myTrimReserve(0), 
		myBlanked(false) 
	{
		myBase = owner->myTop+1;
		myEnd = owner->myEnd;
		owner->myTop = myEnd;
		myTop = myBase;
		myInUseHead = (Header*)myBase;
		myOwner = owner;
		owner->Register(*this);
	}


	// --------------------------------------------------------------------------						
	// Function:	~MemoryFrame
	// Description:	destructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	MemoryFrame::~MemoryFrame()
	{
		if (myBase != NULL)
			Unreserve();
	}


	// --------------------------------------------------------------------------						
	// Function:	Destroy
	// Description:	destroys object in stack
	// Arguments:	pointer to object
	// Returns:		size of object
	// --------------------------------------------------------------------------
	MemorySize MemoryFrame::Destroy(void* value)
	{
		VariableHeader& header = GetVariableHeader(value);
		if (header.myInUse)
		{
			if (header.myMemoryInfo && header.myMemoryInfo->myGCObject)
			{
				GCObject::DestroyObjectVirtuallyIfPossible(header.myMemoryInfo->myGCObject, header.myMemoryInfo->myGCObject);
			}
			else
			{
				if (header.myDestructor != NULL)
					header.myDestructor(&header + 1);
			}
			Deallocate(value, true);
		}
	
		
		MemorySize size = header.mySize;
		memset(value, 0, size);
		new (&header)VariableHeader(this);
		header.mySize = size;
		return size;		
	}


	// --------------------------------------------------------------------------						
	// Function:	Copy
	// Description:	append ofject to end of frame
	// Arguments:	object ptr
	// Returns:		object size
	// --------------------------------------------------------------------------
	MemorySize MemoryFrame::Copy(char* value)
	{
		VariableHeader& header = GetVariableHeader(value);

		if (header.myCopyer != NULL)
			header.myCopyer(value, this);
		else
			AppendCharArray(value, header.mySize);

		return header.mySize;
	}


	// --------------------------------------------------------------------------						
	// Function:	Reserve
	// Description:	sets size of frame
	// Arguments:	frame, size
	// Returns:		if successful
	// --------------------------------------------------------------------------
	bool MemoryFrame::Reserve(MemoryFrame& s, MemorySize size)
	{
		if (IsValid())
			return false;

		myOwner = s.myOwner;	// s.myOwner == &s if it is a stack

		if (!SetSize(size))
		{
			myOwner = NULL;
			return false;
		}
		// must be set once memory is set else master sorts in wrong order
		static_cast<MemoryStack*>(myOwner)->Register(*this);

		return true;
	}


	// --------------------------------------------------------------------------						
	// Function:	Unreserve
	// Description:	clears frame and removes frame from stack
	// Arguments:	none
	// Returns:		frame was removed from stack
	// --------------------------------------------------------------------------
	bool MemoryFrame::Unreserve()
	{

		// destroy all stored data
		Clear();

		// Is normal Stack so unregister with master
		bool removedLocator = false;
		if (IsValid())
		{
			removedLocator = true;
			static_cast<MemoryStack*>(myOwner)->Unregister(*this);
		}

		myOwner = NULL;
		myBase = 0;
		myTop = 0;
		myEnd = 0;
		myInUseHead = 0;

		return removedLocator;
	}


	// --------------------------------------------------------------------------						
	// Function:	AddTrimReserve
	// Description:	adds extra reserved space to frame that cant be trimmed
	// Arguments:	reserve size
	// Returns:		none
	// --------------------------------------------------------------------------
	void MemoryFrame::AddTrimReserve(unsigned int r) 
	{ 
		myTrimReserve += r; 
	}


	// --------------------------------------------------------------------------						
	// Function:	Trim
	// Description:	removed free space at end of frame
	// Arguments:	none
	// Returns:		if successful
	// --------------------------------------------------------------------------
	bool MemoryFrame::Trim()
	{
		return GetOwningStack()->TrimFrame(this);
	}


	// --------------------------------------------------------------------------						
	// Function:	Expand
	// Description:	expand this frame, trimming and  pushing all laster frames
	//				to end of stack
	// Arguments:	none
	// Returns:		if successful
	// --------------------------------------------------------------------------
	bool MemoryFrame::Expand()
	{
		return GetOwningStack()->ExpandFrame(this);
	}


	// --------------------------------------------------------------------------						
	// Function:	SetSize
	// Description:	resize frame
	// Arguments:	size
	// Returns:		if successful
	// --------------------------------------------------------------------------
	bool MemoryFrame::SetSize(MemorySize size)
	{
		if (!IsValid())
			return false;

		// needs to be +1 else locators to this and previous Stack
		// are the same (if previous is empty) and deleting this will 
		// locators to this and delete previous locators too
		char* newBase = GetOwningStack()->myTop;


		MemorySize sizeLeft = GetOwningStack()->myEnd - newBase - 1;
		if (size == 0)
			size = sizeLeft;

		if (sizeLeft < size)
		{
			// Stack to small must grow
			size = GetOwningStack()->Grow((size - sizeLeft) + 1);	// if 0 min grow rate
			if (size == 0)
			{
				RELEASE_ASSERT(false);
				return false;
			}
			newBase = GetOwningStack()->myTop;	// mem may be re positioned
		}


		myBase = newBase;
		myTop = newBase;
		myEnd = newBase + size;
		myInUseHead = (Header*)myBase;

		GetOwningStack()->myTop = myEnd + 1;
		return true;
	}


	// --------------------------------------------------------------------------						
	// Function:	Move
	// Description:	point all pointers to new location
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void MemoryFrame::Move(MemorySize baseDelta, Direction baseDir)
	{

		RegisterMoveAll(baseDelta, baseDir, 0, None);
	}


	// --------------------------------------------------------------------------						
	// Function:	Invalidate
	// Description:	invalidate frame and zap all objects in it
	// Arguments:	none
	// Returns:		if successfull
	// --------------------------------------------------------------------------
	bool  MemoryFrame::Invalidate()
	{
		Unreserve();
		MemoryLocator::Invalidate();
		return true;	// erased locator
	}


	// --------------------------------------------------------------------------						
	// Function:	Blank
	// Description:	set zeros for entire of frame
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void MemoryFrame::Blank()
	{
		ClearRange(0, AddressOffset(myTop).value);
		myBlanked = true;
	}


	// --------------------------------------------------------------------------						
	// Function:	Clear
	// Description:	blank frame and set top to bottom
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void MemoryFrame::Clear()
	{
		if (!myBlanked)
			ClearRange(0, AddressOffset(myTop).value);
		myTop = myBase;
	}


	// --------------------------------------------------------------------------						
	// Function:	ClearRange
	// Description:	destroy all object in range
	// Arguments:	low ptr, high ptr
	// Returns:		none
	// --------------------------------------------------------------------------
	inline void MemoryFrame::ClearRange(MemorySize low, MemorySize high)
	{

		// destruct all data stored 
		char* value = AbsoluteAddress(low);
		char* top = AbsoluteAddress(high);
		while (value < top)
		{
			value += variableHeaderSize + Destroy(value + variableHeaderSize);
		};
	}


	// --------------------------------------------------------------------------						
	// Function:	AppendCharArray
	// Description:	mem copy characters array to end of frame
	// Arguments:	char pointer, size of array
	// Returns:		pointer to object copied
	// --------------------------------------------------------------------------
	char* MemoryFrame::AppendCharArray(const char* value, MemorySize size)
	{
		char* newValue = (char*)Malloc(size);
		memcpy(newValue, value, size);

		VariableHeader* header = new (&GetVariableHeader(newValue)) VariableHeader(this);
		header->mySize = size;
		return newValue;
	}


	// --------------------------------------------------------------------------						
	// Function:	AppendBlank
	// Description:	append zeros to end of frame
	// Arguments:	size
	// Returns:		pointer to appended data
	// --------------------------------------------------------------------------
	char* MemoryFrame::AppendBlank(MemorySize size)
	{
		char* newValue = (char*)Malloc(size);
		memset(newValue, 0, size);

		VariableHeader *header = new (&GetVariableHeader(newValue)) VariableHeader(this);
		header->mySize = size;
		return newValue;
	}


	// --------------------------------------------------------------------------						
	// Function:	Malloc
	// Description:	allocated memory from frame
	// Arguments:	memory size
	// Returns:		pointer to allocate d memory
	// --------------------------------------------------------------------------
	void* MemoryFrame::Malloc(MemorySize size)
	{
		DEBUG_ASSERT(IsValid());

		char* end = myTop + variableHeaderSize + size;
		if (end > myEnd)
		{
			if (((MemoryStack*)myOwner)->GetTopFrame() != this && GetOwningStack()->myEnd - GetOwningStack()->myTop >= (int)size)
			{
				// more space at end of stack
				GetOwningStack()->ShiftAbove(this);
			}
			else 
			{
				if (!GetOwningStack()->Grow(end - myEnd))
				{
					RELEASE_ASSERT(false);
					return 0;
				}

				if (((MemoryStack*)myOwner)->GetTopFrame() != this)
				{
					// more space at end of stack
					GetOwningStack()->ShiftAbove(this);
				}
				else
				{
					myEnd = GetOwningStack()->myEnd;
					GetOwningStack()->myTop = myEnd;
				}
			}
			
		}

		// placement of malloc size
		VariableHeader* header = new (myTop)VariableHeader(this);
		header->myInUse = true;
		header->mySize = size;
		myTop += variableHeaderSize;

		char* mem = myTop;
		myTop += size;

		return mem;
	}


	// --------------------------------------------------------------------------						
	// Function:	Malloc
	// Description:	removes all gaps frame
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void MemoryFrame::Defrag()
	{
		char* freePtr = myTop;
		MemorySize freeSize = 0;
		VariableHeader* nextHeader = reinterpret_cast<VariableHeader*>(myBase);
		while ((char*)nextHeader < myTop)
		{
			VariableHeader* header = nextHeader;
			nextHeader = reinterpret_cast<VariableHeader*>(((char*)nextHeader) + variableHeaderSize + nextHeader->mySize);

			if (!header->myInUse)
			{
				// if not in use add to continuous free space collected
				if (freeSize == 0)
				{
					freePtr = (char*)header;
					freeSize = header->mySize + variableHeaderSize;
				}
				else
				{
					freeSize += header->mySize + variableHeaderSize;
				}
			}
			else if(header->myInUse && freeSize > 0)
			{
				// if in use move back down to begining of free space
				MemorySize size = header->mySize;
				MemorySize diff = (char*)header - freePtr;
			
				memcpy(freePtr, header, size + variableHeaderSize);
				
				// point all pointers to new location
				if (((VariableHeader*)freePtr)->myMemoryInfo)
					((VariableHeader*)freePtr)->myMemoryInfo->RegisterMoveAll(diff, Down);

				freePtr = freePtr + variableHeaderSize + size;
			}
		}
		if (freeSize)
		{
			memset(freePtr, 0, freeSize);
			myTop = (char*)freePtr;
		}
	}

}
