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


#include "../File/BinaryFile.h"
#include "../Arc/Module.h"
#include "../Arc/Environment.h"
#include "Process.h"
#include "VM.h"
#include "Object.h"



namespace shh {


	// Process ///////////////////////////////////////////////////////////////////

	// --------------------------------------------------------------------------						
	// Function:	ClassesSpecifier
	// Description:	dummy function
	// Arguments:	lua state, typename (Agent/Node), path, class specs to return
	//				whether to recurse, class name as appears in script
	// Returns:		if successful
	// --------------------------------------------------------------------------
	bool Process::ClassesSpecifier(const std::string& typeName, const std::string& path, Registry::ClassSpecs& unorderedSpecs, bool recurse, bool report, const std::string& classType) 
	{ 
		return true; 
	}


	// --------------------------------------------------------------------------						
	// Function:	Create
	// Description:	creates a Process
	// Arguments:	privileges of process, Process to derive from
	// Returns:		created process
	// --------------------------------------------------------------------------
	GCPtr<Process> Process::Create(Privileges privileges, GCPtr<Process> spawnFrom)
	{
		GCPtr<Process> oldProcess = Scheduler::GetCurrentProcess();
		GCPtr<Process> newProcess = spawnFrom->Clone();
		Scheduler::SetCurrentProcess(oldProcess);
		return newProcess;
	}


	// --------------------------------------------------------------------------						
	// Function:	Process
	// Description:	constructor
	// Arguments:	privileges this has
	// Returns:		none
	// --------------------------------------------------------------------------
	Process::Process(Privileges privileges) :
		myPrivileges(privileges),
		myYieldable(true)
	{

	}


	// --------------------------------------------------------------------------						
	// Function:	Process
	// Description:	destructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	Process::~Process()
	{
		if (myVM.IsValid())
			myVM->RemoveProcess(GCPtr<Process>(this));
	}


	// --------------------------------------------------------------------------						
	// Function:	GetVM
	// Description:	returns the VM this process belongs to
	// Arguments:	none
	// Returns:		vm
	// --------------------------------------------------------------------------
	const GCPtr<VM> &Process::GetVM() const
	{
		return myVM;
	}


	// --------------------------------------------------------------------------						
	// Function:	GetScheduler
	// Description:	returns scheduler myVM belongs to
	// Arguments:	none
	// Returns:		scheduler
	// --------------------------------------------------------------------------
	const GCPtr<Scheduler>& Process::GetScheduler() const
	{
		return myVM->GetScheduler();
	}


	// --------------------------------------------------------------------------						
	// Function:	GetCurrentEnvironment
	// Description:	gets environment the process is currently in
	// Arguments:	none
	// Returns:		environment
	// --------------------------------------------------------------------------
	const GCPtr<Environment>& Process::GetCurrentEnvironment() const
	{ 
		return myEnvironment; 
	}


	// --------------------------------------------------------------------------						
	// Function:	GetHomeEnvironment
	// Description:	gets home environment the process
	// Arguments:	none
	// Returns:		environment
	// --------------------------------------------------------------------------
	const GCPtr<Environment>& Process::GetHomeEnvironment() const
	{
		return myHomeEnvironment;
	}


	// --------------------------------------------------------------------------						
	// Function:	SetCurrentEnvironment
	// Description:	gets environment the process is currently in
	// Arguments:	environment
	// Returns:		none
	// --------------------------------------------------------------------------
	void Process::SetCurrentEnvironment(const GCPtr<Environment> &env)
	{ 
		myEnvironment = env;
	}


	// --------------------------------------------------------------------------						
	// Function:	SetHomeEnvironment
	// Description:	sets environment process was created in
	// Arguments:	environment to return
	// Returns:		none
	// --------------------------------------------------------------------------
	void Process::SetHomeEnvironment(const GCPtr<Environment>& env)
	{
		myHomeEnvironment = env;
	}


	// --------------------------------------------------------------------------						
	// Function:	IsYieldable
	// Description:	returns if process can be timed out or yielded
	// Arguments:	none
	// Returns:		bool
	// --------------------------------------------------------------------------
	bool Process::IsYieldable() const 
	{ 
		return true; 
	}


