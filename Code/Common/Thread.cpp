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


#include "Thread.h"

namespace shh
{
	ThreadManager* ThreadManager::ourManager = NULL;

	// BaseThread //////////////////////////////////////////////////////////////////

	// --------------------------------------------------------------------------						
	// Function:	BaseThread
	// Description:	constructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	BaseThread::BaseThread()
	{
		ThreadManager::GetManager().AddThread(this);
	}


	// --------------------------------------------------------------------------						
	// Function:	~BaseThread
	// Description:	destructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	BaseThread::~BaseThread()
	{
		ThreadManager::GetManager().RemoveThread(this);
	}




	// ThreadManager ///////////////////////////////////////////////////////////////

	// --------------------------------------------------------------------------						
	// Function:	GetManager
	// Description:	get thread manager
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	ThreadManager& ThreadManager::GetManager()
	{
		if (ourManager == NULL)
			ourManager = new ThreadManager;
		return *ourManager;
	}


	// --------------------------------------------------------------------------						
	// Function:	Destroy
	// Description:	destroy thread manager
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void ThreadManager::Destroy()
	{
		if (ourManager != NULL)
		{
			delete ourManager;
			ourManager = NULL;
		}
	}


	// --------------------------------------------------------------------------						
	// Function:	~ThreadManager
	// Description:	destructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	ThreadManager::~ThreadManager()
	{
		Threads::iterator it = myThreads.begin();
		while (it != myThreads.end())
		{
			delete *it;
			Threads::iterator it = myThreads.begin();
		}
	}


	// --------------------------------------------------------------------------						
	// Function:	AddThread
	// Description:	register thread with manager
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void ThreadManager::AddThread(BaseThread* thread)
	{
		myThreads.push_back(thread);
	}


	// --------------------------------------------------------------------------						
	// Function:	RemoveThread
	// Description:	unregister thread with manager
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void ThreadManager::RemoveThread(BaseThread* thread)
	{
		Threads::iterator it = std::find(myThreads.begin(), myThreads.end(), thread);
		if (it != myThreads.end())
			myThreads.erase(it);
	}


}