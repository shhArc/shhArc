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

#ifdef _MSC_VER
#pragma warning( disable : 4786 4503 )
#endif


#include "../Common/Debug.h"
#include "VM.h"
#include "Scheduler.h"
#include "Messenger.h"
#include "Class.h"
#include "SoftProcess.h"



namespace shh {


	int VM::ourStackSize = DEFAULT_STACK_SIZE;
	shhId VM::ourLastId = 0;


	// --------------------------------------------------------------------------						
	// Function:	VM
	// Description:	constructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	VM::VM() :
		myId(++ourLastId),
		myScheduler(NULL),
		myUnintializedCount(0),
		myInitialized(true)
	{
		myProcessStack.resize(2);
		myProcessStack[0].SetNull();
		myActiveProcess = 0;	
	}


	// --------------------------------------------------------------------------						
	// Function:	VM
	// Description:	other to clone from
	// Arguments:	other
	// Returns:		none
	// --------------------------------------------------------------------------
	VM::VM(const GCPtr<VM>& other) :
		myId(++ourLastId),
		myUnintializedCount(0),
		myInitialized(true)
	{
		if(other->myMasterProcess.IsValid())
			myMasterProcess = other->myMasterProcess->Clone();
	
		myProcessStack.resize(2);
		myProcessStack[0].SetNull();
		myActiveProcess = 0;
	}
	

	// --------------------------------------------------------------------------						
	// Function:	~VM
	// Description:	destructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	VM::~VM()
	{
		Zap();
	}


	// --------------------------------------------------------------------------						
	// Function:	Zap
	// Description:	destroys all my processes
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void VM::Zap()
	{
		Processes::iterator it = mySlaveProcesses.begin();
		if (it != mySlaveProcesses.end())
		{
			GCPtr<Process> toDelete = it->second;
			toDelete.Destroy();
			it = mySlaveProcesses.begin();
		}
		myMasterProcess.Destroy();

		GCPtr<Scheduler> s = myScheduler;
		if (myScheduler)
			myScheduler->RemoveVM(GCPtr<VM>(this));

		s.SetNull();

	}


	// --------------------------------------------------------------------------						
	// Function:	CreateMasterProcess
	// Description:	creates a new master process
	// Arguments:	Implementation. privileges
	// Returns:		Process
	// --------------------------------------------------------------------------
	GCPtr<Process> VM::CreateMasterProcess(Implementation i, Privileges privileges)
	{
		if (myMasterProcess.IsValid())
			return myMasterProcess;
		
		privileges = (privileges & ~SlavePrivilege);
		if (privileges & AgentPrivilege && privileges & BasicPrivilege)
			privileges = (privileges & ~BasicPrivilege);

		if (!(privileges & AgentPrivilege || privileges & BasicPrivilege))
			privileges = (privileges | MasterPrivilege);
		else
			privileges = (privileges & ~MasterPrivilege);

		GCPtr<Process> p;
		if (GetScheduler()->GetBaseProcess(i, privileges, p))
		{
			myMasterProcess = p->Clone();
			myMasterProcess->SetVM(GCPtr<VM>(this));
			myMasterProcess->myPrivileges = myMasterProcess->myPrivileges | privileges;
			return myMasterProcess;
		}

		return GCPtr<Process>();
	}


	// --------------------------------------------------------------------------						
	// Function:	InitializeMasterProcess
	// Description:	Initializes master process by calling initialize function
	// Arguments:	None
	// Returns:		if initialization completed
	// --------------------------------------------------------------------------
	bool VM::InitializeMasterProcess()
	{
		return myMasterProcess->Initialize();
	}


