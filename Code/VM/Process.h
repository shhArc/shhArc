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

#ifndef PROCESS_H
#define PROCESS_H


#include "../Common/SecureStl.h"
#include "../Common/Mutex.h"
#include "../Config/GCPtr.h"
#include "../Arc/Registry.h"
#include "Messenger.h"





namespace shh {

	class Object;
	class Environment;

	class Process : public Messenger
	{
		friend class VM;
		friend class Agent;
		friend class ClassManager;
		friend void ConfigureProcesses(int defaultArgStackSize, int minimumNumberOfProcesses);
		friend void LockProcess();
		friend void UnlockProcess();

	public:

		static bool ClassesSpecifier(const std::string& typeName, const std::string& path, Registry::ClassSpecs& unorderedSpecs, bool recurse, bool report, const std::string& classType);
		static GCPtr<Process> Create(Privileges privileges, GCPtr<Process> spawnFrom);

		Process(Privileges privileges);
		virtual ~Process();

		virtual GCPtr<Process> Clone() = 0;


		virtual const GCPtr<VM> &GetVM() const;
		virtual const GCPtr<Scheduler>& GetScheduler() const;

		const GCPtr<Environment>& GetCurrentEnvironment() const;
		const GCPtr<Environment>& GetHomeEnvironment() const;
		void SetCurrentEnvironment(const GCPtr<Environment>& env);
		void SetHomeEnvironment(const GCPtr<Environment>& env);

		virtual bool IsYieldable() const;
		virtual bool YieldProcess(ExecutionState state, int results);
		virtual int ContinueProcess();
		virtual void TerminateProcess();
		virtual void AssureIntegrity(bool processOnly = false);
		virtual bool HasFunction(const std::string& functionName) const;
		virtual bool CanFinalize() const;
		virtual bool Initialize();
		virtual bool CompleteInitialization();
		virtual bool Terminate(const GCPtr<Messenger>& caller);
		virtual bool CompleteFinalization();

		inline Privileges GetPrivileges() const;
		inline const GCPtr<Object>& GetObject() const;
		inline void Lock();
		inline void Unlock();

	
	protected:


		Privileges myPrivileges;
		GCPtr<Object> myObject;
		GCPtr<VM> myVM;
		GCPtr<Environment> myEnvironment;
		GCPtr<Environment> myHomeEnvironment;
		bool myYieldable;
		Mutex myMutex;

		void SetObject(const GCPtr<Object>& object);
		virtual void SetVM(const GCPtr<VM>& vm);


	};


	// --------------------------------------------------------------------------						
	// Function:	GetPrivileges
	// Description:	returns privileges process has
	// Arguments:	none
	// Returns:		privileges
	// --------------------------------------------------------------------------
	inline Privileges Process::GetPrivileges() const 
	{ 
		return myPrivileges; 
	}


	// --------------------------------------------------------------------------						
	// Function:	GetObject
	// Description:	get object attached to process
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	inline const GCPtr<Object>& Process::GetObject() const
	{ 
		return myObject; 
	}


	// --------------------------------------------------------------------------						
	// Function:	Lock
	// Description:	thread locks process
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	inline void Process::Lock() 
	{ 
		myMutex.LockMutex(); 
	}


	// --------------------------------------------------------------------------						
	// Function:	Unlock
	// Description:	thread unlocks process
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	inline void Process::Unlock() 
	{ 
		myMutex.UnlockMutex(); 
	}

}

#endif	// PROCESS_H
