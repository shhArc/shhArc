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

#ifndef THREAD
#define THREAD

#include "Exception.h"
#include <vector>

#ifdef _WIN64

#define NOMINMAX
#include <windows.h>
#undef GetObject
#include <process.h>

#else

#include <pthread.h>

#endif


namespace shh
{

	// BaseTread /////////////////////////////////////////////////////////////////////

	class BaseThread
	{
		friend class ThreadManager;

	public:

		virtual ~BaseThread();
	
	protected:

		BaseThread();

	};



	// ThreadManager /////////////////////////////////////////////////////////////////

	class ThreadManager
	{
		friend class BaseThread;

	public:

		static ThreadManager& GetManager();
		static void Destroy();

	protected:

		void AddThread(BaseThread* thread);
		void RemoveThread(BaseThread* thread);

	private:

		~ThreadManager();

		typedef std::vector<BaseThread*> Threads;

		Threads myThreads;
		static ThreadManager* ourManager;
	};
 


	// Thread ////////////////////////////////////////////////////////////////////

	template<class WORKER> class Thread : public BaseThread
	{
		friend class ThreadManager;

	public:

		Thread<WORKER>();
		~Thread<WORKER>();

		// This is a pointer to a member function
		typedef void (WORKER::* MemberFunction) ();

		void BeginThread(WORKER* WORKER, MemberFunction mainFunction);
		void SignalNewWork();
		bool WaitForMoreWork();
		void EvilTerminate();

	private:

#ifdef _WIN64
		static unsigned long __stdcall StaticMain(void* data);
		unsigned int myThreadID;
		HANDLE myThreadHandle;
		HANDLE myDieNowEvent;
		HANDLE myNewWorkEvent;
#else
		static void* StaticMain(void* data);
		bool myRunningFlag;
		pthread_t myThread;
		pthread_cond_t myCondition;
		pthread_mutex_t myMutex;
		bool myDieNowFlag;
		bool myNewWorkFlag;
#endif

		WORKER* myWorker;
		MemberFunction myMainFunction;

	};




	// Thread Inlines //////////////////////////////////////////////////////////

#ifdef _WIN64

	// --------------------------------------------------------------------------						
	// Function:	Thread
	// Description:	constructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	template<class WORKER> Thread<WORKER>::Thread()
		: myDieNowEvent(NULL), myNewWorkEvent(NULL), myThreadHandle(NULL)
	{
		
	}


	// --------------------------------------------------------------------------						
	// Function:	~Thread
	// Description:	destructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	template<class WORKER> Thread<WORKER>::~Thread()
	{
		if (myDieNowEvent && myThreadHandle)
		{
			SetEvent(myDieNowEvent);
			WaitForSingleObject(myThreadHandle, INFINITE);
		}

		if (myDieNowEvent)
			CloseHandle(myDieNowEvent);
		if (myNewWorkEvent)
			CloseHandle(myNewWorkEvent);
		if (myThreadHandle)
			CloseHandle(myThreadHandle);

		delete myWorker;
	}

	// --------------------------------------------------------------------------						
	// Function:	StaticMain
	// Description:	thread entry point
	// Arguments:	pointer ro thread
	// Returns:		none
	// --------------------------------------------------------------------------
	template<class WORKER> unsigned long __stdcall Thread<WORKER>::StaticMain(void* data)
	{
		try
		{
			Thread<WORKER>* thread = (Thread<WORKER>*) data;
			((thread->myWorker)->*(thread->myMainFunction))();
		}
		catch (Exception& se)
		{

		}
		catch (std::exception& e)
		{

		}

		return 0;
	}


	// --------------------------------------------------------------------------						
	// Function:	BeginThread
	// Description:	starts thread
	// Arguments:	thread object, main function
	// Returns:		none
	// --------------------------------------------------------------------------
	template<class WORKER> void Thread<WORKER>::BeginThread(WORKER* WORKER, MemberFunction mainFunction)
	{
		myDieNowEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (!myDieNowEvent)
			Exception::Throw("Failed to make myDieNowEvent");
		myNewWorkEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (!myNewWorkEvent)
			Exception::Throw("Failed to make myNewWorkEvent");

		myWorker = WORKER;
		myMainFunction = mainFunction;

		// We use _beginthreadex instead of CreateProcess
		// See on MSDN: The MSJ Win32 Q&A July 1999 by Jeffrey Richter for why
		// (Shorthand: It's to do with C runtime library ugliness)
		typedef unsigned(__stdcall* PTHREAD_START) (void*);
		myThreadHandle = (HANDLE)_beginthreadex(
			0, // security - must be 0 on win98
			0, // stack size, or 0
			(PTHREAD_START)(&StaticMain), // start of thread execution
			(void*)this, // argument list
			0, // can be CREATE_SUSPEND or 0.
			&myThreadID); // Thread id stored in here

		if (!myThreadHandle)
			Exception::Throw("Failed to create Thread");
	}


