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


#include "MemoryLocator.h"
#include "MemoryInfo.h"


namespace shh {

	// Memory::Sorter //////////////////////////////////////////////////////////


	// --------------------------------------------------------------------------						
	// Function:	Sorter
	// Description:	constructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	MemoryLocator::Sorter::Sorter() :
		myLocator(NULL)
	{}

	// --------------------------------------------------------------------------						
	// Function:	Sorter
	// Description:	constructor
	// Arguments:	memory locator
	// Returns:		none
	// --------------------------------------------------------------------------
	MemoryLocator::Sorter::Sorter(MemoryLocator* l) :
		myLocator(l)
	{}


	// --------------------------------------------------------------------------						
	// Function:	==
	// Description:	test if two sorters are the same
	// Arguments:	other sorter
	// Returns:		bool
	// --------------------------------------------------------------------------
	bool MemoryLocator::Sorter::operator==(const Sorter& o) const
	{
		return myLocator == o.myLocator;
	}


	// --------------------------------------------------------------------------						
	// Function:	<
	// Description:	tests if the sorters memory is less than other
	// Arguments:	other sorter
	// Returns:		bool
	// --------------------------------------------------------------------------
	bool MemoryLocator::Sorter::operator<(const Sorter& o) const
	{
		return myLocator->GetBase() < o.myLocator->GetBase();
	}


	// --------------------------------------------------------------------------						
	// Function:	<MemoryPtr>
	// Description:	tests if the sorters memory is greater than other
	// Arguments:	other sorter
	// Returns:		bool
	// --------------------------------------------------------------------------
	bool MemoryLocator::Sorter::operator>(const Sorter& o) const
	{
		return
			myLocator->GetBase() > o.myLocator->GetBase();
	}


	// MemoryLocator //////////////////////////////////////////////////////////////////////////


	// --------------------------------------------------------------------------						
	// Function:	MemoryLocator
	// Description:	constructor
	// Arguments:	if variable sized data, memory base
	// Returns:		none
	// --------------------------------------------------------------------------
	MemoryLocator::MemoryLocator(bool variableSize, char* base) :
		myTop(NULL),
		myVariableSize(variableSize)
	{
		myBase = base;
		myEnd = NULL;
		myTop = myBase;
		myInUseHead = NULL;
		myOwner = this;
	}


	// --------------------------------------------------------------------------						
	// Function:	~MemoryLocator
	// Description:	destructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	MemoryLocator::~MemoryLocator()
	{
		if (myOwner != NULL)
			myOwner->RemoveMemoryLocator(this);
	}


	// --------------------------------------------------------------------------						
	// Function:	Deallocate
	// Description:	clears header to deallocate memory
	// Arguments:	pointer to object, wherether to invalidate pointers to object
	// Returns:		header
	// --------------------------------------------------------------------------
	MemoryLocator::Header* MemoryLocator::Deallocate(void* p, bool invalidateInfo)
	{
		
		if (IsVariableSizeData(p))
		{
			VariableHeader* head = &GetVariableHeader(p);

			if (head->myMemoryInfo && invalidateInfo)
				head->myMemoryInfo->Invalidate();

			head->myMemoryInfo = NULL;
			head->myDestructor = NULL;
			head->myInUse = false;
			head->myCastable = NULL;
			head->myCopyer = NULL;

			return (Header*)((char*)head+1)+head->mySize;
		}
		else
		{
			FixedHeader* head = &GetFixedHeader(p);
			
			if(head->myMemoryInfo && invalidateInfo)
				head->myMemoryInfo->Invalidate();

			head->myMemoryInfo = NULL;
			head->myDestructor = NULL;
			
			
			// remove item from the head of the in use list
			if (myInUseHead == head)
				myInUseHead = head->myNextHeader;


			// link headers either side of deleted item to each other
			if (head->myNextHeader)
				head->myNextHeader->myPreviousHeader = head->myPreviousHeader;
			if (head->myPreviousHeader)
				head->myPreviousHeader->myNextHeader = head->myNextHeader;
		
			return head->myNextHeader;
		}
	}


	// --------------------------------------------------------------------------						
	// Function:	RegisterMoveAll
	// Description:	batch relocating of all headers in memory area, and pointers
	//				to data
	// Arguments:	delta to move of base, direction of base to move,
	//				delta to move of offset, direction of offset to move,
	// Returns:		none
	// --------------------------------------------------------------------------
	void MemoryLocator::RegisterMoveAll(MemorySize baseDelta, Direction baseDir, MemorySize offsetDelta, Direction offsetDir)
	{
		// relocate base
		if (baseDir == Up)
		{
			myBase = myBase + baseDelta;
			myEnd = myEnd + baseDelta;
			if(myTop)
				myTop = myTop + baseDelta;
			if (myInUseHead)
				myInUseHead = (Header*)((char*)myInUseHead + baseDelta);
		}
		else
		{
			myBase = myBase - baseDelta;
			myEnd = myEnd - baseDelta;
			if(myTop)
				myTop = myTop - baseDelta;
			if (myInUseHead)
				myInUseHead = (Header*)((char*)myInUseHead - baseDelta);
		}


		// relocate pointers
		if (myVariableSize)
		{
			VariableHeader* nextHeader = reinterpret_cast<VariableHeader*>(myInUseHead);
			while (nextHeader && (char*)nextHeader < myTop)
			{
				// point all pointers to new location
				if (nextHeader->myInUse && nextHeader->myMemoryInfo)
					nextHeader->myMemoryInfo->RegisterMoveAll(offsetDelta, offsetDir);

				nextHeader = (VariableHeader*)(((char*)nextHeader) + variableHeaderSize + nextHeader->mySize);
			}
		}
		else
		{
			FixedHeader* nextHeader = reinterpret_cast<FixedHeader*>(myInUseHead);
			while (nextHeader)
			{
				// point all pointers to new location
				if (nextHeader->myMemoryInfo)
					nextHeader->myMemoryInfo->RegisterMoveAll(offsetDelta, offsetDir);

				nextHeader = nextHeader->myNextHeader;
			}
		}
	}	


	// --------------------------------------------------------------------------						
	// Function:	Invalidate
	// Description:	invalidate locator
	// Arguments:	none
	// Returns:		if erased locator
	// --------------------------------------------------------------------------
	bool MemoryLocator::Invalidate()
	{
		myOwner = NULL;
		return false;
	}




}
