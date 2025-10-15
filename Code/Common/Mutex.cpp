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


#include "Mutex.h"
#include "ThreadSafety.h"
#include "Debug.h"
#include "Exception.h"


namespace shh {



	// Mutex::Lock ///////////////////////////////////////////////////////////////////

	// --------------------------------------------------------------------------						
	// Function:	Lock
	// Description:	constructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	Mutex::Lock::Lock() :
		myActiveObjects(0),
		myYieldedObjects(0),
		myLocked(false)
	{
		myMutex = new Mutex;
	}


	// --------------------------------------------------------------------------						
	// Function:	~Lock
	// Description:	destructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	Mutex::Lock::~Lock()
	{
		delete myMutex;
	}


	// --------------------------------------------------------------------------						
	// Function:	IncrementActiveObjects
	// Description:	increments number of active
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void Mutex::Lock::IncrementActiveObjects()
	{
		LOCK_MUTEX(myMutex);
		++myActiveObjects;
		UNLOCK_MUTEX(myMutex);

	}


	// --------------------------------------------------------------------------						
	// Function:	IncrementActiveObjects
	// Description:	decrements number of active
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void Mutex::Lock::DecrementActiveObjects()
	{
		LOCK_MUTEX(myMutex);
		--myActiveObjects;
		UNLOCK_MUTEX(myMutex);
	}


	// --------------------------------------------------------------------------						
	// Function:	WaitUntilUnlocked
	// Description:	wait until lock is released (unless given object has lock)
	// Arguments:	object
	// Returns:		none
	// --------------------------------------------------------------------------
	void Mutex::Lock::WaitUntilUnlocked(void *o)
	{
#if MULTI_THREADED || MULTI_THREADED

		LOCK_MUTEX(myMutex);
		if (myLocked && myLockingObject != o)
		{
			++myYieldedObjects;
			UNLOCK_MUTEX(myMutex);
			while (true)
			{
				LOCK_MUTEX(myMutex);
				if (!myLocked)
				{
					--myYieldedObjects;
					UNLOCK_MUTEX(myMutex);
					return;
				}
				UNLOCK_MUTEX(myMutex);
				SLEEP(0);
			}
		}
		UNLOCK_MUTEX(myMutex);
#endif
	}


	// --------------------------------------------------------------------------						
	// Function:	LockObject
	// Description:	waits until given object has lock
	// Arguments:	object
	// Returns:		none
	// --------------------------------------------------------------------------
	void Mutex::Lock::LockObject(void *o)
	{
#if MULTI_THREADED || MULTI_THREADED

		LOCK_MUTEX(myMutex);
		if (myLocked)
		{
			++myYieldedObjects;
			UNLOCK_MUTEX(myMutex);
			while (true)
			{
				LOCK_MUTEX(myMutex);
				if (!myLocked)
				{
					myLocked = true;
					myLockingObject = o;
					UNLOCK_MUTEX(myMutex);

					while (true)
					{
						LOCK_MUTEX(myMutex);
						if (myYieldedObjects == myActiveObjects)
						{
							UNLOCK_MUTEX(myMutex);
							return;
						}
						UNLOCK_MUTEX(myMutex);
						SLEEP(0);
					}
					return;
				}
				else
				{
					UNLOCK_MUTEX(myMutex);
					SLEEP(0);
				}
			}
		}
		else
		{
			++myYieldedObjects;
			myLocked = true;
			myLockingObject = o;
		}
		UNLOCK_MUTEX(myMutex);
#endif
	}


	// --------------------------------------------------------------------------						
	// Function:	UnlockObject
	// Description:	unlocks
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void Mutex::Lock::UnlockObject()
	{
#if MULTI_THREADED || MULTI_THREADED

		LOCK_MUTEX(myMutex);
		if (myLocked)
		{
			myLocked = false;
			--myYieldedObjects;
		}
		UNLOCK_MUTEX(myMutex);
#endif
	}


	// --------------------------------------------------------------------------						
	// Function:	Locked
	// Description:	test if locked
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	bool Mutex::Lock::Locked() const
	{ 
		return myLocked; 
	}



	// Mutex ///////////////////////////////////////////////////////////////////



#ifdef _WIN64


	// --------------------------------------------------------------------------						
	// Function:	Mutex
	// Description:	constructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	Mutex::Mutex()
	{
		myMutex = CreateMutex(NULL, false, NULL);
		if (myMutex == (HANDLE)ERROR_INVALID_HANDLE)
		{
			Exception::Throw("Couldn't create Win mutex");
		}
	}


	// --------------------------------------------------------------------------						
	// Function:	~Mutex
	// Description:	destructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	Mutex::~Mutex()
	{
		if (CloseHandle(myMutex) == 0)
		{
			Exception::Throw("Couldn't close Win mutex");
		}
	}


	// --------------------------------------------------------------------------						
	// Function:	LockMutex
	// Description: locks the mutex
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void Mutex::LockMutex()
	{
		if (WaitForSingleObject(myMutex, INFINITE) != WAIT_OBJECT_0)
		{
			Exception::Throw("Couldn't lock Win mutex");
		}
	}


	// --------------------------------------------------------------------------						
	// Function:	UnlockMutex
	// Description:	unlocks mutex
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void Mutex::UnlockMutex()
	{
		if (ReleaseMutex(myMutex) == 0)
		{
			Exception::Throw("Couldn't release Win mutex");
		}
	}


	// --------------------------------------------------------------------------						
	// Function:	SleepThread
	// Description:	sleeps for a given amount of time
	// Arguments:	time
	// Returns:		none
	// --------------------------------------------------------------------------
	void Mutex::SleepThread(int t)
	{
		Sleep(t);
	}

#else // pthread


	// --------------------------------------------------------------------------						
	// Function:	Mutex
	// Description:	constructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	Mutex::Mutex()
	{
		int ret = pthread_mutex_init(&myMutex, 0 /* attribute */);
		if (ret != 0)
			Exception::Throw("Failed to create pthread mutex - unexpected non zero return value");
	}


	// --------------------------------------------------------------------------						
	// Function:	~Mutex
	// Description:	destructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	Mutex::~Mutex()
	{
		int ret = pthread_mutex_destroy(&myMutex);
		if (ret != 0)
			Exception::Throw("Failed to destroy pthread mutex");
	}



	// --------------------------------------------------------------------------						
	// Function:	LockMutex
	// Description: locks the mutex
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void Mutex::LockMutex()
	{
		int ret = pthread_mutex_lock(&myMutex);
		if (ret != 0)
			Exception::Throw("Failed to lock pthread mutex");
	}


	// --------------------------------------------------------------------------						
	// Function:	UnlockMutex
	// Description:	unlocks mutex
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void Mutex::UnlockMutex()
	{
		int ret = pthread_mutex_unlock(&myMutex);
		if (ret != 0)
			Exception::Throw("Failed to unlock pthread mutex");
	}

	// --------------------------------------------------------------------------						
	// Function:	SleepThread
	// Description:	sleeps for a given amount of time
	// Arguments:	time
	// Returns:		none
	// --------------------------------------------------------------------------
	void Mutex::SleepThread(int t)
	{
		sleep(t);
	}
#endif

}
