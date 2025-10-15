///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2025 David K Bhowmik. All rights reserved.
// This file is part of shhArc.
//
// This Software is available under the MIT License with a No Modification clause
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


#include "../Arc/Environment.h"
#include "../VM/Scheduler.h"
#include "../VM/VM.h"
#include "../VM/ClassManager.h"
#include "../File/IOVariant.h"

namespace shh
{
	StringKeyDictionary Environment::ourMetaConfig; 
	Mutex Environment::ourMutex;
	GCPtr<Environment> Environment::ourGlobalEnvironment;

	// --------------------------------------------------------------------------						
	// Function:	Environment
	// Description:	constructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	Environment::Environment() : 
		myEntryCount(0)
	{
	}


	// --------------------------------------------------------------------------						
	// Function:	~Environment
	// Description:	destructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	Environment::~Environment()
	{
		FinalizeObjects();
		ClassManagers::iterator it = myClassManagers.begin();
		while(it != myClassManagers.end())
		{
			if (it->second.IsValid())
			{
				it->second.Destroy();
				it->second.SetNull();
			}
			it++;
		}
		
		if (myScheduler.IsValid())
			myScheduler.Destroy();

		if (myVM.IsValid())
			myVM.Destroy();
	}


	// --------------------------------------------------------------------------						
	// Function:	Initialize
	// Description:	initializes the environment
	// Arguments:	owner, unique id for this instantioation, dictionary of config
	//				vars
	// Returns:		if successful
	// --------------------------------------------------------------------------
	bool Environment::Initialize(const GCPtr<GCObject>& owner, const std::string& id, const StringKeyDictionary& sd)
	{
		return Module::Initialize(owner, id, sd);
	}
	

	// --------------------------------------------------------------------------						
	// Function:	Finalizing
	// Description:	finalizes engironment
	// Arguments:	pointer to self (most derived class) used to confirm object
	//				pointers
	// Returns:		if successful
	// --------------------------------------------------------------------------
	bool Environment::Finalize(GCObject* me)
	{
		return Module::Finalize(me);
	}


	// --------------------------------------------------------------------------						
	// Function:	Update
	// Description:	updates
	// Arguments:	time to update until, update phase
	// Returns:		if successfull
	// --------------------------------------------------------------------------
	bool Environment::Update(double until, unsigned int phase)
	{
		bool ok = Module::Update(until, phase);
		myScheduler->Update(until, phase);
		return ok;
	}


	// --------------------------------------------------------------------------						
	// Function:	GetClassManager
	// Description:	gets class manager of given name
	// Arguments:	name, manager to return
	// Returns:		if managert of name found
	// --------------------------------------------------------------------------
	bool Environment::GetClassManager(const std::string& name, GCPtr<ClassManager>& manager) const
	{
		ClassManagers::const_iterator it = myClassManagers.find(name);
		if (it != myClassManagers.end())
		{
			manager = it->second;
			return true;
		}
		return false;
	}


	// --------------------------------------------------------------------------						
	// Function:	SetClassManager
	// Description:	adds a classmanager with given name to the environment 
	// Arguments:	name, class manager
	// Returns:		if successfull
	// --------------------------------------------------------------------------
	bool Environment::SetClassManager(const std::string& name, const GCPtr<ClassManager> &manager)
	{
		ClassManagers::const_iterator it = myClassManagers.find(name);
		if (it == myClassManagers.end())
		{
			myClassManagers[name] = manager;
			return true;
		}
		return false;
	}


	// --------------------------------------------------------------------------						
	// Function:	GetScheduler
	// Description:	returns the environments scheuller
	// Arguments:	none
	// Returns:		scheduler
	// --------------------------------------------------------------------------
	const GCPtr<Scheduler>& Environment::GetScheduler() const 
	{ 
		return myScheduler; 
	
	}


	// --------------------------------------------------------------------------						
	// Function:	GetVM
	// Description:	returns the VM of the environment
	// Arguments:	none
	// Returns:		VM
	// --------------------------------------------------------------------------
	const GCPtr<VM>& Environment::GetVM() const
	{ 
		return myVM; 
	}


	// --------------------------------------------------------------------------						
	// Function:	SetMetaVariables
	// Description:	sets the meta variables for all environments
	// Arguments:	meta variable dictionary
	// Returns:		none
	// --------------------------------------------------------------------------
	void Environment::SetMetaVariables(const StringKeyDictionary& d)
	{
#if MULTI_THREADED
		ourMutex.LockMutex();
#endif
		ourMetaConfig = d;
#if MULTI_THREADED
		ourMutex.UnlockMutex();
#endif
	}


