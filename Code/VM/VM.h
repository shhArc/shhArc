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

#ifndef VM_H
#define VM_H



#define DEFAULT_STACK_SIZE 100

#include "../Common/SecureStl.h"
#include "../Config/GCPtr.h"
#include "Process.h"
#include "Scheduler.h"
#include <string>
#include <vector>
#include <map>
#include <queue>

namespace shh {

	class Module;
	

	class VM : public virtual GCObject
	{
		friend class Scheduler;
		friend GCPtr< VM >;

	public:

		typedef std::vector<GCPtr<Process> > Stack;
		typedef std::map<shhId, GCPtr<Process> > Processes;
		typedef std::vector< GCPtr<Module> > RegisteredModules;
		typedef std::vector<std::string> Paths;


		VM();
		VM(const GCPtr<VM> &other);
		~VM();

		GCPtr<Process> CreateMasterProcess(Implementation i, Privileges privileges = NoPrivilege);
		bool InitializeMasterProcess();
		GCPtr<Process> CreateSlaveProcess(Implementation i, Privileges privilege = NoPrivilege);

		virtual bool AddSlaveProcess(const GCPtr<Process>& slave);
		const GCPtr<Process> GetProcess(unsigned int id) const;
		virtual bool RemoveProcess(const GCPtr<Process>& p);

		virtual void SwapProcessIn(const GCPtr<Process>& process);
		virtual void SwapProcessOut();

		virtual bool Busy() const;
		virtual const GCPtr<Scheduler>& GetScheduler() const;
		virtual void AssureIntegrity(bool vmOnly = false);

		virtual bool GetArgument(Message& msg, unsigned int arg);
	
		virtual void FlagUninitialized(const GCPtr<Process>& p);
		virtual void FlagInitialized(const GCPtr<Process>& p);
		virtual bool Finalize(GCObject* me);

		inline const GCPtr<Process>& GetMasterProcess() const;
		inline const GCPtr<Process>& GetRootCallingProcess() const;
		inline const GCPtr<Process>& GetCallingProcess() const;
		inline bool IsInitialized() const;
		inline shhId GetId() const;

	protected:

		GCPtr<Scheduler> myScheduler;
		GCPtr<Process> myMasterProcess;
		Processes mySlaveProcesses;

		Stack myProcessStack;
		unsigned int myActiveProcess;

		bool myInitialized;

		virtual void Zap();
		void SetScheduler(const GCPtr<Scheduler>& s);

	private:

		shhId myId;
		unsigned int myUnintializedCount;
		static int ourStackSize;
		static shhId ourLastId;
		
	};



	// --------------------------------------------------------------------------						
	// Function:	GetMasterProcess
	// Description:	gets the main Process of this VM
	// Arguments:	none
	// Returns:		Process
	// --------------------------------------------------------------------------
	inline const GCPtr<Process>& VM::GetMasterProcess() const 
	{ 
		return myMasterProcess; 
	}


	// --------------------------------------------------------------------------						
	// Function:	GetRootCallingProcess
	// Description:	gets the root call Process, else null process
	// Arguments:	none
	// Returns:		Process
	// --------------------------------------------------------------------------
	inline const GCPtr<Process>& VM::GetRootCallingProcess() const 
	{ 
		return myProcessStack[myActiveProcess != 0 ? 1 : 0]; 
	}


	// --------------------------------------------------------------------------						
	// Function:	GetCallingProcess
	// Description:	gets the Process that was active before the current one and 
	//				and therefore must have called the current active one
	// Arguments:	none
	// Returns:		Process
	// --------------------------------------------------------------------------
	inline const GCPtr<Process>& VM::GetCallingProcess() const 
	{ 
		return myProcessStack[myActiveProcess - 1]; 
	}


	// --------------------------------------------------------------------------						
	// Function:	IsInitialized
	// Description:	test of this has been initialized
	// Arguments:	none
	// Returns:		bool
	// --------------------------------------------------------------------------
	inline bool VM::IsInitialized() const 
	{ 
		return myInitialized; 
	}


	// --------------------------------------------------------------------------						
	// Function:	GetId
	// Description:	gets the unique id of this VM
	// Arguments:	none
	// Returns:		id
	// --------------------------------------------------------------------------
	inline shhId VM::GetId() const 
	{ 
		return myId; 
	}


	// --------------------------------------------------------------------------						
	// Function:	SetScheduler
	// Description:	sets the scheduler managing this vm
	// Arguments:	scheduler
	// Returns:		none
	// --------------------------------------------------------------------------
	inline void VM::SetScheduler(const GCPtr<Scheduler>& s) 
	{ 
		myScheduler = s; 
	};

}// namespace shh
#endif

