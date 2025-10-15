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

#include "../Arc/Module.h"
#include "../Common/Exception.h"
#include "../VM/Process.h"
#include "../VM/VM.h"

namespace shh {


	shhId Module::Sorter::ourLastId = 0;


	// --------------------------------------------------------------------------						
	// Function:	Module
	// Description:	constructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	Module::Module() :
		myFlags(0),
		myTimeOut(0.0),
		myPhase(0),
		myRequiresUpdate(false),
		myUpdateUntilTime(0.0),
		myDelta(0.0),
		myLastUpdateCallTime(0.0),
		myImplementation(Engine),
		myFullUpdate(true)
	{}


	// --------------------------------------------------------------------------						
	// Function:	Module
	// Description:	constructor
	// Arguments:	owner, max timme to spend on updating
	// Returns:		none
	// --------------------------------------------------------------------------
	Module::Module(const GCPtr<GCObject>& owner, double timeOut) :
		myFlags(0),
		myTimeOut(timeOut),
		myPhase(0),
		myRequiresUpdate(false),
		myUpdateUntilTime(0.0),
		myDelta(0.0),
		myLastUpdateCallTime(0.0),
		myFullUpdate(true)
	{}


	// --------------------------------------------------------------------------						
	// Function:	~Module
	// Description:	denstructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	Module::~Module()
	{
		for (Module::Map::iterator m = mySubModules.begin(); m != mySubModules.end(); m++)
			m->second.Destroy();

		mySubModules.clear();
	}


	// --------------------------------------------------------------------------						
	// Function:	~Module
	// Description:	denstructor
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	bool Module::RegisterTypes(const std::string &alias, const StringKeyDictionary& sd) 
	{
		bool ok = true;
		StringKeyDictionary subModulesDict;
		subModulesDict = sd.Get("submodules", subModulesDict);
		StringKeyDictionary::VariablesConstIterator mdit = subModulesDict.Begin();

		for (Map::iterator mit = mySubModules.begin(); mit != mySubModules.end(); mit++)
		{
			StringKeyDictionary modSpec;
			if (mdit != subModulesDict.End())
			{
				mdit->second->Get(modSpec);
				mdit++;
			}

			std::string alias;
			alias = modSpec.Get("alias", alias);
			StringKeyDictionary dict;
			dict = modSpec.Get("config", dict);
			if (!mit->second->RegisterTypes(alias, dict))
				ok = false;
		}
		return ok;
	}


	// --------------------------------------------------------------------------						
	// Function:	Register
	// Description:	called when registering the module with a process 
	//				(registing functions etc)
	// Arguments:	alias of module set in config file, dictionary of config vars,
	//				privileges of object registering with
	// Returns:		if successful
	// --------------------------------------------------------------------------
	bool Module::Register(const std::string& alias, const StringKeyDictionary& sd, Privileges privileges)
	{
		bool ok = true;
		StringKeyDictionary subModulesDict;
		subModulesDict = sd.Get("submodules", subModulesDict);
		StringKeyDictionary::VariablesConstIterator mdit = subModulesDict.Begin();

		for (Map::iterator mit = mySubModules.begin(); mit != mySubModules.end(); mit++)
		{
			StringKeyDictionary modSpec;
			if (mdit != subModulesDict.End())
			{
				mdit->second->Get(modSpec);
				mdit++;
			}

			std::string alias;
			alias = modSpec.Get("alias", alias);
			StringKeyDictionary dict;
			dict = modSpec.Get("config", dict);

			if (!mit->second->Register(alias, dict, privileges))
				ok = false;
		}
		return ok;
	}