	// --------------------------------------------------------------------------						
	// Function:	YieldProcess
	// Description:	yields the process
	// Arguments:	state to set to when yielded, number of return args in current
	//				function call
	// Returns:		if yielded
	// --------------------------------------------------------------------------
	bool Process::YieldProcess(ExecutionState state, int results) 
	{
		myState = state; 
		return true; 
	}


	// --------------------------------------------------------------------------						
	// Function:	ContinueProcess
	// Description:	continue after yielding
	// Arguments:	none
	// Returns:		number of return args required for last active function
	//				when it was yielded
	// --------------------------------------------------------------------------
	int Process::ContinueProcess() 
	{ 
		myState = ExecutionContinue; 
		return 0; 
	}


	// --------------------------------------------------------------------------						
	// Function:	TerminateProcess
	// Description:	destorys the process
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void Process::TerminateProcess() 
	{ 
		myState = ExecutionTerminate; 
	}


	// --------------------------------------------------------------------------						
	// Function:	HasFunction
	// Description:	test if process has function of given name
	// Arguments:	name
	// Returns:		bool
	// --------------------------------------------------------------------------
	bool Process::HasFunction(const std::string& functionName) const 
	{ 
		return false; 
	}


	// --------------------------------------------------------------------------						
	// Function:	AssureIntegrity
	// Description:	validates integrity of process
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void Process::AssureIntegrity(bool processOnly) 
	{}
	

	// --------------------------------------------------------------------------						
	// Function:	CanFinalize
	// Description:	returns if able to finalize at present time
	// Arguments:	none
	// Returns:		returns if able to finalize at present time
	// --------------------------------------------------------------------------
	bool Process::CanFinalize() const 
	{ 
		return myObject.IsValid() ? myObject->CanFinalize() : true; 
	}
	

	// --------------------------------------------------------------------------						
	// Function:	Initialize
	// Description:	initializes process
	// Arguments:	none
	// Returns:		if sucessful
	// --------------------------------------------------------------------------
	bool Process::Initialize() 
	{ 
		return true; 
	}


	// --------------------------------------------------------------------------						
	// Function:	CompleteInitialization
	// Description:	call when initialization is complete
	// Arguments:	none
	// Returns:		if successful
	// --------------------------------------------------------------------------
	bool Process::CompleteInitialization()
	{
		Messenger::CompleteInitialization();
		GetVM()->FlagInitialized(GCPtr<Process>(this));
		return true;

	}


	// --------------------------------------------------------------------------						
	// Function:	Terminate
	// Description:	finalizes and destroys
	// Arguments:	caller
	// Returns:		none
	// --------------------------------------------------------------------------
	bool Process::Terminate(const GCPtr<Messenger>& caller)
	{
		if(myObject.IsValid() && myObject->GetOwner() == myVM->GetScheduler())
			myVM->GetScheduler()->RemoveUpdater(myObject);
		return true;
	}


	// --------------------------------------------------------------------------						
	// Function:	CompleteFinalization
	// Description:	final stage of finalizaatiom destroys this
	// Arguments:	none
	// Returns:		of successful
	// --------------------------------------------------------------------------
	bool Process::CompleteFinalization()
	{
		GCPtr<Object> object = myObject;
		object.Destroy();
		return Messenger::CompleteFinalization();
	}


	// --------------------------------------------------------------------------						
	// Function:	SetObject
	// Description:	sets object this process is associated with
	// Arguments:	object
	// Returns:		none
	// --------------------------------------------------------------------------
	void Process::SetObject(const GCPtr<Object>& object)
	{ 
		myObject = object; 
	}
	

	// --------------------------------------------------------------------------						
	// Function:	SetVM
	// Description:	sets vm this process belongs to
	// Arguments:	vm
	// Returns:		none
	// --------------------------------------------------------------------------
	void Process::SetVM(const GCPtr<VM>& vm)
	{ 
		myVM = vm; 
	}
	
}
