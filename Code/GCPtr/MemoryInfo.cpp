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

#include "MemoryInfo.h"


namespace shh {


	// --------------------------------------------------------------------------						
	// Function:	MemoryInfo
	// Description:	constructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	MemoryInfo::MemoryInfo() : 
		myMemoryManaged(false),
		myLocator(NULL), 
		myValid(0), 
		myDying(0), 
		myReferenceCount(0), 
		myObject(NULL), 
		myGCObject(NULL) 
	{ 
		myOffset.value = 0; 
	}


	// --------------------------------------------------------------------------						
	// Function:	MemoryInfo
	// Description:	constructor
	// Arguments:	object to point to, reference count. if object is valid
	// Returns:		none
	// --------------------------------------------------------------------------
	MemoryInfo::MemoryInfo(void* object, bool memoryManaged, int count, bool valid) :
		myMemoryManaged(memoryManaged),
		myLocator(NULL), 
		myValid((int)valid), 
		myDying(0), 
		myReferenceCount(count), 
		myObject(object), 
		myGCObject(NULL) 
	{ 
		myOffset.value = 0; 
	}


	// --------------------------------------------------------------------------						
	// Function:	MemoryInfo
	// Description:	constructor
	// Arguments:	memory locator, offset of object, reference count. 
	//				if object is valid
	// Returns:		none
	// --------------------------------------------------------------------------
	MemoryInfo::MemoryInfo(MemoryLocator* l, MemoryOffset offset, bool memoryManaged, int count, bool valid) :
		myMemoryManaged(memoryManaged),
		myLocator(l), 
		myValid((int)valid), 
		myDying(0), myReferenceCount(count), 
		myGCObject(NULL)
	{
		LOCKEXCHANGEGCOBJECT(myOffset.ptr, offset.ptr);
		void* obj = AbsoluteAddress();
		LOCKEXCHANGEGCOBJECT(myObject, obj);
	}

	
	// --------------------------------------------------------------------------						
	// Function:	~MemoryInfo
	// Description:	destructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	MemoryInfo::~MemoryInfo()
	{
		Invalidate();
		if (myLocator)
		{

			if (MemoryLocator::IsVariableSizeData((char*)myObject))
			{
				MemoryLocator::VariableHeader& header = MemoryLocator::GetVariableHeader((char*)myObject);
				MemoryInfo *null = NULL;
				LOCKEXCHANGEGCOBJECT(header.myMemoryInfo, null);
			}
			else
			{
				MemoryLocator::FixedHeader& header = MemoryLocator::GetFixedHeader((char*)myObject);
				MemoryInfo* null = NULL;
				LOCKEXCHANGEGCOBJECT(header.myMemoryInfo, null);
			}

		}
	}


	// --------------------------------------------------------------------------						
	// Function:	Invalidate
	// Description:	invalidate the pointer object
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	bool MemoryInfo::Invalidate()
	{
		SetValid(false);
		return false;
	}


	// --------------------------------------------------------------------------						
	// Function:	RegisterMoveAll
	// Description: point all pointers to new location (batch usage)
	// Arguments:	offset, direction of move
	// Returns:		none
	// --------------------------------------------------------------------------
	void MemoryInfo::RegisterMoveAll(MemorySize offsetDelta, MemoryLocator::Direction offsetDir)
	{
		if (offsetDir == MemoryLocator::Up)
		{
			char* ptr = myOffset.ptr + offsetDelta;
			LOCKEXCHANGEGCOBJECT(myOffset.ptr, ptr);
		}
		else if (offsetDir == MemoryLocator::Down)
		{
			char* ptr = myOffset.ptr - offsetDelta;
			LOCKEXCHANGEGCOBJECT(myOffset.ptr, ptr);
		}
		void* obj = AbsoluteAddress();
		LOCKEXCHANGEGCOBJECT(myObject, obj);
	}


	// --------------------------------------------------------------------------						
	// Function:	RegisterMoveSingle
	// Description:	moves pointer to new offset (single usage)
	// Arguments:	memory locator, offset
	// Returns:		none
	// --------------------------------------------------------------------------
	void MemoryInfo::RegisterMoveSingle(MemoryLocator* locator, MemoryOffset offset)
	{
		LOCKEXCHANGEGCOBJECT(myLocator, locator);
		LOCKEXCHANGEGCOBJECT(myOffset, offset);
		void* obj = AbsoluteAddress();
		LOCKEXCHANGEGCOBJECT(myObject, obj);
	}


}