	// --------------------------------------------------------------------------						
	// Function:	CreateSlaveProcess
	// Description:	creates a new slave process
	// Arguments:	Implementation
	// Returns:		Process
	// --------------------------------------------------------------------------
	GCPtr<Process> VM::CreateSlaveProcess(Implementation i, Privileges privileges)
	{
		privileges = (privileges & ~MasterPrivilege);
		if (privileges & AgentPrivilege && privileges & BasicPrivilege)
			privileges = (privileges & ~BasicPrivilege);

		if(!(privileges & AgentPrivilege || privileges & BasicPrivilege))
			privileges = (privileges | MasterPrivilege);
		else
			privileges = (privileges & ~MasterPrivilege);


		GCPtr<Process> p;
		if (GetScheduler()->GetBaseProcess(i, privileges, p))
		{
			GCPtr<Process> slave = p->Clone();
		
			slave->SetVM(GCPtr<VM>(this));
			mySlaveProcesses[slave->myId] = slave;
			slave->myPrivileges = slave->myPrivileges | privileges;
			return slave;
		}

		return GCPtr<Process>();
	}


	// --------------------------------------------------------------------------						
	// Function:	AddSlaveProcess
	// Description:	adds a new slave process
	// Arguments:	Process
	// Returns:		if initialization completed
	// --------------------------------------------------------------------------
	bool VM::AddSlaveProcess(const GCPtr<Process> &slave)
	{
		if (slave->GetVM().IsValid())
			return false;

		slave->myPrivileges = (slave->myPrivileges & ~MasterPrivilege);
		if (slave->myPrivileges & AgentPrivilege && slave->myPrivileges & BasicPrivilege)
			slave->myPrivileges = (slave->myPrivileges | ~BasicPrivilege);

		if (!(slave->myPrivileges & AgentPrivilege || slave->myPrivileges & BasicPrivilege))
			slave->myPrivileges = (slave->myPrivileges | SlavePrivilege);
		else
			slave->myPrivileges = (slave->myPrivileges & ~SlavePrivilege);

		slave->SetVM(GCPtr<VM>(this));
		mySlaveProcesses[slave->myId] = slave;

		return true;
	}


	// --------------------------------------------------------------------------						
	// Function:	GetProcess
	// Description:	Gets process
	// Arguments:	id
	// Returns:		process
	// --------------------------------------------------------------------------
	const GCPtr<Process> VM::GetProcess(unsigned int id) const
	{
		if (id == 1)
			return myMasterProcess;

		Processes::const_iterator it = mySlaveProcesses.find(id);
		if (it == mySlaveProcesses.end())
			return GCPtr<Process>();

		return it->second;
	}


	// --------------------------------------------------------------------------						
	// Function:	RemoveProcess
	// Description:	removed process from vm
	// Arguments:	Process
	// Returns:		if done
	// --------------------------------------------------------------------------
	bool VM::RemoveProcess(const GCPtr<Process> &p)
	{
		if (myMasterProcess == p)
		{	
			GCPtr<Module> updater;
			updater.DynamicCast(myMasterProcess);
			if (updater.IsValid())
				myScheduler->RemoveUpdater(updater);

			myMasterProcess.SetNull();
			return true;
		}

		Processes::iterator it = mySlaveProcesses.find(p->myId);
		if (it == mySlaveProcesses.end())
			return false;

		GCPtr<Module> updater;
		updater.DynamicCast(it->second);
		if (updater.IsValid())
			myScheduler->RemoveUpdater(updater);

		mySlaveProcesses.erase(it);
		return true;
	}


	// --------------------------------------------------------------------------						
	// Function:	SwapProcessIn
	// Description:	swaps the given process into the luastate
	// Arguments:	process to put in
	// Returns:		none
	// --------------------------------------------------------------------------	
	void VM::SwapProcessIn(const GCPtr<Process>& process)
	{
		myActiveProcess++;
		if (myActiveProcess >= myProcessStack.size())		
			myProcessStack.resize(myActiveProcess + ourStackSize);

		Scheduler::SetCurrentProcess(process);
		myProcessStack[myActiveProcess] = process;
	}


	// --------------------------------------------------------------------------						
	// Function:	SwapProcessOut
	// Description:	swaps the current process out
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void VM::SwapProcessOut()
	{
		if (myProcessStack[myActiveProcess].IsValid())
			myProcessStack[myActiveProcess]->AssureIntegrity();

		myActiveProcess--;
		if (myActiveProcess < myProcessStack.size() - ourStackSize)
			myProcessStack.resize(myProcessStack.size() + ourStackSize);


		if (myActiveProcess != 0)
			Scheduler::SetCurrentProcess(myProcessStack[myActiveProcess]);
		else
			Scheduler::SetCurrentProcess(GCPtr<Process>());		
	}


