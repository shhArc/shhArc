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

#ifndef MEMORYINFO_H
#define MEMORYINFO_H


#ifdef _WIN64

#include <windows.h>
#undef GetObject


#define LOCKEXCHANGEGCOBJECT(a, b) InterlockedExchange64((LONGLONG*)&a, *(LONGLONG*)&b)
#define LOCKINCREMENTGCREFCOUNT(a) InterlockedIncrement64((LONGLONG*)&a)
#define LOCKDECREMENTGCREFCOUNT(a) InterlockedDecrement64((LONGLONG*)&a)
#define GETTHREADID() GetCurrentThreadId()


#else

#include <pthread.h>


#define LOCKEXCHANGEGCOBJECT(a, b) __sync_val_compare_and_swap((long long*)&a, *(long long*)&a,*(long long*)&b)
#define LOCKINCREMENTGCREFCOUNT(a) __sync_add_and_fetch(&a, 1)
#define LOCKDECREMENT64(a) __sync_add_and_fetch(a, -1)
#define GETTHREADID() pthread_self()


#endif 

#include "../Common/SecureStl.h"
#include "../Common/Exception.h"
#include "MemoryLocator.h"

namespace shh {

	class GCObjectBase;


	class MemoryInfo
	{
		friend class MemoryLocator;
		friend class MemoryPtr;
		friend class GCObjectBase;
		friend class MemoryFrame;
		friend class Chunk;

	public:

		MemoryInfo();
		MemoryInfo(void* object, bool memoryManaged, int count = 0, bool valid = true);
		MemoryInfo(MemoryLocator* l, MemoryOffset offset, bool memoryManaged, int count = 0, bool valid = true);
		virtual ~MemoryInfo();

		bool Invalidate();
	
		inline MemoryOffset GetOffset() const;
		inline char* AbsoluteAddress() const;


		inline bool IsValid() const;
		inline void SetValid(const bool valid);
		inline bool IsDying() const;
		inline void SetDying(const bool dying);
		inline void IncrementReferenceCount();
		inline int DecrementReferenceCount();
		inline int GetReferenceCount() const;
		inline bool IsMemoryManaged() const;

	protected:

		virtual	void RegisterMoveAll(MemorySize offsetDelta = 0, MemoryLocator::Direction offsetDir = MemoryLocator::None);
		virtual void RegisterMoveSingle(MemoryLocator* locator, MemoryOffset offset);

		bool myMemoryManaged;
		MemoryLocator* myLocator;
		MemoryOffset myOffset;
		int myValid;
		int myDying;
		int myReferenceCount;
		void* myObject;
		GCObjectBase* myGCObject;

	};


	// MemoryInfo Inline ///////////////////////////////////////////////////

	// --------------------------------------------------------------------------						
	// Function:	GetOffset
	// Description:	returns object address offset from memory base
	// Arguments:	none
	// Returns:		offset
	// --------------------------------------------------------------------------
	inline MemoryOffset MemoryInfo::GetOffset() const
	{
		if (IsValid())
			return myOffset;
		Exception::Throw("Bad MemoryPtr dereference");
		return MemoryOffset();
	}


	// --------------------------------------------------------------------------						
	// Function:	AbsoluteAddress
	// Description:	returns absolute address of object pointed to
	// Arguments:	none
	// Returns:		address
	// --------------------------------------------------------------------------
	inline char* MemoryInfo::AbsoluteAddress() const
	{
		if (IsValid())
			return myLocator->myBase + myOffset.value;
		Exception::Throw("Bad MemoryPtr dereference");
		return NULL;
	}


	// --------------------------------------------------------------------------						
	// Function:	IsValid
	// Description:	returns if object pointed to is valid
	// Arguments:	none
	// Returns:		true if valid
	// --------------------------------------------------------------------------
	inline bool MemoryInfo::IsValid() const 
	{ 
		return (bool)myValid; 
	}


	// --------------------------------------------------------------------------						
	// Function:	SetValid
	// Description:	sets whether object pointed to is valid
	// Arguments:	bool
	// Returns:		none
	// --------------------------------------------------------------------------
	inline void MemoryInfo::SetValid(const bool valid) 
	{ 
		int v = (int)valid;  LOCKEXCHANGEGCOBJECT(myValid, v); 
	}


	// --------------------------------------------------------------------------						
	// Function:	IsDying
	// Description:	returns whether object pointed to is being deleted
	// Arguments:	none
	// Returns:		true if being delted
	// --------------------------------------------------------------------------
	inline bool MemoryInfo::IsDying() const 
	{ 
		return (bool)myDying; 
	}


	// --------------------------------------------------------------------------						
	// Function:	SetDying
	// Description:	set whether object pointed to is being deleted
	// Arguments:	bool
	// Returns:		none
	// --------------------------------------------------------------------------
	inline void MemoryInfo::SetDying(const bool dying) 
	{ 
		int d = (int)dying; LOCKEXCHANGEGCOBJECT(myDying, d); 
	}


	// --------------------------------------------------------------------------						
	// Function:	IncrementReferenceCount
	// Description:	increments number of pointers pointing to this object
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	inline void MemoryInfo::IncrementReferenceCount() 
	{ 
		LOCKINCREMENTGCREFCOUNT(myReferenceCount); 
	}


	// --------------------------------------------------------------------------						
	// Function:	DecrementReferenceCount
	// Description:	decrements number of pointers pointing to this object
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	inline int MemoryInfo::DecrementReferenceCount() 
	{ 
		return (int)LOCKDECREMENTGCREFCOUNT(myReferenceCount); 
	}


	// --------------------------------------------------------------------------						
	// Function:	GetReferenceCount
	// Description:	gets number of pointers pointing to this object
	// Arguments:	none
	// Returns:		ref count
	// --------------------------------------------------------------------------
	inline int MemoryInfo::GetReferenceCount() const 
	{ 
		return myReferenceCount; 
	}


	// --------------------------------------------------------------------------						
	// Function:	IsMemoryManaged
	// Description:	returns if memory managed
	// Arguments:	none
	// Returns:		if managed
	// --------------------------------------------------------------------------
	inline bool MemoryInfo::IsMemoryManaged() const
	{
		return myMemoryManaged;
	}


}

#endif //MEMORYINFO_H
