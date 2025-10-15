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
#ifndef THREADSAFETY
#define THREADSAFETY

#include "../Config/Config.h"
#include "Debug.h"


namespace shh {


#if MULTI_THREADED

#ifdef _WIN64

#include <windows.h>
#undef GetObject
#define LOCKEXCHANGE64(a, b) InterlockedExchange64((LONGLONG*)&a, *(LONGLONG*)&b)
#define LOCKINCREMENT64(a) InterlockedIncrement64((LONGLONG*)&a)
#define LOCKDECREMENT64(a) InterlockedDecrement64((LONGLONG*)&a)
#define GETTHREADID() GetCurrentThreadId()


#else

#include <pthread.h>

#define LOCKEXCHANGE64(a, b) __sync_val_compare_and_swap((long long*)&a, *(long long*)&a,*(long long*)&b)
#define LOCKINCREMENT64(a) __sync_add_and_fetch(&a, 1)
#define LOCKDECREMENT64(a) __sync_add_and_fetch(a, -1)
#define GETTHREADID() pthread_self()

#endif 

#define LOCK_MUTEX(m) m->LockMutex()
#define UNLOCK_MUTEX(m) m->UnlockMutex()
#define SLEEP(t) Mutex::SleepThread(t)



#else

#define LOCK_MUTEX(m)
#define UNLOCK_MUTEX(m)
#define SLEEP(t)
#define LOCKEXCHANGE64(a, b) a=b
#define LOCKINCREMENT64(a) ++a
#define LOCKDECREMENT64(a) --a


#endif

#define LOCKEXCHANGE LOCKEXCHANGE64
#define LOCKINCREMENT LOCKINCREMENT64
#define LOCKDECREMENT LOCKDECREMENT64 



	// --------------------------------------------------------------------------						
	// Function:	LockExchange
	// Description:	thread safely sets a=b
	// Arguments:	arg a, arg b
	// Returns:		none
	// --------------------------------------------------------------------------
	template<class T> void LockExchange(T& a, T& b)
	{
		if (sizeof(T) == 8)
		{
			LOCKEXCHANGE64(a, b);
		}
		else
		{
			DEBUG_ASSERT(false);
			a = b;
		}
	}


	// --------------------------------------------------------------------------						
	// Function:	LockIncrement
	// Description:	thread safely increments a
	// Arguments:	arg a
	// Returns:		none
	// --------------------------------------------------------------------------
	template<class T> void LockIncrement(T& a)
	{
		if (sizeof(T) == 8)
		{
			LOCKINCREMENT64(a);
		}
		else
		{
			DEBUG_ASSERT(false);
			++a;
		}
	}



	// --------------------------------------------------------------------------						
	// Function:	LockDecrement
	// Description:	thread safely deccrements a
	// Arguments:	arg a
	// Returns:		none
	// --------------------------------------------------------------------------
	template<class T> void LockDecrement(T& a)
	{
		if (sizeof(T) == 8)
		{
			LOCKDECREMENT64(a);
		}
		else
		{
			DEBUG_ASSERT(false);
			++a;
		}
	}
}
#endif