	// --------------------------------------------------------------------------						
	// Function:	Initialize
	// Description:	initalizes the module after creation
	// Arguments:	owner, unique id for this instance, dictionary of config vars
	// Returns:		if successful
	// --------------------------------------------------------------------------
	bool Module::Initialize(const GCPtr<GCObject>& owner, const std::string &id, const StringKeyDictionary& sd)
	{
		bool ok = true;

		myOwner = owner;
		myId = id;
		
		StringKeyDictionary subModulesDict;
		subModulesDict = sd.Get("submodules", subModulesDict);
		StringKeyDictionary::VariablesConstIterator mdit = subModulesDict.Begin();
		
		for (Map::iterator mit = mySubModules.begin(); mit != mySubModules.end(); mit++)
		{
			StringKeyDictionary modSpec;
			if (mdit != subModulesDict.End())
			{
				mdit->second->Get(modSpec);
				mdit++;
			}

			std::string id;
			id = modSpec.Get("id", id);
	
			if (!mit->second->Initialize(GCPtr<GCObject>(this), id, modSpec))
				ok = false;
		}
		return ok;
	}
	
	
	// --------------------------------------------------------------------------						
	// Function:	Finalize
	// Description:	finalizes module before deletion
	// Arguments:	pointer to self (most derived class) needed for multiple
	//				inheritance
	// Returns:		if successful
	// --------------------------------------------------------------------------
	bool Module::Finalize(GCObject* me)
	{
		bool ok = true;
		for (Map::iterator mit = mySubModules.begin(); mit != mySubModules.end(); mit++)
		{
			if (mit->second.IsValid())
			{
				if (!mit->second->Finalize(mit->second.GetObject()))
					ok = false;
			}
		}
		return ok;
	}


	// --------------------------------------------------------------------------						
	// Function:	Update
	// Description:	updates
	// Arguments:	time to update until, update phase
	// Returns:		if successfull
	// --------------------------------------------------------------------------
	bool Module::Update(double until, unsigned int phase)
	{
		if (myRequiresUpdate)
		{
			myDelta = myLastUpdateCallTime ? until - myLastUpdateCallTime : 0.0;
			myUpdateUntilTime = until;
			if (myFullUpdate)
			{
				for (Map::iterator mit = mySubModules.begin(); mit != mySubModules.end(); mit++)
					mit->second->Update(until, phase);
			}
			return true;
		}
		return false;
	}


	// --------------------------------------------------------------------------						
	// Function:	FlagUpdateCompleted
	// Description:	called after all updates of this module are finished
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void Module::FlagUpdateCompleted()
	{
		myLastUpdateCallTime = myUpdateUntilTime;
		for (Map::iterator mit = mySubModules.begin(); mit != mySubModules.end(); mit++)
			mit->second->FlagUpdateCompleted();
	}


	// --------------------------------------------------------------------------						
	// Function:	InitializeSubModule
	// Description:	initializes a submodule of given id
	// Arguments:	sub module id, dictionary of config varts
	// Returns:		if successful
	// --------------------------------------------------------------------------
	bool Module::InitializeSubModule(const std::string& id, const StringKeyDictionary& sd)
	{
		Module::Map::iterator it = mySubModules.begin();
		while (it != mySubModules.end())
		{
			if (it->first.myName == id)
				return it->second->Initialize(GCPtr<Module>(this), id, sd);
			
			it++;
		}
		return false;
	}


	// --------------------------------------------------------------------------						
	// Function:	FinalizeSubModule
	// Description:	finalilizes a submodule of given id
	// Arguments:	sub module id, dictionary of config varts
	// Returns:		if successful
	// --------------------------------------------------------------------------
	bool Module::FinalizeSubModule(const std::string& id)
	{
		Module::Map::iterator it = mySubModules.begin();
		while (it != mySubModules.end())
		{
			if (it->first.myName == id)
				return it->second->Finalize(it->second.GetObject());
			it++;
		}
		return false;
	}


	// --------------------------------------------------------------------------						
	// Function:	UpdateSubModule
	// Description:	updates a submodule of given id
	// Arguments:	sub module id, time to update until, phase of update
	// Returns:		if successful
	// --------------------------------------------------------------------------
	bool Module::UpdateSubModule(const std::string& id, double until, unsigned int phase)
	{
		Module::Map::iterator it = mySubModules.begin();
		while (it != mySubModules.end())
		{
			if (it->first.myName == id)
			{
				if (it->second->GetNumPhasesPerUpdate() == 0 || myPhase < it->second->GetNumPhasesPerUpdate())
					return it->second->Update(until, phase);
				else
					return true;
			}
			it++;
		}
		return false;
	}


