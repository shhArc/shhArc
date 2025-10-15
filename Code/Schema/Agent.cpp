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
#pragma warning(disable:4786 4503)
#endif

#include "../VM/ClassManager.h"
#include "Agent.h"
#include "Node.h"
#include <inttypes.h>


namespace shh {

	IMPLEMENT_MEMORY_MANAGED(Agent);


	// --------------------------------------------------------------------------						
	// Function:	Create
	// Description:	creates an agent
	// Arguments:	class manager the agent will belong to, class of agent
	//				process associated with agent
	// Returns:		agent
	// --------------------------------------------------------------------------
	GCPtr<Object> Agent::Create(const GCPtr<ClassManager>& classManager, const GCPtr<Class>& objectClass, const GCPtr<Process>& process)
	{
		GCPtr<Agent> agent(new Agent(classManager, objectClass, process));
		GCPtr<Object> object;
		object.DynamicCast(agent);
		return object;
	}


	// --------------------------------------------------------------------------						
	// Function:	Agent
	// Description:	constructor
	// Arguments:	class manager the agent will belong to, class of agent
	//				process associated with agent
	// Returns:		none
	// --------------------------------------------------------------------------
	Agent::Agent(const GCPtr<ClassManager>& classManager, const GCPtr<Class>& objectClass, const GCPtr<Process>& process) :
		Object(classManager, objectClass, process),
		Schema(),
		myNeedsComponentIntegrityCheck(false)	
	{
		SetGCMemoryStart(this);
	}


	// --------------------------------------------------------------------------						
	// Function:	Agent
	// Description:	destructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	Agent::~Agent()
	{
		Zap();
	}


	// --------------------------------------------------------------------------						
	// Function:	Finalize
	// Description:	finalizeds agent
	// Arguments:	pointer to this
	// Returns:		none
	// --------------------------------------------------------------------------
	bool Agent::Finalize(GCObject*me)
	{
		if (!myFinalized && !IsDying())
		{
			Agent* agent = dynamic_cast<Agent*>(me);
			agent->AssureIntegrity(true);
			Module::Finalize(me);
			myFinalized = true;
			return VM::Finalize(me);
		}
		return false;
	}


	// --------------------------------------------------------------------------						
	// Function:	AssureIntegrity
	// Description:	integrity checks the agent
	// Arguments:	if only do vm or object too
	// Returns:		none
	// --------------------------------------------------------------------------
	void Agent::AssureIntegrity(bool vmOnly)
	{
		// note expects current agent to be this agent
		if (myNeedsComponentIntegrityCheck && !vmOnly)
		{
			myClassManager->AssureIntegrity(GCPtr<Object>(this));
			myNeedsComponentIntegrityCheck = false;
		}
		VM::AssureIntegrity();
	}


	// --------------------------------------------------------------------------						
	// Function:	AddSlaveProcess
	// Description:	adds a slave process
	// Arguments:	slave
	// Returns:		if added
	// --------------------------------------------------------------------------
	bool Agent::AddSlaveProcess(const GCPtr<Process>& slave)
	{
		if (VM::AddSlaveProcess(slave))
		{
			return true;
		}
		return false;
	}


	// --------------------------------------------------------------------------						
	// Function:	RemoveProcess
	// Description:	removes a process
	// Arguments:	process
	// Returns:		if removed
	// --------------------------------------------------------------------------
	bool Agent::RemoveProcess(const GCPtr<Process>& p)
	{
		Processes::iterator it = mySlaveProcesses.find(p->GetId());
		if (it != mySlaveProcesses.end())
		{
			GCPtr<Schema> s;
			s.DynamicCast(it->second->GetObject());
			if (s.IsValid())
				RemoveSchema(s);
		}
		return VM::RemoveProcess(p);
	}


	// --------------------------------------------------------------------------						
	// Function:	Initialize
	// Description:	intializes agent an super Vn and Object
	// Arguments:	owner of agent, id string of the object, disctionary to 
	//				initialize with
	// Returns:		if sucessful
	// --------------------------------------------------------------------------
	bool Agent::Initialize(const GCPtr<GCObject>& owner, const std::string& id, const StringKeyDictionary &sd)
	{
		bool ok = Object::Initialize(owner, id, sd);
		myMasterProcess = myProcess;
		myProcess->myPrivileges = myProcess->myPrivileges | MasterPrivilege;
		myMasterProcess->SetVM(GCPtr<VM>(this));
		myMasterProcess->Initialize();
		FlagUninitialized(myProcess);
		return ok;
	}


	// --------------------------------------------------------------------------						
	// Function:	GetActiveAgent
	// Description:	gets currently active agent in this thread
	// Arguments:	none
	// Returns:		agent
	// --------------------------------------------------------------------------
	GCPtr<Agent> Agent::GetActiveAgent()
	{
		GCPtr<VM> vm = Scheduler::GetCurrentProcess()->GetVM();
		GCPtr<Agent> agent;
		agent.DynamicCast(vm);
		return agent;
	}


	// --------------------------------------------------------------------------						
	// Function:	PushSelf
	// Description:	pushes self onto current process
	// Arguments:	implementation of process
	// Returns:		none
	// --------------------------------------------------------------------------
	void Agent::PushSelf(Implementation i)
	{
		GCPtr<Agent>* me = new GCPtr<Agent>(this);
		Type<GCPtr<Agent>>::GetStatic()->Push(i, me);
	}


	// --------------------------------------------------------------------------						
	// Function:	CanFinalize
	// Description:	returns if can finalize as it may still have
	//				slave processes
	// Arguments:	none
	// Returns:		bool
	// --------------------------------------------------------------------------
	bool Agent::CanFinalize() const
	{ 
		return mySlaveProcesses.empty(); 
	}

}