	// --------------------------------------------------------------------------						
	// Function:	SignalNewWork
	// Description:	tells thread there is more work
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	template<class WORKER> void Thread<WORKER>::SignalNewWork()
	{
		SetEvent(myNewWorkEvent);
	}


	// --------------------------------------------------------------------------						
	// Function:	WaitForMoreWork
	// Description:	pauses thread until work is signaled
	// Arguments:	none
	// Returns:		true if we got more work, or false to die
	// --------------------------------------------------------------------------
	template<class WORKER> bool Thread<WORKER>::WaitForMoreWork()
	{
		HANDLE objects[2];
		objects[0] = myDieNowEvent;
		objects[1] = myNewWorkEvent;

		DWORD dw = WaitForMultipleObjects(2, objects, false, INFINITE);

		// Return true if we got more work, or false to die
		if (dw == WAIT_OBJECT_0 + 1)
			return true;
		else
			return false;
	}


	// --------------------------------------------------------------------------						
	// Function:	EvilTerminate
	// Description:	bailed thread
	// Arguments:	none
	// Returns:		true if we got more work, or false to die
	// --------------------------------------------------------------------------
	template<class WORKER> void Thread<WORKER>::EvilTerminate()
	{
		if (TerminateThread(myThreadHandle, FALSE) == 0)
			Exception::Throw("Evil call to TerminateThread failed");
	}





#else

	// --------------------------------------------------------------------------						
	// Function:	Thread
	// Description:	constructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	template<class WORKER> Thread<WORKER>::Thread<WORKER>() :
		myRunningFlag(false),
		myDieNowFlag(false),
		myNewWorkFlag(false)
	{
		pthread_cond_init(&myCondition, NULL);
		pthread_mutex_init(&myMutex, NULL);
	}

	// --------------------------------------------------------------------------						
	// Function:	~Thread
	// Description:	destructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	template<class WORKER> Thread<WORKER>::~Thread<WORKER>()
	{
		if (myRunningFlag)
		{
			pthread_mutex_lock(&myMutex);
			myDieNowFlag = true;
			pthread_cond_signal(&myCondition);
			pthread_mutex_unlock(&myMutex);

			// wait for thread to die.
			pthread_join(myThread, NULL);
		}
		pthread_cond_destroy(&myCondition);
		pthread_mutex_destroy(&myMutex);
	}


	// --------------------------------------------------------------------------						
	// Function:	StaticMain
	// Description:	thread entry point
	// Arguments:	pointer ro thread
	// Returns:		none
	// --------------------------------------------------------------------------
	template<class WORKER> void* Thread<WORKER>::StaticMain(void* data)
	{
		Thread<WORKER>* thread = (Thread<WORKER>*) data;
		((thread->myWorker)->*(thread->myMainFunction))();
		return 0;
	}


	// --------------------------------------------------------------------------						
	// Function:	BeginThread
	// Description:	starts thread
	// Arguments:	thread object, main function
	// Returns:		none
	// --------------------------------------------------------------------------
	template<class WORKER> void Thread<WORKER>::BeginThread(WORKER* WORKER, MemberFunction mainFunction)
	{
		myWorker = WORKER;
		myMainFunction = mainFunction;
		if (pthread_create(&myThread, NULL, StaticMain, (void*)this) != 0)
			Exception::Throw("Failed to create thread in Thread.inl");

		myRunningFlag = true;

	}


	// --------------------------------------------------------------------------						
	// Function:	SignalNewWork
	// Description:	tells thread there is more work
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	template<class WORKER> void Thread<WORKER>::SignalNewWork()
	{
		pthread_mutex_lock(&myMutex);
		myNewWorkFlag = true;
		pthread_cond_signal(&myCondition);
		pthread_mutex_unlock(&myMutex);
	}


	// --------------------------------------------------------------------------						
	// Function:	WaitForMoreWork
	// Description:	pauses thread until work is signaled
	// Arguments:	none
	// Returns:		true if we got more work, or false to die
	// --------------------------------------------------------------------------
	template<class WORKER> bool Thread<WORKER>::WaitForMoreWork()
	{
		bool ret = true;

		pthread_mutex_lock(&myMutex);
		while (!myNewWorkFlag && !myDieNowFlag)
			pthread_cond_wait(&myCondition, &myMutex);

		if (myDieNowFlag)
			ret = false;

		myNewWorkFlag = false;
		pthread_mutex_unlock(&myMutex);
		return ret;
	}

#endif


}

#endif


