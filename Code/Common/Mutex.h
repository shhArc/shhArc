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


#ifndef MUTEX_H
#define MUTEX_H


#ifdef _WIN64
	#define _WIN64_LEAN_AND_MEAN
	#include <windows.h>
	#undef GetObject
#else
	#include <pthread.h>
#endif

#include "SecureStl.h"
#include "Exception.h"

namespace shh {

	class Mutex
	{
	public:


		class Lock
		{
		public:

			Lock();
			~Lock();
			void IncrementActiveObjects();
			void DecrementActiveObjects();
			void WaitUntilUnlocked(void *o);
			void LockObject(void *o);
			void UnlockObject();
			bool Locked() const;

		private:

			int myActiveObjects;
			int myYieldedObjects;
			bool myLocked;
			void *myLockingObject;
			Mutex* myMutex;

		};

		Mutex();
		~Mutex();

		void LockMutex();
		void UnlockMutex();
		void SleepThread(int t);

	private:
#ifdef _WIN64
		HANDLE myMutex;
#else
		pthread_mutex_t myMutex;
#endif
	};

}
#endif // MUTEX_H