	// --------------------------------------------------------------------------						
	// Function:	SetGlobalEnvironment
	// Description:	sets the global environment
	// Arguments:	global environment
	// Returns:		none
	// --------------------------------------------------------------------------
	void Environment::SetGlobalEnvironment(const GCPtr<Environment>& e)
	{
#if MULTI_THREADED
		ourMutex.LockMutex();
#endif
		ourGlobalEnvironment = e;
#if MULTI_THREADED
		ourMutex.UnlockMutex();
#endif
	}


	// --------------------------------------------------------------------------						
	// Function:	GetMeta
	// Description:	get meta variable but forces out character arrays to KEYs 
	//				and also allow defaultValue	to come in as a charater array 
	//				but return as a string
	// Arguments:	variable key, default value if not found
	// Returns:		variable got
	// --------------------------------------------------------------------------
	std::string Environment::GetMeta(const std::string& variableKey, const char defaultValue[])
	{
		return GetMeta(variableKey, std::string(defaultValue));
	}


	// --------------------------------------------------------------------------						
	// Function:	SetMeta
	// Description:	set meta variable but forces out character arrays to KEYs 
	//				and also allow defaultValue	to come in as a charater array 
	// Arguments:	variable key, value to set
	// Returns:		if successfull
	// --------------------------------------------------------------------------
	bool Environment::SetMeta(const std::string& variableKey, const char value[])
	{
		return SetMeta(variableKey, std::string(value));
	}


	// --------------------------------------------------------------------------						
	// Function:	GetGlobal
	// Description:	get global variable but forces out character arrays to KEYs 
	//				and also allow defaultValue	to come in as a charater array 
	//				but return as a string
	// Arguments:	variable key, default value if not found
	// Returns:		variable got
	// --------------------------------------------------------------------------
	std::string Environment::GetGlobal(const std::string& variableKey, const char defaultValue[], bool checkMeta)
	{
		return GetGlobal(variableKey, std::string(defaultValue), checkMeta);
	}


	// --------------------------------------------------------------------------						
	// Function:	SetGlobal
	// Description:	set global variable but forces out character arrays to KEYs 
	//				and also allow defaultValue	to come in as a charater array 
	// Arguments:	variable key, value to set
	// Returns:		if successfull
	// --------------------------------------------------------------------------
	bool Environment::SetGlobal(const std::string& variableKey, const char value[])
	{
		return SetGlobal(variableKey, std::string(value));
	}


	// --------------------------------------------------------------------------						
	// Function:	GetLocal
	// Description:	get local variable but forces out character arrays to KEYs 
	//				and also allow defaultValue	to come in as a charater array 
	//				but return as a string
	// Arguments:	variable key, default value if not found
	// Returns:		variable got
	// --------------------------------------------------------------------------

	std::string Environment::GetLocal(const std::string& variableKey, const char defaultValue[]) const
	{
		return GetLocal(variableKey, std::string(defaultValue));
	}


	// --------------------------------------------------------------------------						
	// Function:	SetLocal
	// Description:	set local variable but forces out character arrays to KEYs 
	//				and also allow defaultValue	to come in as a charater array 
	// Arguments:	variable key, value to set
	// Returns:		if successfull
	// --------------------------------------------------------------------------	bool Environment::SetLocal(const std::string& variableKey, const char value[])
	bool Environment::SetLocal(const std::string& variableKey, const char value[])
	{
		return SetLocal(variableKey, std::string(value));
	}


	// --------------------------------------------------------------------------						
	// Function:	IncEntryCount
	// Description:	logs number of entries into this environment for scripts
	//				in other environments
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void Environment::IncEntryCount() 
	{ 
		myEntryCount++; 
	}


	// --------------------------------------------------------------------------						
	// Function:	DecEntryCount
	// Description:	logs number of entries into this environment for scripts
	//				in other environments
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void Environment::DecEntryCount()
	{ 
		myEntryCount--; 
	}


	// --------------------------------------------------------------------------						
	// Function:	FinalizeObjects
	// Description:	finalizes all objects
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void Environment::FinalizeObjects()
	{
		myScheduler->ClearAllMessages();
		for(ClassManagers::iterator it = myClassManagers.begin(); it != myClassManagers.end(); it++)
			it->second->FinalizeObjects();

		myScheduler->Update(-1.0, 0);

	}

}
