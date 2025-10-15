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

#include "../Arc/Priority.h"
#include "../Common/Exception.h"

namespace shh {

	bool Priority::ourPrioritiesFrozem = false;
	Priority::Priorities Priority::ourPriorities = Priority::InitPriorities();
	const int Priority::ourBadPriority = 2147483647;
	int Priority::ourshhArc = -3;
	int Priority::ourEngine = -2;
	int Priority::ourSystem = -1;
	int Priority::ourAgent = 0;
	int Priority::ourMaster = 1;
	int Priority::ourSlave = 2;
	int Priority::ourSchema = 3;
	int Priority::ourExecute = 3;

	// --------------------------------------------------------------------------						
	// Function:	InitPriorities
	// Description:	returns default priorities
	// Arguments:	none
	// Returns:		default priorities
	// --------------------------------------------------------------------------
	Priority::Priorities Priority::InitPriorities()
	{
		Priority::Priorities priorities;
		priorities["PioritySHHARC"] = -3;
		priorities["PriorityEngine"] = -2;
		priorities["PrioritySystem"] = -1;
		priorities["PriorityAgent"] = 0;
		priorities["PriorityMaster"] = 1;
		priorities["PrioritySlave"] = 2;
		priorities["PrioritySchema"] = 3;
		priorities["PriorityExecute"] = 3;
		return priorities;
	}

	// --------------------------------------------------------------------------						
	// Function:	Priority
	// Description:	constructor
	// Arguments:	value of priority
	// Returns:		none
	// --------------------------------------------------------------------------
	Priority::Priority(int value) :
		myPriority(value)
	{
		
	}


	// --------------------------------------------------------------------------						
	// Function:	GetPriority
	// Description:	gets the priority value of a given id
	// Arguments:	id
	// Returns:		priority
	// --------------------------------------------------------------------------
	int Priority::GetPriority(const std::string& id)  
	{
		Priorities::const_iterator it = ourPriorities.find(id);
		if (it == ourPriorities.end())
			return ourBadPriority;
		return it->second;
	}


	// --------------------------------------------------------------------------						
	// Function:	SetPriority
	// Description:	sets the priority value of a given id
	// Arguments:	id, priority
	// Returns:		if successfull
	// --------------------------------------------------------------------------
	bool Priority::SetPriority(const std::string& id, int value)
	{
		if (!ourPrioritiesFrozem)
		{
			ourPriorities[id] = value;

			if(id == "PioritySHHARC")
				ourshhArc = value;
			else if(id == "PriorityEngine")
				ourEngine = value;
			else if(id == "PrioritySystem")
				ourSystem = value;
			else if(id == "PriorityAgent")
				ourAgent = value;
			else if(id == "PriorityMaster")
				ourMaster = value;
			else if(id == "PrioritySlave")
				ourSlave = value;
			else if(id == "PrioritySchema")
				ourSchema = value;
			else if(id == "PriorityExecute")
				ourExecute = value;	

			return true;
		}
		return false;
	}


	// --------------------------------------------------------------------------						
	// Function:	Freeze
	// Description:	locks priorities so they can not longer be changes
	// Arguments:	none
	// Returns:		none
	// --------------------------------------------------------------------------
	void Priority::Freeze()
	{
		ourPrioritiesFrozem = true;
	}
}