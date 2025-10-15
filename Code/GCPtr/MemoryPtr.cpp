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

#include "MemoryPtr.h"

namespace shh {


	// --------------------------------------------------------------------------						
	// Function:	MemoryPtr
	// Description:	constructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	MemoryPtr::MemoryPtr() : myInfo(NULL) 
	{}


	// --------------------------------------------------------------------------						
	// Function:	MemoryPtr
	// Description:	constructor
	// Arguments:	memory info to use
	// Returns:		none
	// --------------------------------------------------------------------------
	MemoryPtr::MemoryPtr(MemoryInfo* info)
	{
		LOCKEXCHANGEGCOBJECT(myInfo, info);
		if (myInfo != NULL)
			myInfo->IncrementReferenceCount();
	}


	// --------------------------------------------------------------------------						
	// Function:	MemoryPtr
	// Description:	copy constructor
	// Arguments:	memory pointer to copy
	// Returns:		none
	// --------------------------------------------------------------------------
	MemoryPtr::MemoryPtr(const MemoryPtr& p)
	{

		MemoryInfo* info = p.myInfo;
		LOCKEXCHANGEGCOBJECT(myInfo, info);
		if (myInfo != NULL)
			myInfo->IncrementReferenceCount();
	}


	// --------------------------------------------------------------------------						
	// Function:	MemoryPtr
	// Description:	desstructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	MemoryPtr::~MemoryPtr()
	{
		if (myInfo != NULL && myInfo->DecrementReferenceCount() == 0)
			delete myInfo;
	}


}