	// --------------------------------------------------------------------------						
	// Function:	Busy
	// Description:	return if has any running processes
	// Arguments:	none
	// Returns:		bool
	// --------------------------------------------------------------------------
	bool VM::Busy() const 
	{ 
		return myActiveProcess != 0; 
	}


	// --------------------------------------------------------------------------						
	// Function:	GetScheduler
	// Description:	returns scheduler that manages this
	// Arguments:	none
	// Returns:		scheduler
	// --------------------------------------------------------------------------
	const GCPtr<Scheduler>& VM::GetScheduler() const 
	{ 
		return myScheduler; 
	}


	// --------------------------------------------------------------------------						
	// Function:	AssureIntegrity
	// Description:	integrity checks VM
	// Arguments:	if do for vm only or other stuff too
	// Returns:		none
	// --------------------------------------------------------------------------
	void VM::AssureIntegrity(bool vmOnly) 
	{}


	// --------------------------------------------------------------------------						
	// Function:	GetArgument
	// Description:	get arguments from master process stack add adds to msg
	// Arguments:	msg, arg num
	// Returns:		if got
	// --------------------------------------------------------------------------
	bool VM::GetArgument(Message& msg, unsigned int arg)
	{ 
		return myMasterProcess->GetArgument(msg, arg); 
	}


	// --------------------------------------------------------------------------						
	// Function:	FlagUninitialized
	// Description:	flag Vm as unintialized e.g if subprocess have been created 
	//				but not yet initialized
	// Arguments:	process not intialized yet
	// Returns:		none
	// --------------------------------------------------------------------------
	void VM::FlagUninitialized(const GCPtr<Process>& p)
	{
		if (p->GetVM().GetObject() == this)
		{
			myUnintializedCount++;
			myInitialized = false;
		}
	}


	// --------------------------------------------------------------------------						
	// Function:	FlagInitialized
	// Description:	flag Vm as intialized e.g of subprocess have been created 
	//				have now been initialized
	// Arguments:	process intialized yet
	// Returns:		none
	// --------------------------------------------------------------------------
	void VM::FlagInitialized(const GCPtr<Process>& p)
	{
		if (p->GetVM().GetObject() == this && myUnintializedCount > 0)
			myUnintializedCount--;

		if (myUnintializedCount == 0)
		{
			myMasterProcess->PostInitialization();
			GCPtr<Object> o = myMasterProcess->GetObject();
			
			if (o.IsValid())
			{
				o->PostInitialization();
				const GCPtr<Class>& cls = o->GetClass();
				if (cls->HasFunction(SoftProcess::ourUpdateMessage) || cls->GetImplementation() == Engine)
					myScheduler->AddUpdater(o);
			}
			for (Processes::iterator it = mySlaveProcesses.begin(); it != mySlaveProcesses.end(); it++)
			{

				it->second->PostInitialization();
				o = it->second->GetObject();
				if (o.IsValid())
				{
					o->PostInitialization();
					const GCPtr<Class>& cls = o->GetClass();
					if (cls->HasFunction(SoftProcess::ourUpdateMessage) || cls->GetImplementation() == Engine)
						myScheduler->AddUpdater(o);
				}
			}
			myInitialized = true;
		}
	}


	// --------------------------------------------------------------------------						
	// Function:	Finalize
	// Description:	finalize VM and all its processes
	// Arguments:	pointer to this
	// Returns:		if successful
	// --------------------------------------------------------------------------
	bool VM::Finalize(GCObject* me)
	{
		if (myMasterProcess.IsValid())
			myMasterProcess->Terminate(GCPtr<Messenger>());

		Processes slaves = mySlaveProcesses;
		Processes::iterator it = slaves.begin();
		while (it != slaves.end())
		{
			if (it->second->Terminate(GCPtr<Messenger>()))
				it->second.Destroy();

			it++;
		}

		return false;
	}



} // namespace shh