	// --------------------------------------------------------------------------						
	// Function:	GetNextMessage
	// Description:	gets the next message this module has for update that is
	//				before the given until time
	// Arguments:	until time, phase of update, message returned
	// Returns:		if got a message
	// --------------------------------------------------------------------------
	bool Module::GetNextMessage(double until, unsigned int phase, Message*& msg)
	{
		msg = NULL;
		return false;
	}


	// --------------------------------------------------------------------------						
	// Function:	SetActive
	// Description:	called when this module becomes active
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void Module::SetActive(const GCPtr<GCObject>& object)
	{}
	

	// --------------------------------------------------------------------------						
	// Function:	SetAsInactive
	// Description:	called when this module becomes inactive
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void Module::SetAsInactive()
	{}


	// --------------------------------------------------------------------------						
	// Function:	GetNumPhasesPerUpdate
	// Description:	return number of update phases this module has 
	// Arguments:	none
	// Returns:		number of phases
	// --------------------------------------------------------------------------
	unsigned int Module::GetNumPhasesPerUpdate() const
	{
		unsigned int max = 0;
		for (Map::const_iterator mit = mySubModules.begin(); mit != mySubModules.end(); mit++)
		{
			unsigned int numUpdated = mit->second->GetNumPhasesPerUpdate();
			if (numUpdated > max)
				max = numUpdated;
		}
		return max;
	}


	// --------------------------------------------------------------------------						
	// Function:	AssureIntegrity
	// Description:	runs the modules integrity check
	// Arguments:	none
	// Returns:		if successfull
	// --------------------------------------------------------------------------
	bool Module::AssureIntegrity()
	{ 
		return true; 
	}


	// --------------------------------------------------------------------------						
	// Function:	RequiresIntegrityCheckOnInit
	// Description:	returns whether this module requires integrity check on 
	//				initialization
	// Arguments:	none
	// Returns:		bool
	// --------------------------------------------------------------------------
	bool Module::RequiresIntegrityCheckOnInit() const
	{ 
		return GetFlag(IntegrityCehckOnInit); 
	}


	// --------------------------------------------------------------------------						
	// Function:	GetName
	// Description:	returns the module name
	// Arguments:	none
	// Returns:		name
	// --------------------------------------------------------------------------
	std::string Module::GetName() const
	{ 
		return "shhModule"; 
	}


	// --------------------------------------------------------------------------						
	// Function:	StripName
	// Description:	formats name text (used for class typenames)
	// Arguments:	text
	// Returns:		formatted string
	// --------------------------------------------------------------------------
	std::string Module::StripName(const std::string& text)
	{ 
		return text.substr(text.rfind(" ") + 1, text.length() - text.rfind(" ") + 1); 
	}

	
	// --------------------------------------------------------------------------						
	// Function:	SetFlag
	// Description:	sets a modules flag
	// Arguments:	flag number, value to set
	// Returns:		none
	// --------------------------------------------------------------------------
	void Module::SetFlag(int flagNo, bool state)
	{
		unsigned int flag = (state ? (unsigned int)1 : (unsigned int)0) << flagNo;
		unsigned int mask = ~((unsigned int)1 << flagNo);
		myFlags &= mask;
		myFlags |= flag;
	}


	// --------------------------------------------------------------------------						
	// Function:	GetFlag
	// Description:	returns the state of a flag
	// Arguments:	flag number
	// Returns:		state
	// --------------------------------------------------------------------------
	bool Module::GetFlag(int flagNo) const
	{
		unsigned int flag = (unsigned int)1 << flagNo;
		return (myFlags & flag) != 0;
	}
}//namespace shh

