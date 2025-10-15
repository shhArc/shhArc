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

#ifndef MEMORYPTR_H
#define MEMORYPTR_H



#include "../Common/SecureStl.h"
#include "../Common/Exception.h"
#include "MemoryInfo.h"

namespace shh {

	class MemoryPtr
	{
	public:

		MemoryPtr();
		MemoryPtr(MemoryInfo* info);
		MemoryPtr(const MemoryPtr& p);
		~MemoryPtr();

		inline void operator=(const MemoryPtr& p);
		inline bool IsValid() const;
		inline bool IsDying() const;

		inline bool operator==(const MemoryPtr& other) const;

		inline MemoryOffset GetOffset() const;
		inline char* AbsoluteAddress() const;

		inline MemoryInfo* GetInfo() const;

		inline operator bool() const;

		// compare the address of the count object so that a pointer to a deleted
		// object maintains sort order.
		inline bool operator<(MemoryPtr const& other) const;

	protected:

		MemoryInfo* myInfo;
		
	};



	// MemoryPtr Inline //////////////////////////////////////////////////////////////

	// --------------------------------------------------------------------------						
	// Function:	=
	// Description:	assign operator
	// Arguments:	object to assign from
	// Returns:		none
	// --------------------------------------------------------------------------
	inline void MemoryPtr::operator=(const MemoryPtr& p)
	{
		if (myInfo != NULL)
		{
			if (myInfo->DecrementReferenceCount() == 0)
				delete myInfo;
		}
		MemoryInfo* info = p.myInfo;
		LOCKEXCHANGEGCOBJECT(myInfo, info);
		if (myInfo != NULL)
			myInfo->IncrementReferenceCount();
	}


	// --------------------------------------------------------------------------						
	// Function:	IsValid
	// Description:	test if object pointed to is valid
	// Arguments:	none
	// Returns:		bool
	// --------------------------------------------------------------------------
	inline bool MemoryPtr::IsValid() const 
	{ 
		return myInfo != NULL && myInfo->IsValid(); 
	}


	// --------------------------------------------------------------------------						
	// Function:	IsDying
	// Description:	test if object pointed to is being delted
	// Arguments:	none
	// Returns:		bool
	// --------------------------------------------------------------------------
	inline bool MemoryPtr::IsDying() const 
	{ 
		return myInfo != NULL && myInfo->IsDying(); 
	}


	// --------------------------------------------------------------------------						
	// Function:	==
	// Description:	test if pointers are pointing to the same object
	// Arguments:	other object
	// Returns:		true if same
	// --------------------------------------------------------------------------
	inline bool MemoryPtr::operator==(const MemoryPtr& other) const 
	{ 
		return myInfo == other.myInfo; 
	}


	// --------------------------------------------------------------------------						
	// Function:	GetOffset
	// Description:	returns the memory offset of the object frome memory base
	// Arguments:	none
	// Returns:		memory offest
	// --------------------------------------------------------------------------
	inline MemoryOffset MemoryPtr::GetOffset() const
	{
		if (myInfo != NULL)
			return myInfo->GetOffset();
		Exception::Throw("Bad MemoryPtr dereference");
	}


	// --------------------------------------------------------------------------						
	// Function:	AbsoluteAddress
	// Description:	returns the absolute memory address of the object (base+offset)
	// Arguments:	none
	// Returns:		pointer
	// --------------------------------------------------------------------------
	inline char* MemoryPtr::AbsoluteAddress() const
	{
		if (myInfo != NULL)
			return myInfo->AbsoluteAddress();
		Exception::Throw("Bad MemoryPtr dereference");
	}


	// --------------------------------------------------------------------------						
	// Function:	GetInfo
	// Description:	returns memory pointer info
	// Arguments:	none
	// Returns:		memory info pointer
	// --------------------------------------------------------------------------
	inline MemoryInfo* MemoryPtr::GetInfo() const 
	{ 
		return myInfo; 
	}


	// --------------------------------------------------------------------------						
	// Function:	bool
	// Description:	returns memory pointer is valid
	// Arguments:	none
	// Returns:		true if valid
	// --------------------------------------------------------------------------
	inline MemoryPtr::operator bool() const 
	{ 
		return IsValid(); 
	}

	
	// --------------------------------------------------------------------------						
	// Function:	<
	// Description:	test if object address if less than other pointer
	// Arguments:	other pointer
	// Returns:		true if less
	// --------------------------------------------------------------------------
	inline bool MemoryPtr::operator<(MemoryPtr const& other) const
	{
		return myInfo < other.myInfo;
	}
}

#endif //MEMORYPTR_H
